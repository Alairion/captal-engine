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
