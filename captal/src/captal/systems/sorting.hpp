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

#ifndef CAPTAL_SYSTEMS_SORTING_HPP_INCLUDED
#define CAPTAL_SYSTEMS_SORTING_HPP_INCLUDED

#include "../config.hpp"

#include <entt/entity/registry.hpp>

#include "../components/node.hpp"
#include "../components/drawable.hpp"
#include "../components/draw_index.hpp"

namespace cpt::systems
{

template<components::drawable_specialization Drawable = components::drawable>
void z_sorting(entt::registry& world)
{
    world.sort<components::node>([](const components::node& left, const components::node& right) -> bool
    {
        const vec3f left_position{left.position() - left.origin()};
        const vec3f right_position{right.position() - right.origin()};

        return std::make_pair(left_position.z(), left_position.y()) < std::make_pair(right_position.z(), right_position.y());
    });

    world.sort<Drawable, components::node>();
}

template<components::drawable_specialization Drawable = components::drawable>
void index_sorting(entt::registry& world)
{
    world.sort<components::draw_index>([](components::draw_index left, components::draw_index right) -> bool
    {
        return left.index < right.index;
    });

    world.sort<Drawable, components::draw_index>();
}

template<components::drawable_specialization Drawable = components::drawable>
void index_z_sorting(entt::registry& world)
{
    world.sort<components::node>([&world](entt::entity left, entt::entity right) -> bool
    {
        const auto left_draw_index {world.get<components::draw_index>(left)};
        const auto right_draw_index{world.get<components::draw_index>(right)};

        const auto& left_node {world.get<components::node>(left)};
        const auto& right_node{world.get<components::node>(right)};

        const vec3f left_position {left_node.position()  - left_node.origin()};
        const vec3f right_position{right_node.position() - right_node.origin()};

        return std::make_tuple(left_draw_index.index, left_position.z(), left_position.y()) < std::make_tuple(right_draw_index.index, right_position.z(), right_position.y());
    });

    world.sort<Drawable, components::node>();
}


}

#endif
