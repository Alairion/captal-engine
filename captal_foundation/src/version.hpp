#ifndef CAPTAL_FOUNDATION_VERSION_HPP_INCLUDED
#define CAPTAL_FOUNDATION_VERSION_HPP_INCLUDED

#include <cstdint>
#include <cstring>

namespace cpt
{

inline namespace foundation
{

struct alignas(std::uint64_t) version
{
    std::uint16_t major; //No default initializers because version must be trivial
    std::uint16_t minor;
    std::uint32_t patch;
};

inline std::uint64_t from_version(version value) noexcept
{
    std::uint64_t output{};
    std::memcpy(&output, &value, sizeof(version));

    return output;
}

inline version to_version(std::uint64_t value) noexcept
{
    version output{};
    std::memcpy(&output, &value, sizeof(std::uint64_t));

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
