#include "physics.hpp"

#include "../components/node.hpp"
#include "../components/physical_body.hpp"

namespace cpt
{

namespace systems
{

void physics(entt::registry& world)
{
    world.view<components::node, const components::physical_body>().each([](components::node& node, const components::physical_body& body)
    {
        node.move_to(glm::vec3{body.attachment()->position(), node.position().z});
        node.set_rotation(body.attachment()->rotation());
        //node.set_origin(glm::vec3{body.attachment()->mass_center(), 0.0f});
    });
}

}

}
