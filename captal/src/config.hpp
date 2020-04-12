#ifndef CAPTAL_CONFIG_HPP_INCLUDED
#define CAPTAL_CONFIG_HPP_INCLUDED

#include <cstdint>
#include <type_traits>

#ifdef _WIN32
    #ifdef CAPTAL_SHARED_BUILD
        #define CAPTAL_API __declspec(dllexport)
    #else
        #define CAPTAL_API __declspec(dllimport)
    #endif
#else
    #define CAPTAL_API
#endif

namespace cpt
{

template<typename T>
static constexpr T pi{static_cast<T>(3.141592653589793238462643383279)};

enum class endian : std::uint32_t
{
#ifdef _WIN32
    little = 0,
    big    = 1,
    native = little
#else
    little = __ORDER_LITTLE_ENDIAN__,
    big    = __ORDER_BIG_ENDIAN__,
    native = __BYTE_ORDER__
#endif
};

inline namespace enum_operations
{

template<typename E>
struct enable_enum_operations
{
    static constexpr bool value{false};
};

template<typename E, typename = typename std::enable_if<enable_enum_operations<E>::value>::type>
constexpr E operator&(E left, E right) noexcept
{
    return static_cast<E>(static_cast<std::underlying_type_t<E>>(left) & static_cast<std::underlying_type_t<E>>(right));
}

template<typename E, typename = typename std::enable_if<enable_enum_operations<E>::value>::type>
constexpr E& operator&=(E& left, E right) noexcept
{
    return reinterpret_cast<E&>(reinterpret_cast<std::underlying_type_t<E>&>(left) &= static_cast<std::underlying_type_t<E>>(right));
}

template<typename E, typename = typename std::enable_if<enable_enum_operations<E>::value>::type>
constexpr E operator|(E left, E right) noexcept
{
    return static_cast<E>(static_cast<std::underlying_type_t<E>>(left) | static_cast<std::underlying_type_t<E>>(right));
}

template<typename E, typename = typename std::enable_if<enable_enum_operations<E>::value>::type>
constexpr E& operator|=(E& left, E right) noexcept
{
    return reinterpret_cast<E&>(reinterpret_cast<std::underlying_type_t<E>&>(left) |= static_cast<std::underlying_type_t<E>>(right));
}

template<typename E, typename = typename std::enable_if<enable_enum_operations<E>::value>::type>
constexpr E operator~(E right) noexcept
{
    return static_cast<E>(~static_cast<std::underlying_type_t<E>>(right));
}

}

}

#endif
