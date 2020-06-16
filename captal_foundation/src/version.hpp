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
