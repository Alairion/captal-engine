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
