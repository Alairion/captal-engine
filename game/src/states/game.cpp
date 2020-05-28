#include "game.hpp"

#include <captal/components/node.hpp>
#include <captal/components/camera.hpp>
#include <captal/components/drawable.hpp>

#include <captal/systems/render.hpp>
#include <captal/systems/frame.hpp>

#include <captal/algorithm.hpp>

#include "../viewport_compute.hpp"

namespace mpr
{

namespace states
{

game::game(cpt::render_window_ptr window)
:m_window{std::move(window)}
{
    m_view_entity = m_world.create();
    m_world.assign<cpt::components::node>(m_view_entity);
    m_world.assign<cpt::components::camera>(m_view_entity, cpt::make_view(m_window)).attachment()->fit_to(m_window);
}

void game::on_enter(cpt::state_stack& stack [[maybe_unused]])
{

}

void game::on_leave(cpt::state_stack& stack [[maybe_unused]])
{
    m_connections.clear();
}

void game::on_update(cpt::state_stack& stack, float elapsed_time)
{
    if(!stack.is_top(this))
    {
        return;
    }

    m_time += time::minute{elapsed_time};


    cpt::systems::render(m_world);
    cpt::systems::end_frame(m_world);
}

}

}
