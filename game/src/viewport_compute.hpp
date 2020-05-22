#ifndef MY_PROJECT_VIEWPORT_COMPUTE_HPP_INCLUDED
#define MY_PROJECT_VIEWPORT_COMPUTE_HPP_INCLUDED

#include "config.hpp"

#include <captal/render_window.hpp>

#include <glm/vec3.hpp>

namespace mpr
{

inline std::uint32_t window_scale(const cpt::render_window_ptr& window) noexcept
{
    return (window->height() + 180u) / 360u;
}

template<typename T>
T scale_from_window(const cpt::render_window_ptr& window, T value) noexcept
{
    const std::uint32_t scale{window_scale(window)};

    return value / static_cast<T>(scale);
}

template<typename T>
T scale_to_window(const cpt::render_window_ptr& window, T value) noexcept
{
    const std::uint32_t scale{window_scale(window)};

    return value * static_cast<T>(scale);
}

template<typename... Args>
std::tuple<Args...> scale_from_window(const cpt::render_window_ptr& window, Args&&... value) noexcept
{
    return std::make_tuple(scale_from_window(window, std::forward(value))...);
}

template<typename... Args>
std::tuple<Args...> scale_to_window(const cpt::render_window_ptr& window, Args&&... value) noexcept
{
    return std::make_tuple(scale_to_window(window, std::forward(value))...);
}

inline glm::vec3 scaled_window_center(const cpt::render_window_ptr& window) noexcept
{
    return glm::vec3{scale_from_window(window, window->width() / 2), scale_from_window(window, window->height() / 2), 0};
}

}

#endif
