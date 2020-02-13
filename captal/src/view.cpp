#include "view.hpp"

#include "render_window.hpp"
#include "engine.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace cpt
{

view::view()
:m_buffer{{buffer_part{buffer_part_type::uniform, sizeof(uniform_data)}}}
{

}

view::view(render_target& target)
:m_buffer{{buffer_part{buffer_part_type::uniform, sizeof(uniform_data)}}}
{
    set_target(target);
}

void view::set_target(render_target& target)
{
    m_target = &target;

    m_need_upload = true;
}

void view::fit_to(render_window& window)
{
    m_viewport = tph::viewport{0.0f, 0.0f, static_cast<float>(window.width()), static_cast<float>(window.height()), 0.0f, 1.0f};
    m_scissor = tph::scissor{0, 0, window.width(), window.height()};
    m_size = glm::vec2{static_cast<float>(window.width()), static_cast<float>(window.height())};

    m_need_upload = true;
}

void view::update()
{
    m_need_upload = true;
}

void view::upload()
{
    if(std::exchange(m_need_upload, false))
    {
        if(m_type == view_type::orthographic)
        {
            m_buffer.get<uniform_data>(0).position = glm::vec4{m_position, 0.0f};
            m_buffer.get<uniform_data>(0).view = glm::lookAt(m_position - (m_origin * m_scale), m_position - (m_origin * m_scale) - glm::vec3{0.0f, 0.0f, 1.0f}, glm::vec3{0.0f, 1.0f, 0.0f});
            m_buffer.get<uniform_data>(0).projection = glm::ortho(0.0f, m_size.x * m_scale, 0.0f, m_size.y * m_scale, 1.0f, 0.0f);
        }

        m_buffer.upload();
    }
}

}
