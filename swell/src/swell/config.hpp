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
using clock   = std::chrono::steady_clock;

enum class sample_format : std::uint32_t
{
    float32 = 0x01,
    int32   = 0x02,
    int24   = 0x04,
    int16   = 0x08,
    int8    = 0x10,
    uint8   = 0x20,
};

enum class stream_callback_result : std::uint32_t
{
    play = 0,
    stop = 1,
    abort = 2
};

}

#endif
