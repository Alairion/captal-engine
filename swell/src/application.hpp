#ifndef SWELL_APPLICATION_HPP_INCLUDED
#define SWELL_APPLICATION_HPP_INCLUDED

#include "config.hpp"

#include <vector>

#include "physical_device.hpp"

namespace swl
{

class application
{
public:
    application();
    ~application();
    application(const application&) = delete;
    application& operator=(const application&) = delete;
    application(application&& other) noexcept;
    application& operator=(application&& other) noexcept;

    const std::vector<physical_device>& enumerate_physical_devices() const noexcept
    {
        return m_physical_devices;
    }

    const physical_device& default_device() const noexcept;

private:
    std::vector<physical_device> m_physical_devices{};
    bool m_free{true};
};

}

#endif
