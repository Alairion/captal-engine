#ifndef CAPTAL_APPLICATION_HPP_INCLUDED
#define CAPTAL_APPLICATION_HPP_INCLUDED

#include "config.hpp"

#include <apyre/application.hpp>
#include <swell/application.hpp>
#include <tephra/application.hpp>

namespace cpt
{

class CAPTAL_API application
{
public:
    application(const std::string& application_name, tph::version version);
    ~application() = default;
    application(const application&) = delete;
    application& operator=(const application&) = delete;
    application(application&& other) noexcept = default;
    application& operator=(application&& other) noexcept = default;

    apr::application& system_application() noexcept
    {
        return m_system_application;
    }

    const apr::application& system_application() const noexcept
    {
        return m_system_application;
    }

    swl::application& audio_application() noexcept
    {
        return m_audio_application;
    }

    const swl::application& audio_application() const noexcept
    {
        return m_audio_application;
    }

    tph::application& graphics_application() noexcept
    {
        return m_graphics_application;
    }

    const tph::application& graphics_application() const noexcept
    {
        return m_graphics_application;
    }

private:
    apr::application m_system_application{};
    swl::application m_audio_application{};
    tph::application m_graphics_application{};
};

}

#endif
