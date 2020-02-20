#include "view.hpp"

#include "render_window.hpp"
#include "render_texture.hpp"
#include "engine.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace cpt
{

view::view()
:m_buffer{{buffer_part{buffer_part_type::uniform, sizeof(uniform_data)}}}
{

}

view::view(const render_target_ptr& target, const render_technique_info& info)
:m_buffer{{buffer_part{buffer_part_type::uniform, sizeof(uniform_data)}}}
{
    set_target(target, info);
}

view::view(const render_target_ptr& target, render_technique_ptr technique)
:m_buffer{{buffer_part{buffer_part_type::uniform, sizeof(uniform_data)}}}
{
    set_target(target, std::move(technique));
}

void view::set_target(const render_target_ptr& target, const render_technique_info& info)
{
    m_target = target.get();
    m_render_technique = make_render_technique(target, info);

    m_need_upload = true;
}

void view::set_target(const render_target_ptr& target, render_technique_ptr technique)
{
    m_target = target.get();
    m_render_technique = std::move(technique);

    m_need_upload = true;
}

void view::fit_to(const render_window_ptr& window)
{
    m_viewport = tph::viewport{0.0f, 0.0f, static_cast<float>(window->width()), static_cast<float>(window->height()), 0.0f, 1.0f};
    m_scissor = tph::scissor{0, 0, window->width(), window->height()};
    m_size = glm::vec2{static_cast<float>(window->width()), static_cast<float>(window->height())};

    m_need_upload = true;
}

void view::fit_to(const render_texture_ptr& texture)
{
    m_viewport = tph::viewport{0.0f, 0.0f, static_cast<float>(texture->width()), static_cast<float>(texture->height()), 0.0f, 1.0f};
    m_scissor = tph::scissor{0, 0, texture->width(), texture->height()};
    m_size = glm::vec2{static_cast<float>(texture->width()), static_cast<float>(texture->height())};

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
