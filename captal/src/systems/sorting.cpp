#include "sorting.hpp"

#include "../components/node.hpp"

namespace cpt
{

namespace systems
{

void z_sorting(entt::registry& world)
{
    world.sort<components::node>([](const components::node& left, const components::node& right) -> bool
    {
        return left.position().y < right.position().y || left.position().z < right.position().z;
    });
}

}

}
