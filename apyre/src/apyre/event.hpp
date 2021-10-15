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

#ifndef APYRE_EVENT_HPP_INCLUDED
#define APYRE_EVENT_HPP_INCLUDED

#include "config.hpp"

#include <variant>
#include <vector>
#include <unordered_map>
#include <string>
#include <array>
#include <optional>

#include "inputs.hpp"

namespace apr
{

class application;
class window;

struct quit_event
{

};

struct window_event
{
    enum : std::uint32_t
    {
        closed = 0,
        resized,
        minimized,
        maximized,
        restored,
        moved,
        gained_focus,
        lost_focus,
        mouse_entered,
        mouse_left,
    };

    std::uint32_t type{};
    std::uint32_t window{};
    std::int32_t  x{};
    std::int32_t  y{};
    std::uint32_t width{};
    std::uint32_t height{};
};

struct mouse_event
{
    enum : std::uint32_t
    {
        moved = 0,
        button_pressed,
        button_released,
        wheel_scrolled,
    };

    std::uint32_t type{};
    std::uint32_t window{};
    std::int32_t  x{};
    std::int32_t  y{};
    std::int32_t  relative_x{};
    std::int32_t  relative_y{};
    mouse_button  button{};
    std::uint32_t clicks{};
    std::int32_t  wheel{};
};

struct keyboard_event
{
    enum : std::uint32_t
    {
        key_pressed,
        key_released
    };

    std::uint32_t type{};
    std::uint32_t window{};
    keycode  key{};
    scancode scan{};
    key_modifier modifiers{};
};

struct text_event
{
    enum : std::uint32_t
    {
        text_entered,
    };

    std::uint32_t type{};
    std::uint32_t window{};
    std::string   text{};
};

using event = std::variant<quit_event, window_event, mouse_event, keyboard_event, text_event>;

constexpr std::uint32_t event_window_id(const event& event) noexcept
{
    return std::visit([](auto&& alternative)
    {
        if constexpr(std::is_same_v<std::decay_t<decltype(alternative)>, quit_event>)
        {
            return 0u;
        }
        else
        {
            return alternative.window;
        }
    }, event);
}

enum class event_mode : std::uint32_t
{
    poll = 0,
    wait = 1,
};

class APYRE_API event_queue
{
public:
    event_queue() = default;
    ~event_queue() = default;
    event_queue(const event_queue&) = delete;
    event_queue& operator=(const event_queue&) = delete;
    event_queue(event_queue&& other) noexcept = delete;
    event_queue& operator=(event_queue&& other) noexcept = delete;

    std::optional<event> next(event_mode mode = event_mode::poll);
    std::optional<event> next(window& window, event_mode mode = event_mode::poll);

    void register_window(std::uint32_t id)
    {
        m_events.emplace(id, std::vector<event>{});
    }

    void unregister_window(std::uint32_t id)
    {
        m_events.erase(id);
    }

private:
    void flush(event_mode mode, std::uint32_t id);
    std::optional<event> next(event_mode mode, std::uint32_t id);

private:
    std::unordered_map<std::uint32_t, std::vector<event>> m_events{};
};

class APYRE_API event_iterator
{
public:
    using value_type = event;
    using difference_type = std::ptrdiff_t;
    using pointer = const event*;
    using reference = const event&;
    using iterator_category = std::input_iterator_tag;

public:
    constexpr event_iterator() = default;
    explicit event_iterator(application& application, event_mode mode = event_mode::poll);
    explicit event_iterator(application& application, window& window, event_mode mode = event_mode::poll);

    ~event_iterator() = default;
    event_iterator(const event_iterator&) = default;
    event_iterator& operator=(const event_iterator&) = default;
    event_iterator(event_iterator&& other) noexcept = default;
    event_iterator& operator=(event_iterator&& other) noexcept = default;

    reference operator*() const noexcept
    {
        return *m_current_event;
    }

    pointer operator->() const noexcept
    {
        return &(*m_current_event);
    }

    event_iterator& operator++()
    {
        if(m_window)
        {
            m_current_event = m_queue->next(*m_window, m_mode);
        }
        else
        {
            m_current_event = m_queue->next(m_mode);
        }

        return *this;
    }

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
        return m_current_event.has_value() != other.m_current_event.has_value();
    }

private:
    event_queue* m_queue{};
    event_mode m_mode{};
    window* m_window{};
    std::optional<event> m_current_event{std::nullopt};
};

}

#endif
