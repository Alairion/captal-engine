#ifndef CAPTAL_FRAME_HPP_INCLUDED
#define CAPTAL_FRAME_HPP_INCLUDED

#include "../config.hpp"

#include <entt/entity/registry.hpp>

#include "../components/node.hpp"

namespace cpt::systems
{

inline void end_frame(entt::registry& world)
{
    world.view<components::node>().each([](components::node& node)
    {
        node.clear();
    });
}

}

#endif
