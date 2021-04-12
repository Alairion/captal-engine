#include "application.hpp"

#include <algorithm>
#include <stdexcept>

#include <SDL.h>
#include <SDL_syswm.h>

namespace apr
{

static application_extension filter_instance_extensions(application_extension extensions)
{
#ifdef SDL_VIDEO_DRIVER_WINDOWS
    return extensions;
#else
    return application_extension::none;
#endif
}

application::application(application_extension extensions)
:m_event_queue{std::make_unique<apr::event_queue>()}
,m_extensions{filter_instance_extensions(extensions)}
,m_free{true}
{
    m_event_queue->register_window(0);

    SDL_SetMainReady();

    if(SDL_Init(SDL_INIT_VIDEO))
        throw std::runtime_error{"Can not init SDL video. " + std::string{SDL_GetError()}};

    for(std::int32_t i{}; i < SDL_GetNumVideoDisplays(); ++i)
    {
        SDL_Rect rect{};
        SDL_GetDisplayBounds(i, &rect);

        float hdpi{};
        float vdpi{};
        if(!SDL_GetDisplayDPI(i, nullptr, &hdpi, &vdpi))
        {
            hdpi = 1.0f;
            vdpi = 1.0f;
        }

        auto& monitor{m_monitors.emplace_back()};
        monitor.m_id             = i;
        monitor.m_x              = rect.x;
        monitor.m_y              = rect.y;
        monitor.m_width          = rect.w;
        monitor.m_height         = rect.h;
        monitor.m_horizontal_dpi = static_cast<double>(hdpi);
        monitor.m_vertical_dpi   = static_cast<double>(vdpi);
        monitor.m_name           = SDL_GetDisplayName(i);
    }

    SDL_EventState(SDL_DROPBEGIN, SDL_IGNORE);
    SDL_EventState(SDL_DROPFILE, SDL_IGNORE);
    SDL_EventState(SDL_DROPTEXT, SDL_IGNORE);
    SDL_EventState(SDL_DROPCOMPLETE, SDL_IGNORE);
}

application::~application()
{
    if(m_free)
    {
        SDL_Quit();
    }
}

application::application(application&& other) noexcept
:m_event_queue{std::move(other.m_event_queue)}
,m_monitors{std::move(other.m_monitors)}
,m_extensions{other.m_extensions}
,m_free{std::exchange(other.m_free, false)}
{

}

application& application::operator=(application&& other) noexcept
{
    m_event_queue = std::move(other.m_event_queue);
    m_monitors = std::move(other.m_monitors);
    m_extensions = other.m_extensions;
    m_free = std::exchange(other.m_free, m_free);

    return *this;
}

}
