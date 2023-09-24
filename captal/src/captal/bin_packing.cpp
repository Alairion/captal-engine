//MIT License
//
//Copyright (c) 2021 Alexy Pellegrini
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "bin_packing.hpp"

#include <cassert>
#include <algorithm>

namespace cpt
{

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

std::optional<bin_packer::rect> bin_packer::append(std::uint32_t image_width, std::uint32_t image_height)
{
    const auto accept = [this](const auto it, const splits& splits, const rect& candidate, std::uint32_t image_width, std::uint32_t image_height)
    {
        m_spaces.erase(it);

        if(splits.count > 0)
        {
            m_spaces.insert(std::lower_bound(std::begin(m_spaces), std::end(m_spaces), splits.parts[0], rect_area_comparator), splits.parts[0]);

            if(splits.count > 1)
            {
                m_spaces.insert(std::lower_bound(std::begin(m_spaces), std::end(m_spaces), splits.parts[1], rect_area_comparator), splits.parts[1]);
            }
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

void bin_packer::grow(std::uint32_t width, std::uint32_t height)
{
    if(width > 0)
    {
        const rect top_right{m_width, 0, width, m_height};
        m_spaces.insert(std::lower_bound(std::begin(m_spaces), std::end(m_spaces), top_right, rect_area_comparator), top_right);
    }

    if(height > 0)
    {
        const rect bottom_left{0, m_height, m_width, height};
        m_spaces.insert(std::lower_bound(std::begin(m_spaces), std::end(m_spaces), bottom_left, rect_area_comparator), bottom_left);
    }

    if(width > 0 && height > 0)
    {
        const rect bottom_right{m_width, m_height, width, width};
        m_spaces.insert(std::lower_bound(std::begin(m_spaces), std::end(m_spaces), bottom_right, rect_area_comparator), bottom_right);
    }

    m_width += width;
    m_height += height;
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
