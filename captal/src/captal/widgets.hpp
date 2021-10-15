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

#ifndef CAPTAL_WIDGETS_HPP_INCLUDED
#define CAPTAL_WIDGETS_HPP_INCLUDED

#include "config.hpp"

#include <limits>
#include <tuple>
#include <concepts>
#include <string>

#include "render_window.hpp"
#include "text.hpp"
#include "signal.hpp"

namespace cpt
{

//The minimum requirements of every widgets
template<typename T>
concept widget = requires(T w, const apr::window_event& we, const apr::mouse_event& me, const apr::keyboard_event& ke, const apr::text_event& te)
{
    requires std::is_move_constructible_v<T>;
    requires std::is_move_assignable_v<T>;

    {w.x}          -> std::convertible_to<std::uint32_t>;
    {w.y}          -> std::convertible_to<std::uint32_t>;
    {w.width}      -> std::convertible_to<std::uint32_t>;
    {w.height}     -> std::convertible_to<std::uint32_t>;
    {w.min_width}  -> std::convertible_to<std::uint32_t>;
    {w.min_height} -> std::convertible_to<std::uint32_t>;
    {w.max_width}  -> std::convertible_to<std::uint32_t>;
    {w.max_height} -> std::convertible_to<std::uint32_t>;

    {w.visible}   -> std::convertible_to<bool>;
    {w.has_focus} -> std::convertible_to<bool>;

    w.gained_focus(we);
    w.lost_focus(we);
    w.mouse_entered(we);
    w.mouse_left(we);
    w.moved(we);
    w.resized(we);

    {w.mouse_button_pressed(me)}  -> std::convertible_to<bool>;
    {w.mouse_button_released(me)} -> std::convertible_to<bool>;
    {w.mouse_moved(me)}           -> std::convertible_to<bool>;
    {w.mouse_wheel_scrolled(me)}  -> std::convertible_to<bool>;

    {w.key_pressed(ke)}  -> std::convertible_to<bool>;
    {w.key_released(ke)} -> std::convertible_to<bool>;

    {w.text_entered(te)} -> std::convertible_to<bool>;
};

template<typename T>
concept renderable_widget = requires(T w)
{
    requires widget<T>;

    //a way to generate renderable data and return it to form
};

template<typename T>
concept tuple_like = requires(T t)
{
    std::get<0>(t);
    std::tuple_size_v<T>;
    typename std::tuple_element<0, T>::type;
};

template<tuple_like Tuple, typename T, T... Ints>
consteval bool is_all_widgets(std::integer_sequence<T, Ints...>)
{
    return (widget<std::tuple_element_t<Ints, Tuple>> && ...);
}

template<typename T>
concept widget_tuple = requires
{
    requires tuple_like<T>;
    requires is_all_widgets<T>(std::make_index_sequence<std::tuple_size_v<T>>{});
};

template<typename T>
concept parent_widget = requires(T w)
{
    requires widget<T>;
    requires widget_tuple<std::decay_t<decltype(w.children)>> || widget<std::decay_t<decltype(w.children)>>;
};
/*
enum class layout_alignement : std::uint32_t
{
    left    = 0x01,
    right   = 0x02,
    hcenter = 0x40,
    top     = 0x10,
    bottom  = 0x20,
    vcenter = 0x40,

    top_left      = top | left,
    top_right     = top | right,
    top_center    = top | hcenter,
    bottom_left   = bottom | left,
    bottom_right  = bottom | right,
    bottom_center = bottom | hcenter,
    center_right  = vcenter | right,
    center_left   = vcenter | left,
    center        = vcenter | hcenter,
};*/

enum class layout_direction : std::uint32_t
{
    left_to_right = 0,
    right_to_left = 1,
    bottom_to_top = 2,
    top_to_bottom = 3,
};

template<typename T>
concept layout = requires(T l)
{
    requires parent_widget<T>;

    {l.top_margin}    -> std::convertible_to<std::uint32_t>;
    {l.right_margin}  -> std::convertible_to<std::uint32_t>;
    {l.bottom_margin} -> std::convertible_to<std::uint32_t>;
    {l.left_margin}   -> std::convertible_to<std::uint32_t>;
    {l.spacing}       -> std::convertible_to<std::int32_t>;
    {l.direction}     -> std::convertible_to<layout_direction>;
};

struct basic_widget
{
    std::int32_t  x{};
    std::int32_t  y{};
    std::uint32_t width{1};
    std::uint32_t height{1};
    std::uint32_t min_width{1};
    std::uint32_t min_height{1};
    std::uint32_t max_width{std::numeric_limits<std::uint32_t>::max()};
    std::uint32_t max_height{std::numeric_limits<std::uint32_t>::max()};

    bool visible{true};
    bool has_focus{};

    void gained_focus(const apr::window_event& event [[maybe_unused]])
    {

    }

    void lost_focus(const apr::window_event& event [[maybe_unused]])
    {

    }

    void mouse_entered(const apr::window_event& event [[maybe_unused]])
    {

    }

    void mouse_left(const apr::window_event& event [[maybe_unused]])
    {

    }

    void moved(const apr::window_event& event [[maybe_unused]])
    {

    }

    void resized(const apr::window_event& event [[maybe_unused]])
    {

    }

    bool mouse_button_pressed(const apr::mouse_event& event [[maybe_unused]])
    {
        return false;
    }

    bool mouse_button_released(const apr::mouse_event& event [[maybe_unused]])
    {
        return false;
    }

    bool mouse_moved(const apr::mouse_event& event [[maybe_unused]])
    {
        return false;
    }

    bool mouse_wheel_scrolled(const apr::mouse_event& event [[maybe_unused]])
    {
        return false;
    }

    bool key_pressed(const apr::keyboard_event& event [[maybe_unused]])
    {
        return false;
    }

    bool key_released(const apr::keyboard_event& event [[maybe_unused]])
    {
        return false;
    }

    bool text_entered(const apr::text_event& event [[maybe_unused]])
    {
        return false;
    }
};

template<typename... Children>
struct box_layout
{
    bool visible{true};
    bool has_focus{};

    std::uint32_t    top_margin{6};
    std::uint32_t    right_margin{6};
    std::uint32_t    bottom_margin{6};
    std::uint32_t    left_margin{6};
    std::int32_t     spacing{6};
    layout_direction direction{};

    std::tuple<Children...> children{};

    void moved(const apr::window_event& event [[maybe_unused]])
    {

    }

    void resized(const apr::window_event& event [[maybe_unused]])
    {

    }
};

class form final : public basic_renderable
{
    class widget_container_base
    {
    public:
        widget_container_base() = default;
        virtual ~widget_container_base() = default;
        widget_container_base(const widget_container_base&) = delete;
        widget_container_base& operator=(const widget_container_base&) = delete;
        widget_container_base(widget_container_base&&) noexcept = delete;
        widget_container_base& operator=(widget_container_base&&) noexcept = delete;

        virtual void resize(std::uint32_t width, std::uint32_t height) = 0;
    };

    template<widget Widget>
    class widget_container final : public widget_container_base
    {
    public:
        using widget_type = Widget;

    public:
        widget_container() = default;

        template<typename T>
        widget_container(T&& widget)
        :m_widget{std::forward<T>(widget)}
        {

        }

        ~widget_container() = default;
        widget_container(const widget_container&) = delete;
        widget_container& operator=(const widget_container&) = delete;
        widget_container(widget_container_base&&) noexcept = delete;
        widget_container& operator=(widget_container&&) noexcept = delete;

        void resize(std::uint32_t width, std::uint32_t height) override
        {
            m_widget.resize(width, height);
        }

    private:
        widget_type m_widget{};
    };

public:
    template<widget Widget>
    form(Widget&& top_widget)
    :m_widget{std::make_unique<widget_container<Widget>>(std::forward<Widget>(top_widget))}
    {

    }

    ~form() = default;
    form(const form&) = delete;
    form& operator=(const form&) = delete;
    form(form&&) noexcept = default;
    form& operator=(form&&) noexcept = default;

    void dispatch_event(const apr::event& event)
    {

    }

    void resize(std::uint32_t width, std::uint32_t height)
    {
        m_widget->resize(width, height);
    }

private:
    std::unique_ptr<widget_container_base> m_widget{};
};

}

#endif
