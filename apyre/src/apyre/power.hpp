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
