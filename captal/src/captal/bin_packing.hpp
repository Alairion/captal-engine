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

#ifndef CAPTAL_BIN_PACKING_HPP_INCLUDED
#define CAPTAL_BIN_PACKING_HPP_INCLUDED

#include "config.hpp"

#include <vector>
#include <array>
#include <optional>

namespace cpt
{

class CAPTAL_API bin_packer
{
public:
    struct rect
    {
        std::uint32_t x{};
        std::uint32_t y{};
        std::uint32_t width{};
        std::uint32_t height{};
    };

public:
    bin_packer() = default;
    explicit bin_packer(std::uint32_t width, std::uint32_t height);

    bin_packer(const bin_packer&) = delete;
    bin_packer& operator=(const bin_packer&) = delete;
    bin_packer(bin_packer&&) noexcept = default;
    bin_packer& operator=(bin_packer&&) noexcept = default;

    std::optional<rect> append(std::uint32_t image_width, std::uint32_t image_height);
    void grow(std::uint32_t width, std::uint32_t height);

    std::uint32_t width() const noexcept
    {
        return m_width;
    }

    std::uint32_t height() const noexcept
    {
        return m_height;
    }

private:
    struct splits
    {
        std::size_t count{};
        std::array<rect, 2> parts{};
    };

private:
    splits split(std::uint32_t image_width, std::uint32_t image_height, const rect& space) noexcept;

private:
    std::uint32_t m_width{};
    std::uint32_t m_height{};
    std::vector<rect> m_spaces{};
};

}

#endif
