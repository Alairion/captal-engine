#ifndef APYRE_CONFIG_HPP_INCLUDED
#define APYRE_CONFIG_HPP_INCLUDED

#include <captal_foundation/config.hpp>

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

using namespace cpt::foundation;
using namespace cpt::foundation::enum_operations;

}

#endif
