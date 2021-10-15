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

        SDL_DisplayMode mode{};
        SDL_GetDesktopDisplayMode(i, &mode);

        float hdpi{};
        float vdpi{};
        if(SDL_GetDisplayDPI(i, nullptr, &hdpi, &vdpi))
        {
            hdpi = 96.0f;
            vdpi = 96.0f;
        }

        auto& monitor{m_monitors.emplace_back()};
        monitor.m_id             = i;
        monitor.m_x              = rect.x;
        monitor.m_y              = rect.y;
        monitor.m_width          = rect.w;
        monitor.m_height         = rect.h;
        monitor.m_horizontal_dpi = static_cast<double>(hdpi);
        monitor.m_vertical_dpi   = static_cast<double>(vdpi);
        monitor.m_frequency      = static_cast<double>(mode.refresh_rate);
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
