#ifndef MY_PROJECT_CLOCK_HPP_INCLUDED
#define MY_PROJECT_CLOCK_HPP_INCLUDED

#include "config.hpp"

#include <chrono>

namespace mpr
{

namespace clock
{

using second_t = std::chrono::duration<double>;
using minute_t = std::chrono::duration<double, std::ratio<1, 60>>;
using hour_t = std::chrono::duration<double, std::ratio<1, 3600>>;
using day_t = std::chrono::duration<double, std::ratio<1, 86400>>;
using duration_t = second_t;

}

}

#endif
