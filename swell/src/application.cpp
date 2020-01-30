#include "application.hpp"

#include <algorithm>
#include <stdexcept>

#include <portaudio.h>

namespace swl
{

application::application()
{
    if(const auto err{Pa_Initialize()}; err != paNoError)
        throw std::runtime_error{"Can not initialize audio. " + std::string{Pa_GetErrorText(err)}};

    m_physical_devices.reserve(static_cast<std::size_t>(Pa_GetDeviceCount()));
    for(std::int32_t i{}; i < Pa_GetDeviceCount(); ++i)
    {
        const PaDeviceInfo* device{Pa_GetDeviceInfo(i)};

        physical_device physical_device
        {
            i,
            static_cast<std::uint32_t>(device->maxOutputChannels),
            physical_device::time_type{device->defaultLowOutputLatency},
            physical_device::time_type{device->defaultHighOutputLatency},
            static_cast<std::uint32_t>(device->defaultSampleRate),
            device->name
        };

        m_physical_devices.push_back(std::move(physical_device));
    }
}

application::~application()
{
    if(m_free)
        Pa_Terminate();
}

application::application(application&& other) noexcept
:m_physical_devices{std::move(other.m_physical_devices)}
,m_free{std::exchange(other.m_free, false)}
{

}

application& application::operator=(application&& other) noexcept
{
    m_physical_devices = std::move(other.m_physical_devices);
    m_free = std::exchange(other.m_free, m_free);

    return *this;
}

const physical_device& application::default_device() const noexcept
{
    return m_physical_devices[Pa_GetDefaultOutputDevice()];
}

}
