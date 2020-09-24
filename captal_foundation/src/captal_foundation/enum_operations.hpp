#ifndef CAPTAL_FOUNDATION_ENUMS_OPERATIONS_HPP_INCLUDED
#define CAPTAL_FOUNDATION_ENUMS_OPERATIONS_HPP_INCLUDED

#include <type_traits>

namespace cpt
{

inline namespace foundation
{

inline namespace enum_operations
{

template<typename E>
struct enable_enum_operations
{
    static constexpr bool value{false};
};

template<typename E> requires(enable_enum_operations<E>::value)
constexpr E operator&(E left, E right) noexcept
{
    return static_cast<E>(static_cast<std::underlying_type_t<E>>(left) & static_cast<std::underlying_type_t<E>>(right));
}

template<typename E> requires(enable_enum_operations<E>::value)
constexpr E& operator&=(E& left, E right) noexcept
{
    left = left & right;

    return left;
}

template<typename E> requires(enable_enum_operations<E>::value)
constexpr E operator|(E left, E right) noexcept
{
    return static_cast<E>(static_cast<std::underlying_type_t<E>>(left) | static_cast<std::underlying_type_t<E>>(right));
}

template<typename E> requires(enable_enum_operations<E>::value)
constexpr E& operator|=(E& left, E right) noexcept
{
    left = left | right;

    return left;
}

template<typename E> requires(enable_enum_operations<E>::value)
constexpr E operator~(E right) noexcept
{
    return static_cast<E>(~static_cast<std::underlying_type_t<E>>(right));
}

}

}

}

#endif
