#include "event.hpp"

#include <stdexcept>
#include <algorithm>
#include <iostream>

#include <SDL.h>
#include <SDL_vulkan.h>

#include <captal_foundation/encoding.hpp>

#include "application.hpp"
#include "window.hpp"

namespace apr
{

scancode to_scancode(keycode key) noexcept
{
    return static_cast<scancode>(SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(key)));
}

keycode to_keycode(scancode scan) noexcept
{
    return static_cast<keycode>(SDL_GetScancodeFromKey(static_cast<SDL_Scancode>(scan)));
}

std::string to_string(keycode key)
{
    return SDL_GetKeyName(static_cast<SDL_Keycode>(key));
}

std::string to_string(scancode scan)
{
    return SDL_GetScancodeName(static_cast<SDL_Scancode>(scan));
}

static std::optional<event> translate(const SDL_Event& sdl_event)
{
    if(sdl_event.type == SDL_QUIT)
    {
        return std::make_optional(event{quit_event{}});
    }
    else if(sdl_event.type == SDL_WINDOWEVENT)
    {
        if(sdl_event.window.event == SDL_WINDOWEVENT_CLOSE)
        {
            return std::make_optional(event{window_event{window_event::closed, sdl_event.window.windowID}});
        }
        else if(sdl_event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        {
            window_event output{};
            output.type = window_event::resized;
            output.window = sdl_event.window.windowID;
            output.width = sdl_event.window.data1;
            output.height = sdl_event.window.data2;

            return std::make_optional(event{output});
        }
        else if(sdl_event.window.event == SDL_WINDOWEVENT_MINIMIZED)
        {
            return std::make_optional(event{window_event{window_event::minimized, sdl_event.window.windowID}});
        }
        else if(sdl_event.window.event == SDL_WINDOWEVENT_MAXIMIZED)
        {
            return std::make_optional(event{window_event{window_event::maximized, sdl_event.window.windowID}});
        }
        else if(sdl_event.window.event == SDL_WINDOWEVENT_RESTORED)
        {
            return std::make_optional(event{window_event{window_event::restored, sdl_event.window.windowID}});
        }
        else if(sdl_event.window.event == SDL_WINDOWEVENT_MOVED)
        {
            window_event output{};
            output.type = window_event::moved;
            output.window = sdl_event.window.windowID;
            output.x = sdl_event.window.data1;
            output.y = sdl_event.window.data2;

            return std::make_optional(event{output});
        }
        else if(sdl_event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
        {
            return std::make_optional(event{window_event{window_event::gained_focus, sdl_event.window.windowID}});
        }
        else if(sdl_event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
        {
            return std::make_optional(event{window_event{window_event::lost_focus, sdl_event.window.windowID}});
        }
        else if(sdl_event.window.event == SDL_WINDOWEVENT_ENTER)
        {
            return std::make_optional(event{window_event{window_event::mouse_entered, sdl_event.window.windowID}});
        }
        else if(sdl_event.window.event == SDL_WINDOWEVENT_LEAVE)
        {
            return std::make_optional(event{window_event{window_event::mouse_left, sdl_event.window.windowID}});
        }
    }
    else if(sdl_event.type == SDL_MOUSEMOTION)
    {
        mouse_event output{};
        output.type = mouse_event::moved;
        output.window = sdl_event.motion.windowID;
        output.x = sdl_event.motion.x;
        output.y = sdl_event.motion.y;

        if(sdl_event.motion.state & SDL_BUTTON_LMASK)
        {
            output.button |= mouse_button::left;
        }

        if(sdl_event.motion.state & SDL_BUTTON_RMASK)
        {
            output.button |= mouse_button::right;
        }

        if(sdl_event.motion.state & SDL_BUTTON_MMASK)
        {
            output.button |= mouse_button::middle;
        }

        if(sdl_event.motion.state & SDL_BUTTON_X1MASK)
        {
            output.button |= mouse_button::side1;
        }

        if(sdl_event.motion.state & SDL_BUTTON_X2MASK)
        {
            output.button |= mouse_button::side2;
        }

        return std::make_optional(event{output});
    }
    else if(sdl_event.type == SDL_MOUSEBUTTONDOWN)
    {
        mouse_event output{};
        output.type = mouse_event::button_pressed;
        output.window = sdl_event.button.windowID;
        output.x = sdl_event.button.x;
        output.y = sdl_event.button.y;
        output.clicks = sdl_event.button.clicks;

        if(sdl_event.button.button == SDL_BUTTON_LEFT)
        {
            output.button |= mouse_button::left;
        }
        else if(sdl_event.button.button == SDL_BUTTON_RIGHT)
        {
            output.button |= mouse_button::right;
        }
        else if(sdl_event.button.button == SDL_BUTTON_MIDDLE)
        {
            output.button |= mouse_button::middle;
        }
        else if(sdl_event.button.button == SDL_BUTTON_X1)
        {
            output.button |= mouse_button::side1;
        }
        else if(sdl_event.button.button == SDL_BUTTON_X2)
        {
            output.button |= mouse_button::side2;
        }

        return std::make_optional(event{output});
    }
    else if(sdl_event.type == SDL_MOUSEBUTTONUP)
    {
        mouse_event output{};
        output.type = mouse_event::button_released;
        output.window = sdl_event.button.windowID;
        output.x = sdl_event.button.x;
        output.y = sdl_event.button.y;
        output.clicks = sdl_event.button.clicks;

        if(sdl_event.button.button == SDL_BUTTON_LEFT)
        {
            output.button |= mouse_button::left;
        }
        else if(sdl_event.button.button == SDL_BUTTON_RIGHT)
        {
            output.button |= mouse_button::right;
        }
        else if(sdl_event.button.button == SDL_BUTTON_MIDDLE)
        {
            output.button |= mouse_button::middle;
        }
        else if(sdl_event.button.button == SDL_BUTTON_X1)
        {
            output.button |= mouse_button::side1;
        }
        else if(sdl_event.button.button == SDL_BUTTON_X2)
        {
            output.button |= mouse_button::side2;
        }

        return std::make_optional(event{output});
    }
    else if(sdl_event.type == SDL_MOUSEWHEEL)
    {
        mouse_event output{};
        output.type = mouse_event::wheel_scroll;
        output.window = sdl_event.wheel.windowID;
        output.wheel = sdl_event.wheel.y;

        return std::make_optional(event{output});
    }
    else if(sdl_event.type == SDL_KEYDOWN)
    {
        keyboard_event output{};
        output.type = keyboard_event::key_pressed;
        output.window = sdl_event.key.windowID;
        output.scan = static_cast<scancode>(sdl_event.key.keysym.scancode);
        output.key = static_cast<keycode>(sdl_event.key.keysym.sym);
        output.modifiers = static_cast<key_modifier>(sdl_event.key.keysym.mod);

        return std::make_optional(event{output});
    }
    else if(sdl_event.type == SDL_KEYUP)
    {
        keyboard_event output{};
        output.type = keyboard_event::key_released;
        output.window = sdl_event.key.windowID;
        output.scan = static_cast<scancode>(sdl_event.key.keysym.scancode);
        output.key = static_cast<keycode>(sdl_event.key.keysym.sym);
        output.modifiers = static_cast<key_modifier>(sdl_event.key.keysym.mod);

        return std::make_optional(event{output});
    }
    else if(sdl_event.type == SDL_TEXTINPUT)
    {
        text_event output{};
        output.type = text_event::text_entered;
        output.window = sdl_event.text.windowID;
        output.text = sdl_event.text.text;

        return std::make_optional(event{output});
    }

    return std::nullopt;
}

std::optional<event> event_queue::next(event_mode mode)
{
    return next(mode, 0);
}

std::optional<event> event_queue::next(window& window, event_mode mode)
{
    const auto event{next(mode, window.id())};

    if(event)
    {
        if(std::holds_alternative<apr::window_event>(*event))
        {
            const auto& window_event{std::get<apr::window_event>(*event)};

            if(window_event.type == apr::window_event::resized)
            {
                int width{};
                int height{};
                SDL_Vulkan_GetDrawableSize(window.m_window, &width, &height);

                std::uint64_t surface_size{};
                surface_size |= static_cast<std::uint64_t>(width);
                surface_size |= static_cast<std::uint64_t>(height) << 32ull;

                window.m_surface_size->store(surface_size, std::memory_order_release);
            }
        }
    }

    return event;
}

void event_queue::flush(event_mode mode, std::uint32_t id)
{
    const auto push = [this](event new_event)
    {
        if(const auto it{m_events.find(event_window_id(new_event))}; it != std::end(m_events))
        {
            auto& buffer{it->second};

            buffer.insert(std::begin(buffer), new_event);
        }
    };

    if(mode == event_mode::poll)
    {
        SDL_Event sdl_event{};
        while(SDL_PollEvent(&sdl_event))
        {
            if(const auto new_event{translate(sdl_event)}; new_event)
            {
                push(*new_event);
            }
        }
    }
    else if(mode == event_mode::wait)
    {
        while(true)
        {
            SDL_Event sdl_event{};
            if(!SDL_WaitEvent(&sdl_event))
                throw std::runtime_error{"Can not wait for event. " + std::string{SDL_GetError()}};

            if(const auto new_event{translate(sdl_event)}; new_event)
            {
                push(*new_event);

                if(event_window_id(*new_event) == id)
                {
                    return;
                }
            }
        }
    }
}

std::optional<event> event_queue::next(event_mode mode, std::uint32_t id)
{
    auto& buffer{m_events.find(id)->second};

    if(std::empty(buffer))
    {
        flush(mode, id);
    }

    if(!std::empty(buffer))
    {
        const auto output{buffer.back()};

        buffer.pop_back();

        return std::make_optional(output);
    }

    return std::nullopt;
}

event_iterator::event_iterator(application& application, event_mode mode)
:m_queue{&application.event_queue()}
,m_mode{mode}
{
    operator++();
}

event_iterator::event_iterator(application& application, window& window, event_mode mode)
:m_queue{&application.event_queue()}
,m_mode{mode}
,m_window{&window}
{
    operator++();
}

}
