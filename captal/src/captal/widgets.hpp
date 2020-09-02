#ifndef CAPTAL_WIDGETS_HPP_INCLUDED
#define CAPTAL_WIDGETS_HPP_INCLUDED

#include "config.hpp"

#include <limits>
#include <tuple>

namespace cpt
{

template<typename... Children>
struct widget
{
    std::int32_t x{};
    std::int32_t y{};/*
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint32_t minimum_width{};
    std::uint32_t minimum_height{};
    std::uint32_t maximum_width{std::numeric_limits<std::uint32_t>::max()};
    std::uint32_t maximum_height{std::numeric_limits<std::uint32_t>::max()};*/
    std::tuple<Children...> children{};
};

template<typename... Children>
struct box_layout : widget<Children...>
{

};

}

#endif
