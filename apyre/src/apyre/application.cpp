#include "application.hpp"

#include <algorithm>
#include <stdexcept>

#include <SDL.h>

namespace apr
{

application::application()
:m_event_queue{std::make_unique<apr::event_queue>()}
,m_free{true}
{
    SDL_SetMainReady();

    if(SDL_Init(SDL_INIT_VIDEO))
        throw std::runtime_error{"Can not init SDL video. " + std::string{SDL_GetError()}};

    for(std::int32_t i{}; i < SDL_GetNumVideoDisplays(); ++i)
    {
        SDL_Rect rect{};
        SDL_GetDisplayBounds(i, &rect);

        m_monitors.emplace_back(i, rect.x, rect.y, rect.w, rect.h, SDL_GetDisplayName(i));
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
,m_free{std::exchange(other.m_free, false)}
{

}

application& application::operator=(application&& other) noexcept
{
    m_event_queue = std::move(other.m_event_queue);
    m_monitors = std::move(other.m_monitors);
    m_free = std::exchange(other.m_free, m_free);

    return *this;
}

}
