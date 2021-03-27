#ifndef APYRE_CONFIG_HPP_INCLUDED
#define APYRE_CONFIG_HPP_INCLUDED

#include <captal_foundation/base.hpp>
#include <captal_foundation/enum_operations.hpp>

#ifdef _WIN32
    #if defined(APYRE_SHARED_BUILD)
        #define APYRE_API __declspec(dllexport)
    #elif !defined(APYRE_STATIC_BUILD)
        #define APYRE_API __declspec(dllimport)
    #else
        #define APYRE_API
    #endif
#else
    #define APYRE_API
#endif

namespace apr
{

using namespace cpt::foundation;

}

#endif
