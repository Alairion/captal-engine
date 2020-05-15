#ifndef CAPTAL_FOUNDATION_CONFIG_HPP_INCLUDED
#define CAPTAL_FOUNDATION_CONFIG_HPP_INCLUDED

#include "optional_ref.hpp"
#include "enum_operations.hpp"
#include "version.hpp"
#include "endian.hpp"

namespace cpt
{

inline namespace foundation
{

template<typename T>
inline constexpr T pi{static_cast<T>(3.141592653589793238462643383279)};

}

}

#endif
