#include "sorting.hpp"

#include "../components/node.hpp"
#include "../components/drawable.hpp"

namespace cpt
{

namespace systems
{

void z_sorting(entt::registry& world)
{
    world.sort<components::node>([](const components::node& left, const components::node& right) -> bool
    {
        const glm::vec3 left_position{left.position() - left.origin()};
        const glm::vec3 right_position{right.position() - right.origin()};

        return left_position.z < right_position.z || left_position.y < right_position.y;
    });

    world.sort<components::drawable, components::node>();
}

}

}
