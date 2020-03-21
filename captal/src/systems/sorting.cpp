#include "sorting.hpp"

#include "../components/node.hpp"
#include "../components/drawable.hpp"
#include "../components/draw_index.hpp"

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

        return std::make_pair(left_position.z, left_position.y) < std::make_pair(right_position.z, right_position.y);
    });

    world.sort<components::drawable, components::node>();
}

void index_sorting(entt::registry& world)
{
    world.sort<components::draw_index>([](components::draw_index left, components::draw_index right) -> bool
    {
        return left.index < right.index;
    });

    world.sort<components::drawable, components::draw_index>();
}

void index_z_sorting(entt::registry& world)
{
    world.sort<components::node>([&world](entt::entity left, entt::entity right) -> bool
    {
        components::draw_index left_draw_index{world.get<components::draw_index>(left)};
        components::draw_index right_draw_index{world.get<components::draw_index>(right)};

        const components::node& left_node{world.get<components::node>(left)};
        const components::node& right_node{world.get<components::node>(right)};

        const glm::vec3 left_position{left_node.position() - left_node.origin()};
        const glm::vec3 right_position{right_node.position() - right_node.origin()};

        return std::make_tuple(left_draw_index.index, left_position.z, left_position.y) < std::make_tuple(right_draw_index.index, right_position.z, right_position.y);
    });

    world.sort<components::drawable, components::node>();
}

}

}
