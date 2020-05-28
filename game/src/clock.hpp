#ifndef MY_PROJECT_CLOCK_HPP_INCLUDED
#define MY_PROJECT_CLOCK_HPP_INCLUDED

#include "config.hpp"

#include <captal/color.hpp>

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

//7 -> 19: day
//19 -> 20: sunset
//20 -> 21: moonrise
//21 -> 5: night
//5 -> 6: moonset
//6 -> 7: sunrise

//zangle = 0.16 + t*π - 0.16*2t
//xyangle = (-π)/(16) + (2*π*t)/(16)
//sunpos = cos(zangle) * cos(xyangle), sin(xyangle), sin(zangle) * cos(xyangle)

inline constexpr time::hour day_begin{7.0f};
inline constexpr time::hour sunset_begin{19.0f};
inline constexpr time::hour moonrise_begin{20.0f};
inline constexpr time::hour night_begin{21.0f};
inline constexpr time::hour moonset_begin{5.0f};
inline constexpr time::hour sunrise_begin{6.0f};

enum class weather : std::uint32_t
{
    clear
};

template<typename Rep, typename Period>
std::array<directional_light, 2> compute_directional_lights(const std::chrono::duration<Rep, Period>& total_time, weather weather [[maybe_unused]] = weather::clear)
{
    const auto sun_direction = [](float normalized_time) -> glm::vec4
    {
        const float z_angle{0.09f + (normalized_time * cpt::pi<float>) - (0.09f * 2.0f * normalized_time)};
        const float xy_angle{(-cpt::pi<float> / 16.0f) + (2 * cpt::pi<float> * normalized_time / 16.0f)};

        return -glm::vec4{std::cos(z_angle) * std::cos(xy_angle), std::sin(xy_angle), std::sin(z_angle) * std::cos(xy_angle), 0.0f};
    };

    const auto moon_direction = [](float normalized_time) -> glm::vec4
    {
        const float z_angle{0.09f + (normalized_time * cpt::pi<float>) - (0.09f * 2.0f * normalized_time)};
        const float xy_angle{-cpt::pi<float> / 16.0f};

        return -glm::vec4{std::cos(z_angle) * std::cos(xy_angle), std::sin(xy_angle), std::sin(z_angle) * std::cos(xy_angle), 0.0f};
    };

    const auto second_moon_direction = [](float normalized_time) -> glm::vec4
    {
        normalized_time = 1.0f - normalized_time;

        const float z_angle{0.5f + (normalized_time * cpt::pi<float>) - (0.5f * 2.0f * normalized_time)};
        const float xy_angle{cpt::pi<float> / 8.0f};

        return -glm::vec4{std::cos(z_angle) * std::cos(xy_angle), std::sin(xy_angle), std::sin(z_angle) * std::cos(xy_angle), 0.0f};
    };

    const auto normalize_time = [](const time::hour& time, const time::hour& begin, const time::hour& end) -> float
    {
        return (time - begin).count() / (end - begin).count();
    };

    static constexpr glm::vec4 sunset_color{cpt::colors::orange}; //#FFA500
    static constexpr glm::vec4 sunrise_color{cpt::colors::powderblue}; //#B0E0E6
    static constexpr glm::vec4 moon_color{cpt::colors::darkslateblue}; //#48408B
    static constexpr glm::vec4 second_moon_color{cpt::colors::dodgerblue}; //#1E90FF

    //[0] = sun or second moon; [1] = moon
    std::array<directional_light, 2> output{};
    const time::hour time{daytime<time::hour>(total_time)};

    if(time >= day_begin && time < sunset_begin) //Day
    {
        //Sun
        output[0].direction = sun_direction(normalize_time(time, day_begin, sunset_begin));
        output[0].ambient = glm::vec4{0.35f, 0.35f, 0.35f, 1.0f};
        output[0].diffuse = glm::vec4{0.65f, 0.65f, 0.65f, 1.0f};
        output[0].specular = glm::vec4{0.50f, 0.50f, 0.50f, 1.0f};
    }
    else if(time >= sunset_begin && time < moonrise_begin) //Sunset
    {
        const float advance{normalize_time(time, day_begin, sunset_begin)};

        //Sun
        output[0].direction = sun_direction(1.0f);
        output[0].ambient = glm::vec4{0.25f, 0.25f, 0.25f, 1.0f} + glm::vec4{0.10f, 0.10f, 0.10f, 1.0f} * (1.0f - advance);
        output[0].diffuse = static_cast<glm::vec4>(cpt::gradient(glm::vec4{0.65f, 0.65f, 0.65f, 1.0f}, sunset_color * glm::vec4{0.65f, 0.65f, 0.65f, 1.0f}, advance));
        output[0].specular = static_cast<glm::vec4>(cpt::gradient(glm::vec4{0.50f, 0.50f, 0.50f, 1.0f}, sunset_color * glm::vec4{0.50f, 0.50f, 0.50f, 1.0f}, advance));
    }
    else if(time >= moonrise_begin && time < night_begin) //Moonrise
    {
        const float advance{normalize_time(time, moonrise_begin, night_begin)};

        if(advance < 0.5f)
        {
            const float half_advance{(0.5f - advance) * 2.0f};

            //Sun
            output[0].direction = sun_direction(1.0f);
            output[0].ambient = glm::vec4{0.05f, 0.05f, 0.05f, 1.0f} + glm::vec4{0.20f, 0.20f, 0.20f, 1.0f} * half_advance;
            output[0].diffuse = sunset_color * glm::vec4{0.65f, 0.65f, 0.65f, 1.0f} * half_advance;
            output[0].specular = sunset_color * glm::vec4{0.50f, 0.50f, 0.50f, 1.0f} * half_advance;
        }
        else
        {
            const float half_advance{(advance - 0.5f) * 2.0f};

            //Second moon
            output[0].direction = second_moon_direction(0.0f);
            output[0].ambient = glm::vec4{0.05f, 0.05f, 0.05f, 1.0f};
            output[0].diffuse = second_moon_color * glm::vec4{0.10f, 0.10f, 0.10f, 1.0f} * half_advance;
            output[0].specular = second_moon_color * glm::vec4{0.075f, 0.075f, 0.075f, 1.0f} * half_advance;
        }

        //Moon
        output[1].direction = moon_direction(0.0f);
        output[1].ambient = glm::vec4{0.15f, 0.15f, 0.15f, 1.0f} * advance;
        output[1].diffuse = moon_color * glm::vec4{0.35f, 0.35f, 0.35f, 1.0f} * advance;
        output[1].specular = moon_color * glm::vec4{0.25f, 0.25f, 0.25f, 1.0f} * advance;
    }
    else if(time >= night_begin || time < moonset_begin)
    {
        const float advance{normalize_time(time, night_begin, moonset_begin)};

        //Second moon
        output[0].direction = second_moon_direction(advance);
        output[0].ambient = glm::vec4{0.05f, 0.05f, 0.05f, 1.0f};
        output[0].diffuse = second_moon_color * glm::vec4{0.15f, 0.15f, 0.15f, 1.0f};
        output[0].specular = second_moon_color * glm::vec4{0.10f, 0.10f, 0.10f, 1.0f};

        //Moon
        output[1].direction = moon_direction(advance);
        output[1].ambient = glm::vec4{0.15f, 0.15f, 0.15f, 1.0f};
        output[1].diffuse = moon_color * glm::vec4{0.35f, 0.35f, 0.35f, 1.0f};
        output[1].specular = moon_color * glm::vec4{0.25f, 0.25f, 0.25f, 1.0f};
    }
    else if(time >= moonset_begin && time < sunrise_begin)
    {
        const float advance{normalize_time(time, moonset_begin, sunrise_begin)};

        if(advance < 0.5f)
        {
            const float half_advance{(0.5f - advance) * 2.0f};

            //Second moon
            output[0].direction = second_moon_direction(0.0f);
            output[0].ambient = glm::vec4{0.05f, 0.05f, 0.05f, 1.0f};
            output[0].diffuse = second_moon_color * glm::vec4{0.10f, 0.10f, 0.10f, 1.0f} * half_advance;
            output[0].specular = second_moon_color * glm::vec4{0.075f, 0.075f, 0.075f, 1.0f} * half_advance;
        }
        else
        {
            const float half_advance{(advance - 0.5f) * 2.0f};

            //Sun
            output[0].direction = sun_direction(1.0f);
            output[0].ambient = glm::vec4{0.05f, 0.05f, 0.05f, 1.0f} + glm::vec4{0.20f, 0.20f, 0.20f, 1.0f} * half_advance;
            output[0].diffuse = sunrise_color * glm::vec4{0.65f, 0.65f, 0.65f, 1.0f} * half_advance;
            output[0].specular = sunrise_color * glm::vec4{0.50f, 0.50f, 0.50f, 1.0f} * half_advance;
        }

        //Moon
        output[1].direction = moon_direction(1.0f);
        output[1].ambient = glm::vec4{0.15f, 0.15f, 0.15f, 1.0f} * (1.0f - advance);
        output[1].diffuse = moon_color * glm::vec4{0.35f, 0.35f, 0.35f, 1.0f} * (1.0f - advance);
        output[1].specular = moon_color * glm::vec4{0.25f, 0.25f, 0.25f, 1.0f} * (1.0f - advance);
    }
    else if(time >= sunrise_begin && time < day_begin)
    {
        const float advance{normalize_time(time, day_begin, sunset_begin)};

        //Sun
        output[0].direction = sun_direction(1.0f);
        output[0].ambient = glm::vec4{0.25f, 0.25f, 0.25f, 1.0f} + glm::vec4{0.10f, 0.10f, 0.10f, 1.0f} * advance;
        output[0].diffuse = static_cast<glm::vec4>(cpt::gradient(sunrise_color * glm::vec4{0.65f, 0.65f, 0.65f, 1.0f}, glm::vec4{0.65f, 0.65f, 0.65f, 1.0f}, advance));
        output[0].specular = static_cast<glm::vec4>(cpt::gradient(sunrise_color * glm::vec4{0.50f, 0.50f, 0.50f, 1.0f}, glm::vec4{0.50f, 0.50f, 0.50f, 1.0f}, advance));
    }

    return output;
}

template<typename Rep, typename Period>
std::uint32_t directional_light_count(const std::chrono::duration<Rep, Period>& total_time)
{
    const time::hour time{daytime<time::hour>(total_time)};

    if(time >= moonrise_begin || time < sunrise_begin)
    {
        return 2;
    }

    return 1;
}

}

#endif
