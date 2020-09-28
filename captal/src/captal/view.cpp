#include "view.hpp"

#include "render_window.hpp"
#include "render_texture.hpp"
#include "engine.hpp"

namespace cpt
{

static uniform_buffer make_uniform_buffer()
{
    return uniform_buffer{{buffer_part{buffer_part_type::uniform, sizeof(view::uniform_data)}}};
}

view::view()
:m_impl{std::make_shared<view_impl>(make_uniform_buffer())}
{
    m_push_constant_buffer.resize(static_cast<std::size_t>(engine::instance().graphics_device().limits().max_push_constant_size / 4u));
}

view::view(const render_target_ptr& target, const render_technique_info& info)
:m_impl{std::make_shared<view_impl>(make_uniform_buffer())}
{
    set_target(target, info);

    m_push_constant_buffer.resize(static_cast<std::size_t>(engine::instance().graphics_device().limits().max_push_constant_size / 4u));
}

view::view(const render_target_ptr& target, render_technique_ptr technique)
:m_impl{std::make_shared<view_impl>(make_uniform_buffer())}
{
    set_target(target, std::move(technique));

    m_push_constant_buffer.resize(static_cast<std::size_t>(engine::instance().graphics_device().limits().max_push_constant_size / 4u));
}

void view::set_target(const render_target_ptr& target, const render_technique_info& info)
{
    m_target = target.get();
    m_impl->render_technique = make_render_technique(target, info);

    m_need_upload = true;
}

void view::set_target(const render_target_ptr& target, render_technique_ptr technique)
{
    m_target = target.get();
    m_impl->render_technique = std::move(technique);

    m_need_upload = true;
}

void view::fit_to(const render_window_ptr& window)
{
    m_viewport = tph::viewport{0.0f, 0.0f, static_cast<float>(window->width()), static_cast<float>(window->height()), 0.0f, 1.0f};
    m_scissor = tph::scissor{0, 0, window->width(), window->height()};
    m_size = vec2f{static_cast<float>(window->width()), static_cast<float>(window->height())};

    m_need_upload = true;
}

void view::fit_to(const render_texture_ptr& texture)
{
    m_viewport = tph::viewport{0.0f, 0.0f, static_cast<float>(texture->width()), static_cast<float>(texture->height()), 0.0f, 1.0f};
    m_scissor = tph::scissor{0, 0, texture->width(), texture->height()};
    m_size = vec2f{static_cast<float>(texture->width()), static_cast<float>(texture->height())};

    m_need_upload = true;
}

void view::upload()
{
    if(std::exchange(m_need_upload, false))
    {
        if(m_type == view_type::orthographic)
        {
            m_impl->buffer.get<uniform_data>(0).position = vec4f{m_position, 0.0f};
            m_impl->buffer.get<uniform_data>(0).view = transpose(look_at(m_position - (m_origin * m_scale), m_position - (m_origin * m_scale) - vec3f{0.0f, 0.0f, 1.0f}, vec3f{0.0f, 1.0f, 0.0f}));
            m_impl->buffer.get<uniform_data>(0).projection = transpose(orthographic(0.0f, m_size.x() * m_scale.x(), 0.0f, m_size.y() * m_scale.y(), m_z_near * m_scale.z(), m_z_far * m_scale.z()));
        }

        m_impl->buffer.upload();
    }
}

void view::bind(tph::command_buffer& buffer)
{
    tph::cmd::set_viewport(buffer, m_viewport);
    tph::cmd::set_scissor(buffer, m_scissor);
    tph::cmd::bind_pipeline(buffer, m_impl->render_technique->pipeline());

    for(auto&& range : m_impl->render_technique->ranges())
    {
        tph::cmd::push_constants(buffer, m_impl->render_technique->pipeline_layout(), range.stages, range.offset, range.size, std::data(m_push_constant_buffer) + range.offset / 4u);
    }
}

cpt::binding& view::add_binding(std::uint32_t index, cpt::binding binding)
{
    auto [it, success] = m_impl->bindings.try_emplace(index, std::move(binding));
    assert(success && "cpt::view::add_binding called with already used binding.");

    update_uniforms();

    return it->second;
}

void view::set_binding(std::uint32_t index, cpt::binding new_binding)
{
    binding(index) = std::move(new_binding);
    update_uniforms();
}

}
