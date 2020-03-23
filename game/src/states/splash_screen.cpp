#include "splash_screen.hpp"

#include <captal/components/node.hpp>
#include <captal/components/drawable.hpp>
#include <captal/components/camera.hpp>

#include <captal/systems/frame.hpp>
#include <captal/systems/render.hpp>

#include "../viewport_compute.hpp"
#include "main_menu.hpp"

namespace mpr
{

namespace states
{

splash_screen::splash_screen(cpt::render_window_ptr window)
:m_window{std::move(window)}
,m_font{std::filesystem::u8path("fonts/basis33.ttf"), 16}
,m_text{cpt::draw_text(m_font, u8"Ceci est un splash screen...\nOu plus si affinité...", cpt::color{1.0f, 1.0f, 1.0f, 0.0f})}
{
    m_text_entity = m_world.create();
    m_world.assign<cpt::components::node>(m_text_entity, scaled_window_center(m_window), glm::vec3{m_text->width() / 2, m_text->height() / 2, 0}, static_cast<float>(window_scale(m_window)));
    m_world.assign<cpt::components::drawable>(m_text_entity, m_text);

    m_view_entity = m_world.create();
    m_world.assign<cpt::components::node>(m_view_entity);
    m_world.assign<cpt::components::camera>(m_view_entity, cpt::make_view(m_window)).attachment()->fit_to(m_window);
}

void splash_screen::on_enter(cpt::state_stack& stack [[maybe_unused]])
{
    m_resize_connection = m_window->on_resized().connect([this](const apr::window_event& event [[maybe_unused]])
    {
        auto& text_node{m_world.get<cpt::components::node>(m_text_entity)};
        text_node.set_scale(static_cast<float>(window_scale(m_window)));
        text_node.move_to(scaled_window_center(m_window));

        auto& view_camera{m_world.get<cpt::components::camera>(m_view_entity)};
        view_camera.attachment()->fit_to(m_window);
    });
}

void splash_screen::on_leave(cpt::state_stack& stack [[maybe_unused]])
{
    m_resize_connection.disconnect();
}

void splash_screen::on_update(cpt::state_stack& stack [[maybe_unused]], float elapsed_time)
{
    m_time += elapsed_time;

    if(m_time >= 4.0f)
    {
        m_text->set_color(cpt::color{1.0f, 1.0f, 1.0f, 0.0f});

        stack.add_post_update_callback([this](cpt::state_stack& stack)
        {
            stack.reset(cpt::make_state<main_menu>(m_window));
        });
    }
    else if(m_time >= 3.0f)
    {
        m_text->set_color(cpt::color{1.0f, 1.0f, 1.0f, (4.0f - m_time) / 1.0f});
    }
    else
    {
        m_text->set_color(cpt::color{1.0f, 1.0f, 1.0f, m_time / 1.0f});
    }

    cpt::systems::render(m_world);
    cpt::systems::end_frame(m_world);
}

}

}
