#include "power.hpp"

#include <SDL_power.h>

namespace apr
{

power_status get_power_status(application& app [[maybe_unused]]) noexcept
{
    int time{};
    int remaining{};

    const SDL_PowerState state{SDL_GetPowerInfo(&time, &remaining)};

    power_status output{};
    output.state = static_cast<power_state>(state);

    if(time != -1) //Both parameters will return -1 if a value can't be determined, or if not running on a battery.
    {
        battery_status battery{};
        battery.remaining = static_cast<double>(remaining) / 100.0;
        battery.remaining_time = std::chrono::seconds{time};

        output.battery = battery;
    }

    return output;
}

}
