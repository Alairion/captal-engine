#include "inputs.hpp"

#include <stdexcept>
#include <utility>

#include <SDL.h>

#include "window.hpp"

namespace apr
{

scancode to_scancode(application& application [[maybe_unused]], keycode key) noexcept
{
    return static_cast<scancode>(SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(key)));
}

keycode to_keycode(application& application [[maybe_unused]], scancode scan) noexcept
{
    return static_cast<keycode>(SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(scan)));
}

std::string to_string(application& application [[maybe_unused]], keycode key)
{
    return SDL_GetKeyName(static_cast<SDL_Keycode>(key));
}

std::string to_string(application& application [[maybe_unused]], scancode scan)
{
    return SDL_GetScancodeName(static_cast<SDL_Scancode>(scan));
}

void begin_text_input(application& application [[maybe_unused]]) noexcept
{
    SDL_StartTextInput();
}

void end_text_input(application& application [[maybe_unused]]) noexcept
{
    SDL_StopTextInput();
}

bool is_text_input_active(application& application [[maybe_unused]]) noexcept
{
    return SDL_IsTextInputActive() == SDL_TRUE;
}

std::uint32_t get_keyboard_focus(application& application [[maybe_unused]]) noexcept
{
    if(auto* window{SDL_GetKeyboardFocus()}; window)
    {
        return SDL_GetWindowID(window);
    }

    return 0;
}

mouse_state get_mouse_state(application& application [[maybe_unused]]) noexcept
{
    int x;
    int y;
    const auto state = static_cast<mouse_button>(SDL_GetMouseState(&x, &y));

    return mouse_state{x, y, state};
}

mouse_state get_global_mouse_state(application& application [[maybe_unused]]) noexcept
{
    int x;
    int y;
    const auto state = static_cast<mouse_button>(SDL_GetGlobalMouseState(&x, &y));

    return mouse_state{x, y, state};
}

void move_mouse(application& application, std::int32_t x, std::int32_t y) noexcept
{
    const auto state{get_global_mouse_state(application)};

    SDL_WarpMouseGlobal(state.x + x, state.y + y);
}

void move_mouse_to(application& application [[maybe_unused]], std::int32_t x, std::int32_t y) noexcept
{
    SDL_WarpMouseGlobal(x, y);
}

void move_mouse_to(application& application [[maybe_unused]], window& window, std::int32_t x, std::int32_t y) noexcept
{
    SDL_WarpMouseInWindow(SDL_GetWindowFromID(window.id()), x, y);
}

void enable_relative_mouse(application& application [[maybe_unused]]) noexcept
{
    SDL_SetRelativeMouseMode(SDL_TRUE);
}

void disable_relative_mouse(application& application [[maybe_unused]]) noexcept
{
    SDL_SetRelativeMouseMode(SDL_FALSE);
}

std::uint32_t get_mouse_focus(application& application [[maybe_unused]]) noexcept
{
    if(auto* window{SDL_GetMouseFocus()}; window)
    {
        return SDL_GetWindowID(window);
    }

    return 0;
}

cursor::cursor(application& application [[maybe_unused]], const std::uint8_t* rgba, std::uint32_t width, std::uint32_t height, std::uint32_t hot_x, std::uint32_t hot_y)
{
    SDL_Surface* surface{SDL_CreateRGBSurfaceFrom(const_cast<std::uint8_t*>(rgba), static_cast<int>(width), static_cast<int>(height), 32, width * 4, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF)};
    if(!surface)
        throw std::runtime_error{"Can not create cursor surface. " + std::string{SDL_GetError()}};

    m_cursor = SDL_CreateColorCursor(surface, hot_x, hot_y);
    SDL_FreeSurface(surface);

    if(!m_cursor)
        throw std::runtime_error{"Can not create cursor." + std::string{SDL_GetError()}};
}

cursor::cursor(application& application [[maybe_unused]], system_cursor type)
:m_cursor{SDL_CreateSystemCursor(static_cast<SDL_SystemCursor>(type))}
{
    if(!m_cursor)
        throw std::runtime_error{"Can not create cursor." + std::string{SDL_GetError()}};
}

cursor::~cursor()
{
    if(m_cursor)
    {
        SDL_FreeCursor(m_cursor);
    }
}

cursor::cursor(cursor&& other) noexcept
:m_cursor{std::exchange(other.m_cursor, nullptr)}
{

}

cursor& cursor::operator=(cursor&& other) noexcept
{
    std::swap(m_cursor, other.m_cursor);

    return *this;
}

void cursor::activate() noexcept
{
    SDL_SetCursor(m_cursor);
}

void hide_cursor(application& application [[maybe_unused]])
{
    if(SDL_ShowCursor(SDL_DISABLE) < 0)
        throw std::runtime_error{"Can not hide cursor." + std::string{SDL_GetError()}};
}

void show_cursor(application& application [[maybe_unused]])
{
    if(SDL_ShowCursor(SDL_ENABLE) < 0)
        throw std::runtime_error{"Can not show cursor." + std::string{SDL_GetError()}};
}

bool is_cursor_visible(application& application [[maybe_unused]]) noexcept
{
    return SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE;
}

}
