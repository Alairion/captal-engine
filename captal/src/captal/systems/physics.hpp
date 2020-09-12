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

            node.move_to({position.x, position.y, node.position().z});
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

            node.move_to({std::floor(position.x), std::floor(position.y), node.position().z});
            node.set_rotation(body->rotation());
        }
    });
}

}

#endif
