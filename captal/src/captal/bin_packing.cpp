#include "bin_packing.hpp"

#include <cassert>

namespace cpt
{

static std::int32_t area_fit_score(std::uint32_t width, std::uint32_t height, const bin_packer::rect& rect) noexcept
{
    return rect.width * rect.height - width * height;
}

bin_packer::bin_packer(uint32_t width, uint32_t height)
:m_width{width}
,m_height{height}
,m_spaces{rect{0, 0, width, height}}
{
    m_spaces.reserve(256);
}

std::optional<bin_packer::rect> bin_packer::append(std::uint32_t image_width, std::uint32_t image_height)
{
    std::ptrdiff_t best_index{std::numeric_limits<std::ptrdiff_t>::max()};
    splits best_splits{};
    std::int32_t best_score{};
    bool best_flipped{};

    for(auto i{std::ssize(m_spaces) - 1}; i >= 0; --i)
    {
        const auto candidate{m_spaces[i]};
        const auto splits{split(image_width, image_height, candidate)};
        const auto flipped{split(image_height, image_width, candidate)};

        if(splits && flipped)
        {
            if(splits->count == 0) //perfect fit
            {
                m_spaces[i] = m_spaces.back();
                m_spaces.pop_back();

                for(std::size_t j{}; j < splits->count; ++j)
                {
                    m_spaces.emplace_back(splits->splits[j]);
                }

                return rect{candidate.x, candidate.y, image_width, image_height};
            }

            if(flipped->count == 0) //perfect fit
            {
                m_spaces[i] = m_spaces.back();
                m_spaces.pop_back();

                for(std::size_t j{}; j < flipped->count; ++j)
                {
                    m_spaces.emplace_back(flipped->splits[j]);
                }

                return rect{candidate.x, candidate.y, image_height, image_width};
            }

            const std::int32_t score{area_fit_score(image_width, image_height, candidate)};

            if(score > best_score)
            {
                best_score = score;
                best_index = i;

                if(flipped->count < splits->count)
                {
                    best_splits = flipped.value();
                    best_flipped = true;
                }
                else
                {
                    best_splits = splits.value();
                    best_flipped = false;
                }
            }
        }
        else if(splits)
        {
            if(splits->count == 0) //perfect fit
            {
                m_spaces[i] = m_spaces.back();
                m_spaces.pop_back();

                for(std::size_t j{}; j < splits->count; ++j)
                {
                    m_spaces.emplace_back(splits->splits[j]);
                }

                return rect{candidate.x, candidate.y, image_width, image_height};
            }

            const std::int32_t score{area_fit_score(image_width, image_height, candidate)};

            if(score > best_score)
            {
                best_score = score;
                best_index = i;
                best_splits = splits.value();
                best_flipped = false;
            }
        }
        else if(flipped)
        {
            if(flipped->count == 0) //perfect fit
            {
                m_spaces[i] = m_spaces.back();
                m_spaces.pop_back();

                for(std::size_t j{}; j < flipped->count; ++j)
                {
                    m_spaces.emplace_back(flipped->splits[j]);
                }

                return rect{candidate.x, candidate.y, image_width, image_height};
            }

            const std::int32_t score{area_fit_score(image_width, image_height, candidate)};

            if(score > best_score)
            {
                best_score = score;
                best_index = i;
                best_splits = flipped.value();
                best_flipped = true;
            }
        }
    }

    if(best_index != std::numeric_limits<std::ptrdiff_t>::max())
    {
        const auto x{m_spaces[best_index].x};
        const auto y{m_spaces[best_index].y};

        m_spaces[best_index] = m_spaces.back();
        m_spaces.pop_back();

        for(std::size_t j{}; j < best_splits.count; ++j)
        {
            m_spaces.emplace_back(best_splits.splits[j]);
        }

        if(best_flipped)
        {
            return rect{x, y, image_height, image_width};
        }

        return rect{x, y, image_width, image_height};
    }

    return std::nullopt;
}

void bin_packer::resize(std::uint32_t width, std::uint32_t height) noexcept
{
    assert(width > m_width);
    assert(height > m_height);

    const auto width_diff{width - m_width};
    const auto height_diff{height - m_height};
    /*
    for(auto& space : m_spaces)
    {
        if(space.x + space.width == m_width)
        {
            space.width = width - space.x;
        }

        if(space.y + space.height == m_height)
        {
            space.height = height - space.y;
        }
    }*/

    m_spaces.emplace_back(rect{0, m_height, width_diff, height_diff});
    m_spaces.emplace_back(rect{m_width, m_height, width_diff, height_diff});
    m_spaces.emplace_back(rect{m_width, 0, width_diff, height_diff});

    m_width = width;
    m_height = height;
}

std::optional<bin_packer::splits> bin_packer::split(uint32_t image_width, uint32_t image_height, const bin_packer::rect& space) noexcept
{
    const auto free_width{static_cast<std::int32_t>(space.width) - static_cast<std::int32_t>(image_width)};
    const auto free_height{static_cast<std::int32_t>(space.height) - static_cast<std::int32_t>(image_height)};

    if(free_width < 0 || free_height < 0)
    {
        return std::nullopt;
    }

    if(free_width == 0 && free_height == 0)
    {
        return splits{};
    }

    if(free_width > 0 && free_height == 0)
    {
        auto output{space};

        output.x += image_width;
        output.width -= image_width;

        return splits{1, {output}};
    }

    if(free_width == 0 && free_height > 0)
    {
        auto output{space};

        output.y += image_height;
        output.height -= image_height;

        return splits{1, {output}};
    }

    if(free_width > free_height)
    {
        const rect bigger_split
        {
            space.x + image_width,
            space.y,
            static_cast<std::uint32_t>(free_width),
            space.height
        };

        const rect lesser_split
        {
            space.x,
            space.y + image_height,
            image_width,
            static_cast<std::uint32_t>(free_height)
        };

        return splits{2, {bigger_split, lesser_split}};
    }

    const rect bigger_split
    {
        space.x,
        space.y + image_height,
        space.width,
        static_cast<std::uint32_t>(free_height)
    };

    const rect lesser_split
    {
        space.x + image_width,
        space.y,
        static_cast<std::uint32_t>(free_width),
        image_height
    };

    return splits{2, {bigger_split, lesser_split}};
}

}
