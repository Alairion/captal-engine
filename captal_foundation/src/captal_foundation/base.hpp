#ifndef CAPTAL_FOUNDATION_BASE_HPP_INCLUDED
#define CAPTAL_FOUNDATION_BASE_HPP_INCLUDED

#include <cstdint>
#include <type_traits>

#ifdef _WIN32
    #ifdef CAPTAL_FOUNDATION_SHARED_BUILD
        #define CAPTAL_FOUNDATION_API __declspec(dllexport)
    #else
        #define CAPTAL_FOUNDATION_API __declspec(dllimport)
    #endif
#else
    #define CAPTAL_FOUNDATION_API
#endif

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
