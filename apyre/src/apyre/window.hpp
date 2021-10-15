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

#ifndef APYRE_WINDOW_HPP_INCLUDED
#define APYRE_WINDOW_HPP_INCLUDED

#include "config.hpp"

#include <string>
#include <memory>
#include <atomic>
#include <functional>
#include <span>
#include <variant>

class SDL_Window;

#ifndef VULKAN_H_ //Trust me, it works
    #define APYRE_VK_DEFINE_HANDLE(object) typedef struct object##_T* object;

    #if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
        #define APYRE_VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
    #else
        #define APYRE_VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
    #endif

    APYRE_VK_DEFINE_HANDLE(VkInstance)
    APYRE_VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)
#endif

namespace apr
{

class application;
class monitor;
class event_queue;

struct win32_window_info
{
    void* hinstance{};
    void* device_context{};
    void* window{};
};

struct x11_window_info
{
    void* display{};
    std::uintptr_t window{};
};

struct wayland_window_info
{
    void* display{};
    void* surface{};
    void* shell_surface{};
};

struct cocoa_window_info
{
    void* window{};
};

struct android_window_info
{
    void* window{};
};

struct uikit_window_info
{
    void* window{};
};

using platform_window_info = std::variant<win32_window_info, x11_window_info, wayland_window_info, cocoa_window_info, android_window_info, uikit_window_info>;

enum class window_system : std::uint32_t
{
    win32   = 0,
    x11     = 1,
    wayland = 2,
    android = 3,
    cocoa   = 4,
    uikit   = 5
};

enum class window_options : std::uint32_t
{
    none = 0,
    fullscreen = 0x01,
    hidden = 0x02,
    borderless = 0x04,
    resizable = 0x08,
    minimized = 0x10,
    maximized = 0x20,
    high_dpi = 0x40,
    extended_client_area = 0x80
};

enum class hit_test_result : std::uint32_t
{
    normal = 0,
    drag = 1,
    resize_topleft = 2,
    resize_top = 3,
    resize_topright = 4,
    resize_right = 5,
    resize_bottomright = 6,
    resize_bottom = 7,
    resize_bottomleft = 8,
    resize_left = 9,
};

class APYRE_API window
{
    friend class event_queue;

public:
    using id_type = std::uint32_t;
    using hit_test_function_type = std::function<hit_test_result(std::int32_t, std::int32_t)>;

public:
    constexpr window() = default;
    explicit window(application& application, const std::string& title, std::uint32_t width, std::uint32_t height, window_options options = window_options::none);
    explicit window(application& application, const monitor& monitor, const std::string& title, std::uint32_t width, std::uint32_t height, window_options options = window_options::none);

    ~window();
    window(const window&) = delete;
    window& operator=(const window&) = delete;
    window(window&& other) noexcept;
    window& operator=(window&& other) noexcept;

    void close() noexcept;
    void resize(std::uint32_t width, std::uint32_t height);
    void change_limits(std::uint32_t min_width, std::uint32_t min_height, std::uint32_t max_width, std::uint32_t max_height);
    void move(std::int32_t relative_x, std::int32_t relative_y);
    void move_to(std::int32_t x, std::int32_t y);
    void move_to(const monitor& monitor, std::int32_t x, std::int32_t y);
    void hide();
    void show();
    void hide_cursor();
    void show_cursor();
    void grab_cursor();
    void release_cursor();
    void minimize();
    void maximize();
    void enable_resizing();
    void disable_resizing();
    void restore();
    void raise();
    void change_title(const std::string& title);
    void change_icon(const std::uint8_t* rgba, std::uint32_t width, std::uint32_t height);
    void change_opacity(float opacity);
    void change_hit_test_function(hit_test_function_type func);
    void switch_to_fullscreen();
    void switch_to_fullscreen(const monitor& monitor);
    void switch_to_windowed_fullscreen();
    void switch_to_windowed_fullscreen(const monitor& monitor);
    void switch_to_windowed();
    void switch_to_windowed(const monitor& monitor);

    VkSurfaceKHR make_surface(VkInstance instance);

    bool is_open() const noexcept
    {
        return m_window;
    }

    id_type id() const noexcept;
    std::uint32_t width() const noexcept;
    std::uint32_t height() const noexcept;
    std::int32_t x() const noexcept;
    std::int32_t x(const monitor& monitor) const noexcept;
    std::int32_t y() const noexcept;
    std::int32_t y(const monitor& monitor) const noexcept;
    bool has_focus() const noexcept;
    bool is_minimized() const noexcept;
    bool is_maximized() const noexcept;
    const monitor& current_monitor() const noexcept;

    std::pair<std::uint32_t, std::uint32_t> atomic_surface_size() const noexcept
    {
        const auto value {m_surface_size->load(std::memory_order_acquire)};
        const auto width {static_cast<std::uint32_t>(value >> 0)};
        const auto height{static_cast<std::uint32_t>(value >> 32)};

        return std::make_pair(width, height);
    }

    platform_window_info platform_info() const noexcept;

private:
    SDL_Window* m_window{};
    event_queue* m_event_queue{};
    std::span<const monitor> m_monitors{};
    window_options m_options{};
    bool m_need_fullscreen_restore{};
    hit_test_function_type m_hit_test_func{};
    std::unique_ptr<std::atomic<std::uint64_t>> m_surface_size{};

#ifdef _WIN32
    std::int32_t m_windowed_width{};
    std::int32_t m_windowed_height{};
#endif
};

}

template<> struct apr::enable_enum_operations<apr::window_options> {static constexpr bool value{true};};

#endif
