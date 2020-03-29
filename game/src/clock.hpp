#ifndef MY_PROJECT_CLOCK_HPP_INCLUDED
#define MY_PROJECT_CLOCK_HPP_INCLUDED

#include "config.hpp"

#include <chrono>
#include <cmath>

namespace mpr
{

namespace time
{

template<typename T>
using second_t = std::chrono::duration<T>;
template<typename T>
using minute_t = std::chrono::duration<T, std::ratio<60>>;
template<typename T>
using hour_t = std::chrono::duration<T, std::ratio<3600>>;
template<typename T>
using day_t = std::chrono::duration<T, std::ratio<86400>>;

using second = second_t<float>;
using minute = minute_t<float>;
using hour = hour_t<float>;
using day = day_t<float>;

}

template<typename To, typename Rep, typename Period>
constexpr To daytime(const std::chrono::duration<Rep, Period>& total_time)
{
    return std::chrono::duration_cast<To>(std::chrono::duration_cast<time::day>(total_time) - std::chrono::floor<time::day_t<std::uint64_t>>>(total_time));
}

//6 -> 7: sunrise
//7 -> 19: day
//19 -> 20: sunset
//20 -> 21: moonrise
//21 -> 4: night
//5 -> 6: moonset

//zangle = 0.16 + t*π - 0.16*2t
//xyangle = (-π)/(16) + (2*π*t)/(16)
//sunpos = cos(zangle) * cos(xyangle), sin(xyangle), sin(zangle) * cos(xyangle)

template<typename Rep, typename Period>
std::pair<directional_light, directional_light> compute_lights(const std::chrono::duration<Rep, Period>& total_time)
{
    const time::hour time{daytime<time::hour>(total_time)};
    std::pair<directional_light, directional_light> output{};

    if(time.count() >= 6.0f && time.count() <= 7.0f)
    {

    }
    else if(time.count() >= 7.0f && time.count() <= 19.0f)
    {

    }
    else if(time.count() >= 19.0f && time.count() <= 20.0f)
    {

    }
    else if(time.count() >= 20.0f && time.count() <= 21.0f)
    {

    }
    else if(time.count() >= 21.0f && time.count() <= 4.0f)
    {

    }
    else if(time.count() >= 5.0f && time.count() <= 6.0f)
    {

    }

    return output;
}

}

#endif
