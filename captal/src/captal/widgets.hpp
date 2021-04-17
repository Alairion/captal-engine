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
struct box_layout : basic_widget
{
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

struct form
{
public:
    form(window_ptr window, render_target_ptr target)
    :m_window{std::move(window)}
    ,m_target{std::move(target)}
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

private:
    window_ptr m_window{};
    render_target_ptr m_target{};
};

}

#endif
