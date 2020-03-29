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
//21 -> 5: night
//5 -> 6: moonset

//zangle = 0.16 + t*π - 0.16*2t
//xyangle = (-π)/(16) + (2*π*t)/(16)
//sunpos = cos(zangle) * cos(xyangle), sin(xyangle), sin(zangle) * cos(xyangle)

template<typename Rep, typename Period>
std::pair<directional_light, directional_light> compute_lights(const std::chrono::duration<Rep, Period>& total_time)
{
    const float time{daytime<time::hour>(total_time).count()};
    std::pair<directional_light, directional_light> output{};

    if(time >= 6.0f && time <= 7.0f) //sunrise
    {

    }
    else if(time >= 7.0f && time <= 19.0f) //day
    {
        const float normalized_time{(time - 7.0f) * (19.0f - 7.0f)};
        const float z_angle{0.16f + (normalized_time * cpt::pi<float>) - (0.16f * normalized_time)};
        const float xy_angle{(-cpt::pi<float> / 16.0f) + (2 * cpt::pi<float> * normalized_time / 16.0f)};
        const glm::vec4 sun_position{std::cos(z_angle) * std::cos(xy_angle), std::sin(xy_angle), std::sin(z_angle) * std::cos(xy_angle), 0.0f};

        output.first.direction = -sun_position;
        output.first.ambiant   = glm::vec4{0.35f, 0.35f, 0.35f, 1.0f};
        output.first.diffuse   = glm::vec4{0.65f, 0.65f, 0.65f, 1.0f};
        output.first.specular  = glm::vec4{0.50f, 0.50f, 0.50f, 1.0f};
    }
    else if(time >= 19.0f && time <= 20.0f) //sunset
    {

    }
    else if(time >= 20.0f && time <= 21.0f) //moonrise
    {

    }
    else if(time >= 21.0f && time <= 5.0f) //night
    {

    }
    else if(time >= 5.0f && time <= 6.0f) //moonset
    {

    }

    return output;
}

}

#endif
