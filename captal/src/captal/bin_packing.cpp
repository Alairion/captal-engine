#include "bin_packing.hpp"

#include <cassert>

namespace cpt
{

static std::uint32_t area_fit_score(std::uint32_t width, std::uint32_t height, const bin_packer::rect& rect) noexcept
{
    return rect.width * rect.height - width * height;
}

static std::uint32_t rect_area_comparator(const bin_packer::rect& left, const bin_packer::rect& right)
{
    return left.width * left.height < right.width * right.height;
}

bin_packer::bin_packer(uint32_t width, uint32_t height)
:m_width{width}
,m_height{height}
,m_spaces{rect{0, 0, width, height}}
{
    m_spaces.reserve(128);
}
/*
std::optional<bin_packer::rect> bin_packer::append(std::uint32_t image_width, std::uint32_t image_height)
{
    std::ptrdiff_t best_index{std::numeric_limits<std::ptrdiff_t>::max()};
    splits best_splits{};
    std::uint32_t best_score{std::numeric_limits<std::uint32_t>::max()};
    bool best_flipped{};

    const auto accept = [this](std::size_t i, const rect& candidate, const bin_packer::splits& splits, std::uint32_t image_width, std::uint32_t image_height)
    {
        m_spaces[i] = m_spaces.back();
        m_spaces.pop_back();

        for(std::size_t j{}; j < splits.count; ++j)
        {
            m_spaces.emplace_back(splits.splits[j]);
        }

        return rect{candidate.x, candidate.y, image_width, image_height};
    };

    for(auto i{std::ssize(m_spaces) - 1}; i >= 0; --i)
    {
        const auto candidate{m_spaces[i]};
        const auto splits{split(image_width, image_height, candidate)};
        const auto flipped{split(image_height, image_width, candidate)};

        if(splits && flipped)
        {
            if(splits->count == 0) //perfect fit
            {
                return accept(i, candidate, splits.value(), image_width, image_height);
            }

            if(flipped->count == 0) //perfect fit
            {
                return accept(i, candidate, flipped.value(), image_height, image_width);
            }

            const auto score{area_fit_score(image_width, image_height, candidate)};

            if(score < best_score)
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
                return accept(i, candidate, splits.value(), image_width, image_height);
            }

            const auto score{area_fit_score(image_width, image_height, candidate)};

            if(score < best_score)
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
                return accept(i, candidate, flipped.value(), image_height, image_width);
            }

            const auto score{area_fit_score(image_width, image_height, candidate)};

            if(score < best_score)
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
        const auto candidate{m_spaces[best_index]};

        if(best_flipped)
        {
            return accept(best_index, candidate, best_splits, image_height, image_width);
        }

        return accept(best_index, candidate, best_splits, image_width, image_height);
    }

    return std::nullopt;
}

void bin_packer::resize(std::uint32_t width, std::uint32_t height) noexcept
{
    assert(width > m_width);
    assert(height > m_height);

    const auto width_diff{width - m_width};
    const auto height_diff{height - m_height};

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
}*/

std::optional<bin_packer::rect> bin_packer::append(std::uint32_t image_width, std::uint32_t image_height)
{
    const auto accept = [this](const auto it, const splits& splits, const rect& candidate, std::uint32_t image_width, std::uint32_t image_height)
    {
        m_spaces.erase(it);

        if(splits.count > 0)
        {
            m_spaces.insert(std::lower_bound(std::begin(m_spaces), std::end(m_spaces), splits.splits[0], rect_area_comparator), splits.splits[0]);
        }

        if(splits.count > 1)
        {
            m_spaces.insert(std::lower_bound(std::begin(m_spaces), std::end(m_spaces), splits.splits[1], rect_area_comparator), splits.splits[1]);
        }

        return rect{candidate.x, candidate.y, image_width, image_height};
    };

    auto it{std::lower_bound(std::begin(m_spaces), std::end(m_spaces), rect{0, 0, image_width, image_height}, rect_area_comparator)};
    if(it != std::end(m_spaces))
    {
        while(it != std::end(m_spaces))
        {
            const auto candidate{*it};

            if(candidate.width >= image_width && candidate.height >= image_height)
            {
                const auto splits{split(image_width, image_height, candidate)};

                return accept(it, splits, candidate, image_width, image_height);
            }
            else if(candidate.width >= image_height && candidate.height >= image_width) //flip
            {
                const auto splits{split(image_height, image_width, candidate)};

                return accept(it, splits, candidate, image_height, image_width);
            }

            ++it;
        }
    }

    return std::nullopt;
}

void bin_packer::resize(std::uint32_t width, std::uint32_t height)
{
    assert(width > m_width);
    assert(height > m_height);

    const auto width_diff{width - m_width};
    const auto height_diff{height - m_height};

    const rect bottom_left{0, m_height, width_diff, height_diff};
    m_spaces.insert(std::lower_bound(std::begin(m_spaces), std::end(m_spaces), bottom_left, rect_area_comparator), bottom_left);

    const rect bottom_right{m_width, m_height, width_diff, height_diff};
    m_spaces.insert(std::lower_bound(std::begin(m_spaces), std::end(m_spaces), bottom_right, rect_area_comparator), bottom_right);

    const rect top_right{m_width, 0, width_diff, height_diff};
    m_spaces.insert(std::lower_bound(std::begin(m_spaces), std::end(m_spaces), top_right, rect_area_comparator), top_right);

    m_width = width;
    m_height = height;
}

bin_packer::splits bin_packer::split(std::uint32_t image_width, std::uint32_t image_height, const rect& space) noexcept
{
    const auto free_width{space.width - image_width};
    const auto free_height{space.height - image_height};

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
