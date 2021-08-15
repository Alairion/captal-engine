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

    m_host_apis.reserve(static_cast<std::size_t>(Pa_GetHostApiCount()));
    for(std::int32_t i{}; i < Pa_GetHostApiCount(); ++i)
    {
        const PaHostApiInfo* api{Pa_GetHostApiInfo(i)};

        m_host_apis.emplace_back(i, api->defaultOutputDevice == paNoDevice ? std::nullopt : std::make_optional(api->defaultOutputDevice), api->name);
    }

    m_physical_devices.reserve(static_cast<std::size_t>(Pa_GetDeviceCount()));
    for(std::int32_t i{}; i < Pa_GetDeviceCount(); ++i)
    {
        const PaDeviceInfo* device{Pa_GetDeviceInfo(i)};

        m_physical_devices.emplace_back(i, static_cast<std::uint32_t>(device->hostApi),
                                        static_cast<std::uint32_t>(device->maxOutputChannels),
                                        seconds{device->defaultLowOutputLatency}, seconds{device->defaultHighOutputLatency},
                                        static_cast<std::uint32_t>(device->defaultSampleRate), device->name);
    }
}

application::~application()
{
    if(m_free)
    {
        Pa_Terminate();
    }
}

application::application(application&& other) noexcept
:m_host_apis{std::move(other.m_host_apis)}
,m_physical_devices{std::move(other.m_physical_devices)}
,m_free{std::exchange(other.m_free, false)}
{

}

application& application::operator=(application&& other) noexcept
{
    m_host_apis = std::move(other.m_host_apis);
    m_physical_devices = std::move(other.m_physical_devices);
    m_free = std::exchange(other.m_free, m_free);

    return *this;
}

const host_api& application::default_host_api() const noexcept
{
    return m_host_apis[Pa_GetDefaultHostApi()];
}

const physical_device& application::default_output_device() const noexcept
{
    return m_physical_devices[Pa_GetDefaultOutputDevice()];
}

}
