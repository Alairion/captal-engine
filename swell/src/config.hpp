#ifndef SWELL_CONFIG_HPP_INCLUDED
#define SWELL_CONFIG_HPP_INCLUDED

#include <cstdint>
#include <type_traits>

namespace swl
{

struct load_from_file_t{};
static constexpr load_from_file_t load_from_file{};

struct load_from_memory_t{};
static constexpr load_from_memory_t load_from_memory{};

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
