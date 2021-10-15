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

#ifndef CAPTAL_FOUNDATION_BASE_HPP_INCLUDED
#define CAPTAL_FOUNDATION_BASE_HPP_INCLUDED

#include <cstdint>
#include <type_traits>

namespace cpt
{

inline namespace foundation
{

template<typename T>
constexpr T align_down(T offset, T alignment) noexcept
{
    return offset & ~(alignment - static_cast<T>(1));
}

template<typename T>
constexpr T align_up(T offset, T alignment) noexcept
{
    return align_down(offset + alignment - static_cast<T>(1), alignment);
}

constexpr std::uint8_t bswap(std::uint8_t value) noexcept
{
    return value;
}

constexpr std::uint16_t bswap(std::uint16_t value) noexcept
{
    return static_cast<std::uint16_t>((value << 8) | (value >> 8));
}

constexpr std::uint32_t bswap(std::uint32_t value) noexcept
{
    value = ((value << 8) & 0xFF00FF00) | ((value >> 8) & 0x00FF00FF);

    return (value << 16) | (value >> 16);
}

constexpr std::uint64_t bswap(std::uint64_t value) noexcept
{
    value = ((value & 0x00000000FFFFFFFFull) << 32) | ((value & 0xFFFFFFFF00000000ull) >> 32);
    value = ((value & 0x0000FFFF0000FFFFull) << 16) | ((value & 0xFFFF0000FFFF0000ull) >> 16);
    value = ((value & 0x00FF00FF00FF00FFull) << 8)  | ((value & 0xFF00FF00FF00FF00ull) >> 8);

    return value;
}

template<typename Enum>
constexpr Enum bswap(Enum value) noexcept requires(std::is_enum_v<Enum> && std::is_unsigned_v<std::underlying_type_t<Enum>>)
{
    return static_cast<Enum>(bswap(static_cast<std::underlying_type_t<Enum>>(value)));
}

}

}

#endif
