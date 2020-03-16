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

struct load_from_file_t{};
static constexpr load_from_file_t load_from_file{};

struct load_from_memory_t{};
static constexpr load_from_memory_t load_from_memory{};

template<class T>
struct is_unbounded_array: std::false_type {};
template<class T>
struct is_unbounded_array<T[]> : std::true_type {};

template<class T>
struct is_bounded_array: std::false_type {};
template<class T, std::size_t N>
struct is_bounded_array<T[N]> : std::true_type {};

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
