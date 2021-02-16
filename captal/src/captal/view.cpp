#include "view.hpp"

#include "render_window.hpp"
#include "render_texture.hpp"
#include "engine.hpp"

namespace cpt
{

view::view(const render_target_ptr& target, const render_technique_info& info)
:m_target{target.get()}
,m_render_technique{make_render_technique(target, info)}
,m_need_upload{true}
{

}

view::view(const render_target_ptr& target, render_technique_ptr technique)
:m_target{target.get()}
,m_render_technique{std::move(technique)}
,m_need_upload{true}
{
    m_bindings.emplace(0, make_uniform_buffer(std::array{buffer_part{buffer_part_type::uniform, sizeof(view::uniform_data)}}));
}

void view::upload(memory_transfer_info& info)
{
    if(std::exchange(m_need_upload, false))
    {
        auto buffer{std::get<uniform_buffer_ptr>(m_bindings.at(0))};

        buffer->get<uniform_data>(0).view = look_at(m_position - (m_origin * m_scale), m_position - (m_origin * m_scale) - vec3f{0.0f, 0.0f, 1.0f}, vec3f{0.0f, 1.0f, 0.0f});
        buffer->get<uniform_data>(0).projection = orthographic(0.0f, m_size.x() * m_scale.x(), 0.0f, m_size.y() * m_scale.y(), m_z_near * m_scale.z(), m_z_far * m_scale.z());

        buffer->upload();
        info.keeper.keep(std::move(buffer));
    }
}

void view::bind(tph::command_buffer& buffer)
{
    if(std::exchange(m_need_descriptor_update, false))
    {
        m_set.reset();
        m_set = m_render_technique->layout()->make_set(0);

        std::vector<tph::descriptor_write> writes{};
        writes.reserve(std::size(m_render_technique->layout()->info().view_bindings));

        for(auto&& binding : m_render_technique->layout()->info().view_bindings)
        {
            const auto it{m_bindings.find(binding.binding)};

            if(it == std::end(m_bindings))
            {
                const auto fallback{m_render_technique->layout()->binding(0, binding.binding)};
                assert(fallback && "cpt::view::bind can not find any suitable binding, neither the view nor the render layout have a binding for specified index.");

                writes.emplace_back(make_descriptor_write(m_set->set(), binding.binding, fallback));
            }
            else
            {
                writes.emplace_back(make_descriptor_write(m_set->set(), binding.binding, it->second));
            }
        }

        tph::write_descriptors(engine::instance().renderer(), writes);
    }

    tph::cmd::set_viewport(buffer, m_viewport);
    tph::cmd::set_scissor(buffer, m_scissor);

    tph::cmd::bind_pipeline(buffer, m_render_technique->pipeline());
    tph::cmd::bind_descriptor_set(buffer, 0, m_set->set(), m_render_technique->layout()->pipeline_layout());

    m_push_constants.push(buffer, m_render_technique->layout()->pipeline_layout(), m_render_technique->layout()->info().view_push_constants);
}

void view::keep(asynchronous_resource_keeper& keeper) const
{
    keeper.keep(m_render_technique);
    keeper.keep(m_set);

    for(auto&& [index, binding] : m_bindings)
    {
        keeper.keep(get_binding_resource(binding));
    }
}

void view::fit(std::uint32_t width, std::uint32_t height)
{
    m_viewport = tph::viewport{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
    m_scissor = tph::scissor{0, 0, width, height};
    m_size = vec2f{static_cast<float>(width), static_cast<float>(height)};

    m_need_upload = true;
}

void view::fit(const render_window_ptr& window)
{
    fit(window->width(), window->height());
}

void view::fit(const render_texture_ptr& texture)
{
    fit(texture->width(), texture->height());
}

void view::set_binding(std::uint32_t index, cpt::binding binding)
{
    const auto to_set{m_bindings.find(index)};

    if(to_set != std::end(m_bindings))
    {
        to_set->second = std::move(binding);
    }
    else
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
            }
        };

        const auto& bindings{m_render_technique->layout()->info().view_bindings};
        const auto  it      {std::find_if(std::begin(bindings), std::end(bindings), predicate)};

        assert(it != std::end(bindings) && "cpt::view::set_binding index must correspond to one of the render layout's bindings.");
        assert(it->type == convert_binding_type(get_binding_type(binding)) && "cpt::view::set_binding binding's type does not correspond to the layout binding's type at index.");
    #endif

        m_bindings.emplace(index, std::move(binding));
    }

    m_need_descriptor_update = true;
}


}
