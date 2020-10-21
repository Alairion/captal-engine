#include "bin_packing.hpp"

namespace cpt
{

bin_packer::bin_packer(uint32_t width, uint32_t height)
:m_width{width}
,m_height{height}
,m_spaces{rect{0, 0, width, height}}
{
    m_spaces.reserve(256);
}

std::optional<bin_packer::rect> bin_packer::append(uint32_t image_width, uint32_t image_height)
{
    for(auto i{std::ssize(m_spaces) - 1}; i >= 0; --i)
    {
        const auto candidate{m_spaces[i]};
        const auto splits{split(image_width, image_height, candidate)};

        if(splits)
        {
            m_spaces[i] = m_spaces.back();
            m_spaces.pop_back();

            for(std::size_t j{}; j < splits->count; ++j)
            {
                m_spaces.emplace_back(splits->splits[j]);
            }

            return rect{candidate.x, candidate.y, image_width, image_height};
        }
    }

    return std::nullopt;
}

void bin_packer::resize(uint32_t width, uint32_t height) noexcept
{
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
    }

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
