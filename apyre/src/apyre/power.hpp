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

#ifndef APYRE_POWER_HPP_INCLUDED
#define APYRE_POWER_HPP_INCLUDED

#include "config.hpp"

#include <chrono>
#include <optional>

namespace apr
{

class application;

enum class power_state : std::uint32_t
{
    unknown = 0,    //Can not determine power status
    on_battery = 1, //Not plugged in, running on the battery
    no_battery = 2, //Plugged in, no battery available
    charging = 3,   //Plugged in, charging battery
    charged = 4,    //Plugged in, battery charged
};

struct battery_status
{
    double remaining{};
    std::chrono::seconds remaining_time{};
};

struct power_status
{
    power_state state{};
    std::optional<battery_status> battery{};
};

APYRE_API power_status get_power_status(application& app) noexcept;

}

#endif
