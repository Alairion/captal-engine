#ifndef SWELL_CONFIG_HPP_INCLUDED
#define SWELL_CONFIG_HPP_INCLUDED

#include <chrono>

#include <captal_foundation/base.hpp>
#include <captal_foundation/enum_operations.hpp>

#ifdef _WIN32
    #if defined(SWELL_SHARED_BUILD)
        #define SWELL_API __declspec(dllexport)
    #elif !defined(SWELL_STATIC_BUILD)
        #define SWELL_API __declspec(dllimport)
    #else
        #define SWELL_API
    #endif
#else
    #define SWELL_API
#endif

namespace swl
{

using namespace cpt::foundation;

using seconds = std::chrono::duration<double>;

}

#endif
