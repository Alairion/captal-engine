#include "main_menu.hpp"

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

main_menu::main_menu(cpt::render_window_ptr window)
:m_window{std::move(window)}
,m_font{"fonts/pcsenior.ttf", cpt::load_from_file, 32}
,m_text{cpt::draw_text(m_font, u8"Jouer", cpt::color{1.0f, 1.0f, 1.0f, 1.0f})}
{
    m_text_entity = m_world.create();
    m_world.assign<cpt::components::node>(m_text_entity, scaled_window_center(m_window), glm::vec3{m_text->width() / 2, m_text->height() / 2, 0}, static_cast<float>(window_scale(m_window)));
    m_world.assign<cpt::components::drawable>(m_text_entity, m_text);

    m_view_entity = m_world.create();
    m_world.assign<cpt::components::node>(m_view_entity);
    m_world.assign<cpt::components::camera>(m_view_entity, cpt::make_view(m_window)).attachment()->fit_to(m_window);
}

void main_menu::on_enter(cpt::state_stack& stack)
{
    m_connections.push_back(m_window->on_mouse_button_pressed().connect([this](const apr::mouse_event& event)
    {
        if(event.button == apr::mouse_button::left)
        {
            m_play_button_pressed = true;

            if(above_play_button(event.x, event.y))
            {
                m_text->set_color(cpt::colors::dodgerblue);
            }
        }
    }));

    m_connections.push_back(m_window->on_mouse_button_released().connect([this, &stack](const apr::mouse_event& event)
    {
        if(static_cast<bool>(event.button & apr::mouse_button::left) && m_play_button_pressed)
        {
            if(above_play_button(event.x, event.y))
            {
                stack.add_post_update_callback([this](cpt::state_stack& stack)
                {
                    m_window->close();
                    stack.pop();
                });
            }
        }

        m_play_button_pressed = false;

        if(above_play_button(event.x, event.y))
        {
            m_text->set_color(cpt::colors::deepskyblue);
        }
        else
        {
            m_text->set_color(cpt::colors::white);
        }
    }));

    m_connections.push_back(m_window->on_mouse_moved().connect([this](const apr::mouse_event& event)
    {
        if(m_play_button_pressed)
        {
            m_text->set_color(cpt::colors::dodgerblue);
        }
        else if(above_play_button(event.x, event.y))
        {
            m_text->set_color(cpt::colors::deepskyblue);
        }
        else
        {
            m_text->set_color(cpt::colors::white);
        }
    }));

    m_connections.push_back(m_window->on_resized().connect([this](const apr::window_event& event [[maybe_unused]])
    {
        auto& text_node{m_world.get<cpt::components::node>(m_text_entity)};
        text_node.set_scale(static_cast<float>(window_scale(m_window)));
        text_node.move_to(scaled_window_center(m_window));

        auto& view_camera{m_world.get<cpt::components::camera>(m_view_entity)};
        view_camera.attachment()->fit_to(m_window);
    }));
}

void main_menu::on_leave(cpt::state_stack& stack [[maybe_unused]])
{
    m_connections.clear();
}

void main_menu::on_update(cpt::state_stack& stack [[maybe_unused]], float elapsed_time [[maybe_unused]])
{
    cpt::systems::render(m_world);
    cpt::systems::end_frame(m_world);
}

bool main_menu::above_play_button(std::int32_t x, std::int32_t y) const noexcept
{
    const std::int32_t real_x{scale_from_window(m_window, x)};
    const std::int32_t real_y{scale_from_window(m_window, y)};
    const glm::vec3 text_position{m_world.get<cpt::components::node>(m_text_entity).real_position()};

    return cpt::bounding_box_query(real_x, real_y, text_position.x, text_position.y, m_text->width(), m_text->height());
}

}

}
