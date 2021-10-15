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

#ifndef CAPTAL_FOUNDATION_VERSION_HPP_INCLUDED
#define CAPTAL_FOUNDATION_VERSION_HPP_INCLUDED

#include <cstdint>
#include <cstring>
#include <bit>

namespace cpt
{

inline namespace foundation
{

struct alignas(std::uint64_t) version
{
    std::uint16_t major{};
    std::uint16_t minor{};
    std::uint32_t patch{};
};

constexpr std::uint64_t pack_version(version value) noexcept
{
    std::uint64_t output{};

    output |= static_cast<std::uint64_t>(value.major) << 48;
    output |= static_cast<std::uint64_t>(value.minor) << 32;
    output |= static_cast<std::uint64_t>(value.patch);

    return output;
}

constexpr version unpack_version(std::uint64_t value) noexcept
{
    version output{};

    output.major = static_cast<std::uint16_t>(value >> 48);
    output.minor = static_cast<std::uint16_t>(value >> 32);
    output.patch = static_cast<std::uint32_t>(value);

    return output;
}

constexpr bool operator==(version left, version right) noexcept
{
    return left.major == right.major && left.minor == right.minor && left.patch == right.patch;
}

constexpr bool operator!=(version left, version right) noexcept
{
    return !(left == right);
}

constexpr bool operator>(version left, version right) noexcept
{
    return left.major > right.major || (left.major == right.major && (left.minor > right.minor || (left.minor == right.minor && left.patch > right.patch)));
}

constexpr bool operator<(version left, version right) noexcept
{
    return left.major < right.major || (left.major == right.major && (left.minor < right.minor || (left.minor == right.minor && left.patch < right.patch)));
}

constexpr bool operator>=(version left, version right) noexcept
{
    return left > right || left == right;
}

constexpr bool operator<=(version left, version right) noexcept
{
    return left < right || left == right;
}

}

}

#endif
