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

#ifndef CAPTAL_SYSTEMS_PHYSICS_HPP_INCLUDED
#define CAPTAL_SYSTEMS_PHYSICS_HPP_INCLUDED

#include "../config.hpp"

#include <entt/entity/registry.hpp>

#include "../components/node.hpp"
#include "../components/rigid_body.hpp"

namespace cpt::systems
{

inline void physics(entt::registry& world)
{
    world.view<components::node, const components::rigid_body>().each([](components::node& node, const components::rigid_body& body)
    {
        if(body && !body->sleeping())
        {
            const auto position{body->position()};

            node.move_to(vec3f{position.x(), position.y(), node.position().z()});
            node.set_rotation(body->rotation());
        }
    });
}

inline void physics_floored(entt::registry& world)
{
    world.view<components::node, const components::rigid_body>().each([](components::node& node, const components::rigid_body& body)
    {
        if(body && !body->sleeping())
        {
            const auto position{body->position()};

            node.move_to(vec3f{std::floor(position.x()), std::floor(position.y()), node.position().z()});
            node.set_rotation(body->rotation());
        }
    });
}

}

#endif
