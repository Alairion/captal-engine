#ifndef APYRE_CONFIG_HPP_INCLUDED
#define APYRE_CONFIG_HPP_INCLUDED

#include <cstdint>
#include <type_traits>

#ifdef _WIN32
    #ifdef APYRE_SHARED_BUILD
        #define APYRE_API __declspec(dllexport)
    #else
        #define APYRE_API __declspec(dllimport)
    #endif
#else
    #define APYRE_API
#endif

namespace apr
{

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
