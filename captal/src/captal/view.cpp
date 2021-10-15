//MIT License
//
//Copyright (c) 2021 Alexy Pellegrini
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "view.hpp"

#include "render_window.hpp"
#include "render_texture.hpp"
#include "engine.hpp"

namespace cpt
{

view::view(const render_target_ptr& target, const render_technique_info& info, render_layout_ptr layout, render_technique_options options)
:view{target, make_render_technique(target, info, layout, options)}
{

}

view::view(const render_target_ptr& target, render_technique_ptr technique)
:m_target{target.get()}
,m_render_technique{std::move(technique)}
,m_need_upload{true}
{
    m_bindings.set(0, make_uniform_buffer(std::array{buffer_part{buffer_part_type::uniform, sizeof(view::uniform_data)}}));
}

void view::upload(memory_transfer_info info)
{
    if(std::exchange(m_need_upload, false))
    {
        auto buffer{std::get<uniform_buffer_ptr>(m_bindings.get(0))};

        buffer->get<uniform_data>(0).view = look_at(m_position - (m_origin * m_scale), m_position - (m_origin * m_scale) - vec3f{0.0f, 0.0f, 1.0f}, vec3f{0.0f, 1.0f, 0.0f});
        buffer->get<uniform_data>(0).projection = orthographic(0.0f, m_size.x() * m_scale.x(), 0.0f, m_size.y() * m_scale.y(), m_z_near * m_scale.z(), m_z_far * m_scale.z());

        buffer->upload();
        info.keeper.keep(std::move(buffer));
    }
}

void view::bind(frame_render_info info)
{
    if(std::exchange(m_need_descriptor_update, false))
    {
        m_set.reset();
        m_to_keep.clear();

        const auto to_bind{m_render_technique->layout()->bindings(render_layout::view_index)};

        m_set = m_render_technique->layout()->make_set(render_layout::view_index);

        #ifdef CAPTAL_DEBUG
        if(!std::empty(m_name))
        {
            tph::set_object_name(engine::instance().renderer(), m_set->set(), m_name + " descriptor set");
        }
        #endif

        std::vector<tph::descriptor_write> writes{};
        writes.reserve(std::size(to_bind));

        for(auto&& binding : to_bind)
        {
            const auto local{m_bindings.try_get(binding.binding)};

            if(local)
            {
                writes.emplace_back(make_descriptor_write(m_set->set(), binding.binding, *local));
                m_to_keep.emplace_back(get_binding_resource(*local));
            }
            else
            {
                const auto fallback{m_render_technique->layout()->default_binding(render_layout::view_index, binding.binding)};
                assert(fallback && "cpt::view::bind can not find any suitable binding, neither the view nor the render layout have a binding for specified index.");

                writes.emplace_back(make_descriptor_write(m_set->set(), binding.binding, *fallback));
                m_to_keep.emplace_back(get_binding_resource(*fallback));
            }
        }

        tph::write_descriptors(engine::instance().renderer(), writes);
    }

    tph::cmd::set_viewport(info.buffer, m_viewport);
    tph::cmd::set_scissor(info.buffer, m_scissor);

    tph::cmd::bind_pipeline(info.buffer, m_render_technique->pipeline());
    tph::cmd::bind_descriptor_set(info.buffer, 0, m_set->set(), m_render_technique->layout()->pipeline_layout());

    m_push_constants.push(info.buffer, m_render_technique->layout(), render_layout::view_index);

    info.keeper.keep(std::begin(m_to_keep), std::end(m_to_keep));
    info.keeper.keep(m_set);
    info.keeper.keep(m_render_technique);
}

void view::fit(std::uint32_t width, std::uint32_t height)
{
    m_viewport = tph::viewport{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
    m_scissor = tph::scissor{0, 0, width, height};
    m_size = vec2f{static_cast<float>(width), static_cast<float>(height)};

    m_need_upload = true;
}

void view::fit(const window_ptr& window)
{
    fit(window->width(), window->height());
}

void view::fit(const texture_ptr& texture)
{
    fit(texture->width(), texture->height());
}

void view::set_binding(std::uint32_t index, cpt::binding binding)
{
#ifndef NDEBUG
    const auto predicate = [index](auto&& other)
    {
        return other.binding == index;
    };

    const auto convert_binding_type = [](binding_type type)
    {
        switch(type)
        {
            case binding_type::texture:        return tph::descriptor_type::image_sampler;
            case binding_type::uniform_buffer: return tph::descriptor_type::uniform_buffer;
            case binding_type::storage_buffer: return tph::descriptor_type::storage_buffer;
            default: std::terminate();
        }
    };

    const auto& bindings{m_render_technique->layout()->bindings(render_layout::view_index)};
    const auto  it      {std::find_if(std::begin(bindings), std::end(bindings), predicate)};

    assert(it != std::end(bindings) && "cpt::view::set_binding index must correspond to one of the render layout's bindings.");
    assert(it->type == convert_binding_type(get_binding_type(binding)) && "cpt::view::set_binding binding's type does not correspond to the layout binding's type at index.");
#endif

    m_bindings.set(index, std::move(binding));
    m_need_descriptor_update = true;
}

#ifdef CAPTAL_DEBUG
void view::set_name(std::string_view name)
{
    m_name = name;

    if(m_set)
    {
        tph::set_object_name(engine::instance().renderer(), m_set->set(), m_name + " descriptor set");
    }
}
#endif

}
