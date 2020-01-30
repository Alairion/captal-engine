#include "render_technique.hpp"

#include "engine.hpp"
#include "vertex.hpp"

#include <iostream>

namespace cpt
{

descriptor_set::descriptor_set(descriptor_pool& parent, tph::descriptor_set set) noexcept
:m_parent{&parent}
,m_set{std::move(set)}
{

}


descriptor_pool::descriptor_pool(render_technique& parent, tph::descriptor_pool pool)
:m_parent{&parent}
,m_pool{std::move(pool)}
{
    for(auto&& set : m_sets)
        set = std::make_shared<descriptor_set>(*this, tph::descriptor_set{engine::instance().renderer(), m_pool, m_parent->descriptor_set_layout()});
}

std::optional<descriptor_set_ptr> descriptor_pool::allocate()
{
    for(auto&& set : m_sets)
    {
        if(set.use_count() == 1)
            return set;
    }

    return std::nullopt;
}

bool descriptor_pool::unused() const noexcept
{
    return std::all_of(std::begin(m_sets), std::end(m_sets), [](const descriptor_set_ptr& data){return data.use_count() == 1;});
}

std::vector<tph::descriptor_set_layout_binding> make_bindings(const std::vector<tph::descriptor_set_layout_binding>& info)
{
    std::vector<tph::descriptor_set_layout_binding> output{};

    //0: view uniform
    //1: model uniform
    //2: texture sampler
    //3: normal map sampler
    //4: height map sampler
    output.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::vertex, 0, tph::descriptor_type::uniform_buffer});
    output.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::vertex, 1, tph::descriptor_type::uniform_buffer});
    output.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, 2, tph::descriptor_type::image_sampler});/*
    output.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, 3, tph::descriptor_type::image_sampler});
    output.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, 4, tph::descriptor_type::image_sampler});*/

    output.insert(std::end(output), std::begin(info), std::end(info));

    return output;
}

tph::pipeline_info make_info(const render_technique_info& info)
{
    tph::pipeline_info output{};

    if(std::empty(info.color_blend.attachments))
    {
        output.color_blend.attachments.emplace_back().blend = true;
    }
    else
    {
        output.color_blend = info.color_blend;
    }

    if(std::empty(info.stages))
    {
        output.stages.push_back(tph::pipeline_shader_stage{engine::instance().default_vertex_shader()});
        output.stages.push_back(tph::pipeline_shader_stage{engine::instance().default_fragment_shader()});
    }
    else
    {
        for(auto&& stage : info.stages)
            output.stages.push_back(tph::pipeline_shader_stage{stage.shader, stage.name, stage.specialisation_info});
    }

    output.vertex_input.bindings.push_back(tph::vertex_input_binding{0, sizeof(vertex)});
    output.vertex_input.attributes.push_back(tph::vertex_input_attribute{0, 0, tph::vertex_format::vec3f, offsetof(vertex, position)});
    output.vertex_input.attributes.push_back(tph::vertex_input_attribute{1, 0, tph::vertex_format::vec4f, offsetof(vertex, color)});
    output.vertex_input.attributes.push_back(tph::vertex_input_attribute{2, 0, tph::vertex_format::vec2f, offsetof(vertex, texture_coord)});
    output.tesselation = info.tesselation;
    output.viewport.viewport_count = 1;
    output.rasterization = info.rasterization;
    output.multisample = info.multisample;
    output.depth_stencil = info.depth_stencil;
    output.dynamic_states.push_back(tph::dynamic_state::viewport);
    output.dynamic_states.push_back(tph::dynamic_state::scissor);

    return output;
}


render_technique::render_technique(render_target& target, const render_technique_info& info)
:m_bindings{make_bindings(info.stages_bindings)}
,m_ranges{info.push_constant_ranges}
,m_descriptor_set_layout{engine::instance().renderer(), m_bindings}
,m_layout{engine::instance().renderer(), {m_descriptor_set_layout}, m_ranges}
,m_pipeline{engine::instance().renderer(), target.get_target(), make_info(info), m_layout}
{
    m_sizes.reserve(std::size(m_bindings));
    for(auto&& binding : m_bindings)
        m_sizes.push_back(tph::descriptor_pool_size{binding.type, static_cast<std::uint32_t>(binding.count * descriptor_pool::pool_size)});
}

descriptor_set_ptr render_technique::make_set()
{
    std::lock_guard lock{m_mutex};

    for(auto&& pool : m_pools)
    {
        if(auto set{pool->allocate()}; set.has_value())
            return std::move(set).value();
    }

    m_pools.push_back(std::make_unique<descriptor_pool>(*this, tph::descriptor_pool{engine::instance().renderer(), m_sizes, static_cast<std::uint32_t>(descriptor_pool::pool_size)}));
    return m_pools.back()->allocate().value();
}

}
