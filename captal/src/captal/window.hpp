#ifndef CAPTAL_WINDOW_HPP_INCLUDED
#define CAPTAL_WINDOW_HPP_INCLUDED

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

class CAPTAL_API window;

using window_event_signal = cpt::signal<window&, const apr::window_event&>;
using mouse_event_signal = cpt::signal<window&, const apr::mouse_event&>;
using keyboard_event_signal = cpt::signal<window&, const apr::keyboard_event&>;
using text_event_signal = cpt::signal<window&, const apr::text_event&>;

class CAPTAL_API window : apr::window
{
    friend class event_iterator;

public:
    window() = default;
    explicit window(const std::string& title, std::uint32_t width, std::uint32_t height, apr::window_options options = apr::window_options::none);
    explicit window(const apr::monitor& monitor, const std::string& title, std::uint32_t width, std::uint32_t height, apr::window_options options = apr::window_options::none);

    ~window() = default;
    window(const window&) = delete;
    window& operator=(const window&) = delete;
    window(window&&) = default;
    window& operator=(window&&) = default;

    using apr::window::close;
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

    using apr::window::is_open;
    using apr::window::id;
    using apr::window::width;
    using apr::window::height;
    using apr::window::x;
    using apr::window::y;
    using apr::window::has_focus;
    using apr::window::is_minimized;
    using apr::window::is_maximized;
    using apr::window::current_monitor;
    using apr::window::atomic_surface_size;

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

    tph::surface& surface() noexcept
    {
        return m_surface;
    }

    const tph::surface& surface() const noexcept
    {
        return m_surface;
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

#ifdef CAPTAL_DEBUG
    void set_name(std::string_view name);
#else
    void set_name(std::string_view name [[maybe_unused]]) const noexcept
    {

    }
#endif

private:
    tph::surface m_surface{};

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
};

using window_ptr = std::shared_ptr<window>;
using window_weak_ptr = std::weak_ptr<window>;

template<typename... Args>
window_ptr make_window(Args&&... args)
{
    return std::make_shared<window>(std::forward<Args>(args)...);
}

class CAPTAL_API event_iterator : apr::event_iterator
{
public:
    using value_type        = apr::event_iterator::value_type;
    using difference_type   = apr::event_iterator::difference_type;
    using pointer           = apr::event_iterator::pointer;
    using reference         = apr::event_iterator::reference;
    using iterator_category = apr::event_iterator::iterator_category;

public:
    constexpr event_iterator() = default;
    event_iterator(window_ptr window, apr::event_mode mode = apr::event_mode::poll);
    event_iterator(window& window, apr::event_mode mode = apr::event_mode::poll);

    ~event_iterator() = default;
    event_iterator(const event_iterator&) = default;
    event_iterator& operator=(const event_iterator&) = default;
    event_iterator(event_iterator&& other) noexcept = default;
    event_iterator& operator=(event_iterator&& other) noexcept = default;

    using apr::event_iterator::operator*;
    using apr::event_iterator::operator->;
    using apr::event_iterator::operator++;

    using apr::event_iterator::begin;
    using apr::event_iterator::end;
    using apr::event_iterator::operator!=;
};

}

#endif
