#include "window.hpp"

#include <algorithm>
#include <stdexcept>

#include <SDL.h>
#include <SDL_vulkan.h>

#include "application.hpp"
#include "monitor.hpp"

namespace apr
{

static std::uint32_t to_sdl_options(window_options options) noexcept
{
    std::uint32_t output{SDL_WINDOW_VULKAN};

    if(static_cast<bool>(options & window_options::fullscreen))
    {
        output |= SDL_WINDOW_FULLSCREEN;
    }

    if(static_cast<bool>(options & window_options::hidden))
    {
        output |= SDL_WINDOW_HIDDEN;
    }
    else{
        output |= SDL_WINDOW_SHOWN;
    }

    if(static_cast<bool>(options & window_options::borderless))
    {
        output |= SDL_WINDOW_BORDERLESS;
    }

    if(static_cast<bool>(options & window_options::resizable))
    {
        output |= SDL_WINDOW_RESIZABLE;
    }

    if(static_cast<bool>(options & window_options::minimized))
    {
        output |= SDL_WINDOW_MINIMIZED;
    }

    if(static_cast<bool>(options & window_options::maximized))
    {
        output |= SDL_WINDOW_MAXIMIZED;
    }

    return output;
}

window::window(application& application, const std::string& title, std::uint32_t width, std::uint32_t height, window_options options)
:m_monitors{std::data(application.enumerate_monitors())}
{
    m_window = SDL_CreateWindow(std::data(title), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, static_cast<int>(width), static_cast<int>(height), to_sdl_options(options));
    if(!m_window)
        throw std::runtime_error{"Can not create window. " + std::string{SDL_GetError()}};
}

window::window(application& application, const monitor& monitor, const std::string& title, std::uint32_t width, std::uint32_t height, window_options options)
:window{application, title, width, height, options}
{
    const std::int32_t x{monitor.x() + ((static_cast<std::int32_t>(monitor.width()) - static_cast<std::int32_t>(width)) / 2)};
    const std::int32_t y{monitor.y() + ((static_cast<std::int32_t>(monitor.height()) - static_cast<std::int32_t>(height)) / 2)};

    SDL_SetWindowPosition(m_window, x, y);
}

window::~window()
{
    if(m_window)
        SDL_DestroyWindow(m_window);
}

window::window(window&& other) noexcept
:m_window{std::exchange(other.m_window, nullptr)}
{

}

window& window::operator=(window&& other) noexcept
{
    m_window = std::exchange(other.m_window, m_window);

    return *this;
}

void window::resize(std::uint32_t width, std::uint32_t height)
{
    SDL_SetWindowSize(m_window, static_cast<int>(width), static_cast<int>(height));
}

void window::change_limits(std::uint32_t min_width, std::uint32_t min_height, std::uint32_t max_width, std::uint32_t max_height)
{
    SDL_SetWindowMinimumSize(m_window, static_cast<int>(min_width), static_cast<int>(min_height));
    SDL_SetWindowMaximumSize(m_window, static_cast<int>(max_width), static_cast<int>(max_height));
}

void window::move(std::int32_t relative_x, std::int32_t relative_y)
{
    SDL_SetWindowPosition(m_window, x() + relative_x, y() + relative_y);
}

void window::move_to(std::int32_t x, std::int32_t y)
{
    SDL_SetWindowPosition(m_window, x, y);
}

void window::move_to(const monitor& monitor, std::int32_t x, std::int32_t y)
{
    SDL_SetWindowPosition(m_window, monitor.x() + x, monitor.y() + y);
}

void window::hide()
{
    SDL_HideWindow(m_window);
}

void window::show()
{
    SDL_ShowWindow(m_window);
}

void window::hide_cursor()
{
    SDL_ShowCursor(SDL_DISABLE);
}

void window::show_cursor()
{
    SDL_ShowCursor(SDL_ENABLE);
}

void window::grab_cursor()
{
    SDL_SetWindowGrab(m_window, SDL_TRUE);
}

void window::release_cursor()
{
    SDL_SetWindowGrab(m_window, SDL_FALSE);
}

void window::minimize()
{
    SDL_MinimizeWindow(m_window);
}

void window::maximize()
{
    SDL_MaximizeWindow(m_window);
}

void window::enable_resizing()
{
    SDL_SetWindowResizable(m_window, SDL_TRUE);
}

void window::disable_resizing()
{
    SDL_SetWindowResizable(m_window, SDL_FALSE);
}

void window::restore()
{
    SDL_RestoreWindow(m_window);
}

void window::raise()
{
    SDL_RaiseWindow(m_window);
}

void window::change_title(const std::string& title)
{
    SDL_SetWindowTitle(m_window, std::data(title));
}

void window::change_icon(const std::uint8_t* rgba, std::uint32_t width, std::uint32_t height)
{
    SDL_Surface* surface{SDL_CreateRGBSurfaceFrom(const_cast<std::uint8_t*>(rgba), static_cast<int>(width), static_cast<int>(height), 32, width * 4, 0xF000, 0x0F00, 0x00F0, 0x000F)};
    SDL_SetWindowIcon(m_window, surface);
    SDL_FreeSurface(surface);
}

void window::change_opacity(float opacity)
{
    SDL_SetWindowOpacity(m_window, opacity);
}

void window::switch_to_fullscreen()
{
    SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
}

void window::switch_to_fullscreen(const monitor& monitor)
{
    move_to(monitor, 0, 0);
    switch_to_fullscreen();
}

void window::switch_to_windowed_fullscreen()
{
    SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void window::switch_to_windowed_fullscreen(const monitor& monitor)
{
    move_to(monitor, 0, 0);
    switch_to_windowed_fullscreen();
}

void window::switch_to_windowed()
{
    SDL_SetWindowFullscreen(m_window, 0);
}

void window::switch_to_windowed(const monitor& monitor)
{
    move_to(monitor, 0, 0);
    switch_to_windowed();
}

window::id_type window::id() const noexcept
{
    return m_window ? SDL_GetWindowID(m_window) : 0;
}

std::uint32_t window::width() const noexcept
{
    int output{};
    SDL_GetWindowSize(m_window, &output, nullptr);
    return static_cast<std::uint32_t>(output);
}

std::uint32_t window::height() const noexcept
{
    int output{};
    SDL_GetWindowSize(m_window, nullptr, &output);
    return static_cast<std::uint32_t>(output);
}

std::int32_t window::x() const noexcept
{
    int output{};
    SDL_GetWindowPosition(m_window, &output, nullptr);
    return output;
}

std::int32_t window::x(const monitor& monitor) const noexcept
{
    int output{};
    SDL_GetWindowPosition(m_window, &output, nullptr);
    return output - monitor.x();
}

std::int32_t window::y() const noexcept
{
    int output{};
    SDL_GetWindowPosition(m_window, nullptr, &output);
    return output;
}

std::int32_t window::y(const monitor& monitor) const noexcept
{
    int output{};
    SDL_GetWindowPosition(m_window, nullptr, &output);
    return output - monitor.y();
}

bool window::has_focus() const noexcept
{
    return static_cast<bool>(SDL_GetWindowFlags(m_window) & (SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS));
}

bool window::is_minimized() const noexcept
{
    return static_cast<bool>(SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED);
}

bool window::is_maximized() const noexcept
{
    return static_cast<bool>(SDL_GetWindowFlags(m_window) & SDL_WINDOW_MAXIMIZED);
}

const monitor& window::current_monitor() const noexcept
{
    return m_monitors[SDL_GetWindowDisplayIndex(m_window)];
}

VkSurfaceKHR window::make_surface(VkInstance instance)
{
    VkSurfaceKHR surface{};

    if(!SDL_Vulkan_CreateSurface(m_window, instance, &surface))
        throw std::runtime_error{"Can not create window surface. " + std::string{SDL_GetError()}};

    return surface;
}

}
