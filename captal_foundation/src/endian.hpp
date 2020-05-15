#ifndef CAPTAL_FOUNDATION_ENDIAN_HPP_INCLUDED
#define CAPTAL_FOUNDATION_ENDIAN_HPP_INCLUDED

#include <cstdint>

namespace cpt
{

inline namespace foundation
{

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

}

}

#endif
