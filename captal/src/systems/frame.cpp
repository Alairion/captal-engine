#include "frame.hpp"

#include "../components/node.hpp"

namespace cpt
{

namespace systems
{

void end_frame(entt::registry& world)
{
    world.view<components::node>().each([](components::node& node)
    {
        node.clear();
    });
}

}

}
