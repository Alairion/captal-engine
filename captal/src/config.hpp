#ifndef CAPTAL_CONFIG_HPP_INCLUDED
#define CAPTAL_CONFIG_HPP_INCLUDED

#include <captal_foundation/config.hpp>

#ifdef _WIN32
    #ifdef CAPTAL_SHARED_BUILD
        #define CAPTAL_API __declspec(dllexport)
    #else
        #define CAPTAL_API __declspec(dllimport)
    #endif
#else
    #define CAPTAL_API
#endif

#endif
