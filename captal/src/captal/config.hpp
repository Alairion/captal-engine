#ifndef CAPTAL_CONFIG_HPP_INCLUDED
#define CAPTAL_CONFIG_HPP_INCLUDED

#include <captal_foundation/base.hpp>
#include <captal_foundation/enum_operations.hpp>
#include <captal_foundation/version.hpp>

#ifdef _WIN32
    #if defined(CAPTAL_SHARED_BUILD)
        #define CAPTAL_API __declspec(dllexport)
    #elif !defined(CAPTAL_STATIC_BUILD)
        #define CAPTAL_API __declspec(dllimport)
    #else
        #define CAPTAL_API
    #endif
#else
    #define CAPTAL_API
#endif

namespace cpt
{

#ifdef CAPTAL_DEBUG
inline constexpr bool debug_enabled{true};
#else
inline constexpr bool debug_enabled{false};
#endif

}

#endif
