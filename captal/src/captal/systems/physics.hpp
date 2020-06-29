#ifndef CAPTAL_SYSTEMS_PHYSICS_HPP_INCLUDED
#define CAPTAL_SYSTEMS_PHYSICS_HPP_INCLUDED

#include "../config.hpp"

#include <entt/entity/registry.hpp>

#include "../components/node.hpp"
#include "../components/physical_body.hpp"

namespace cpt::systems
{

inline void physics(entt::registry& world)
{
    world.view<components::node, const components::physical_body>().each([](components::node& node, const components::physical_body& body)
    {
        assert(body.attachment() && "Invalid attachment");

        if(!body.attachment()->sleeping())
        {
            const auto position{body.attachment()->position()};

            node.move_to(glm::vec3{position.x, position.y, node.position().z});
            node.set_rotation(body.attachment()->rotation());
        }
    });
}

inline void physics_floored(entt::registry& world)
{
    world.view<components::node, const components::physical_body>().each([](components::node& node, const components::physical_body& body)
    {
        assert(body.attachment() && "Invalid attachment");

        if(!body.attachment()->sleeping())
        {
            const auto position{body.attachment()->position()};

            node.move_to(glm::vec3{std::floor(position.x), std::floor(position.y), node.position().z});
            node.set_rotation(body.attachment()->rotation());
        }
    });
}

}

#endif
