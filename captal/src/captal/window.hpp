#ifndef CAPTAL_RENDER_WINDOW_HPP_INCLUDED
#define CAPTAL_RENDER_WINDOW_HPP_INCLUDED

#include "config.hpp"

#include <apyre/window.hpp>
#include <apyre/event.hpp>

#include <tephra/surface.hpp>
#include <tephra/swapchain.hpp>
#include <tephra/render_target.hpp>
#include <tephra/commands.hpp>
#include <tephra/query.hpp>

#include "render_target.hpp"
#include "color.hpp"

namespace cpt
{

class window;

class CAPTAL_API event_iterator
{
public:
    using value_type        = apr::event_iterator::value_type;
    using difference_type   = apr::event_iterator::difference_type;
    using pointer           = apr::event_iterator::pointer;
    using reference         = apr::event_iterator::reference;
    using iterator_category = apr::event_iterator::iterator_category;

public:
    constexpr event_iterator() = default;
    event_iterator(window& window, apr::event_mode mode = apr::event_mode::poll);

    ~event_iterator() = default;
    event_iterator(const event_iterator&) = default;
    event_iterator& operator=(const event_iterator&) = default;
    event_iterator(event_iterator&& other) noexcept = default;
    event_iterator& operator=(event_iterator&& other) noexcept = default;

    reference operator*() const noexcept
    {
        return *m_iterator;
    }

    pointer operator->() const noexcept
    {
        return &(*m_iterator);
    }

    event_iterator& operator++();

    event_iterator begin() const noexcept
    {
        return *this;
    }

    event_iterator end() const noexcept
    {
        return event_iterator{};
    }

    bool operator!=(const event_iterator& other) const noexcept
    {
        return m_iterator != other.m_iterator;
    }

private:
    window* m_window{};
    apr::event_iterator m_iterator{};
};

struct video_mode
{
    std::uint32_t image_count{2};
    tph::texture_usage usage{tph::texture_usage::color_attachment};
    tph::surface_transform transform{tph::surface_transform::identity};
    tph::surface_composite composite{tph::surface_composite::opaque};
    tph::present_mode present_mode{tph::present_mode::fifo};
    bool clipping{true};
};

using window_event_signal = cpt::signal<const apr::window_event&>;
using mouse_event_signal = cpt::signal<const apr::mouse_event&>;
using keyboard_event_signal = cpt::signal<const apr::keyboard_event&>;
using text_event_signal = cpt::signal<const apr::text_event&>;

class CAPTAL_API window : apr::window
{
    friend class event_iterator;

public:
    window() = default;
    explicit window(const std::string& title, std::uint32_t width, std::uint32_t height, const cpt::video_mode& mode, apr::window_options options = apr::window_options::none);
    explicit window(const apr::monitor& monitor, const std::string& title, std::uint32_t width, std::uint32_t height, const cpt::video_mode& mode, apr::window_options options = apr::window_options::none);
    explicit window(apr::window window, const cpt::video_mode& mode);

    virtual ~window();
    window(const window&) = delete;
    window& operator=(const window&) = delete;
    window(window&&) = default;
    window& operator=(window&&) = default;

    using apr::window::resize;
    using apr::window::change_limits;
    using apr::window::move;
    using apr::window::move_to;
    using apr::window::hide;
    using apr::window::show;
    using apr::window::hide_cursor;
    using apr::window::show_cursor;
    using apr::window::grab_cursor;
    using apr::window::release_cursor;
    using apr::window::minimize;
    using apr::window::maximize;
    using apr::window::enable_resizing;
    using apr::window::disable_resizing;
    using apr::window::restore;
    using apr::window::raise;
    using apr::window::change_title;
    using apr::window::change_icon;
    using apr::window::change_opacity;
    using apr::window::switch_to_fullscreen;
    using apr::window::switch_to_windowed_fullscreen;
    using apr::window::switch_to_windowed;

    using apr::window::id;
    using apr::window::width;
    using apr::window::height;
    using apr::window::x;
    using apr::window::y;
    using apr::window::has_focus;
    using apr::window::is_minimized;
    using apr::window::is_maximized;
    using apr::window::current_monitor;

    void close();

    void dispatch_events();
    void discard_events();
    void dispatch_event(const apr::event& event);

    apr::window& get_window() noexcept
    {
        return static_cast<apr::window&>(*this);
    }

    const apr::window& get_window() const noexcept
    {
        return static_cast<const apr::window&>(*this);
    }

    const cpt::video_mode& video_mode() const noexcept
    {
        return m_video_mode;
    }

    tph::texture_format surface_format() const noexcept
    {
        return m_surface_format;
    }

    tph::surface& surface() noexcept
    {
        return m_surface;
    }

    const tph::surface& surface() const noexcept
    {
        return m_surface;
    }

    tph::swapchain& swapchain() noexcept
    {
        return m_swapchain;
    }

    const tph::swapchain& swapchain() const noexcept
    {
        return m_swapchain;
    }

    window_event_signal& on_gained_focus()  noexcept {return m_gained_focus;}
    window_event_signal& on_lost_focus()    noexcept {return m_lost_focus;}
    window_event_signal& on_mouse_entered() noexcept {return m_mouse_entered;}
    window_event_signal& on_mouse_left()    noexcept {return m_mouse_left;}
    window_event_signal& on_moved()         noexcept {return m_moved;}
    window_event_signal& on_resized()       noexcept {return m_resized;}
    window_event_signal& on_minimized()     noexcept {return m_minimized;}
    window_event_signal& on_maximized()     noexcept {return m_maximized;}
    window_event_signal& on_restored()      noexcept {return m_restored;}
    window_event_signal& on_close()         noexcept {return m_close;}

    mouse_event_signal& on_mouse_button_pressed()  noexcept {return m_mouse_button_pressed;}
    mouse_event_signal& on_mouse_button_released() noexcept {return m_mouse_button_released;}
    mouse_event_signal& on_mouse_moved()           noexcept {return m_mouse_moved;}
    mouse_event_signal& on_mouse_wheel_scroll()    noexcept {return m_mouse_wheel_scroll;}

    keyboard_event_signal& on_key_pressed()  noexcept {return m_key_pressed;}
    keyboard_event_signal& on_key_released() noexcept {return m_key_released;}

    text_event_signal& on_text_entered() noexcept {return m_text_entered;}

    bool is_closed() const noexcept
    {
        return m_closed;
    }

    bool is_renderable() const noexcept
    {
        return m_renderable;
    }

#ifdef CAPTAL_DEBUG
    void set_name(std::string_view name);
#else
    void set_name(std::string_view name [[maybe_unused]]) const noexcept
    {

    }
#endif

private:
    cpt::video_mode m_video_mode{};
    tph::texture_format m_surface_format{};
    tph::surface m_surface{};
    tph::swapchain m_swapchain{};

    bool m_closed{};
    bool m_renderable{true};

    window_event_signal   m_gained_focus{};
    window_event_signal   m_lost_focus{};
    window_event_signal   m_mouse_entered{};
    window_event_signal   m_mouse_left{};
    window_event_signal   m_moved{};
    window_event_signal   m_resized{};
    window_event_signal   m_minimized{};
    window_event_signal   m_maximized{};
    window_event_signal   m_restored{};
    window_event_signal   m_close{};
    mouse_event_signal    m_mouse_button_pressed{};
    mouse_event_signal    m_mouse_button_released{};
    mouse_event_signal    m_mouse_moved{};
    mouse_event_signal    m_mouse_wheel_scroll{};
    keyboard_event_signal m_key_pressed{};
    keyboard_event_signal m_key_released{};
    text_event_signal     m_text_entered{};

#ifdef CAPTAL_DEBUG
    std::string m_name{};
#endif
};

using window_ptr = std::shared_ptr<window>;
using window_weak_ptr = std::weak_ptr<window>;

template<typename... Args>
window_ptr make_window(Args&&... args)
{
    return std::make_shared<window>(std::forward<Args>(args)...);
}

}

#endif
