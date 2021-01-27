#include "render_technique.hpp"

#include "engine.hpp"
#include "vertex.hpp"

namespace cpt
{

descriptor_set::descriptor_set(descriptor_pool& parent, tph::descriptor_set set) noexcept
:m_parent{&parent}
,m_set{std::move(set)}
{

}


descriptor_pool::descriptor_pool(render_layout& parent, std::size_t layout_index, tph::descriptor_pool pool)
:m_parent{&parent}
,m_pool{std::move(pool)}
{
    auto& set_layout{m_parent->descriptor_set_layout(layout_index)};
    for(auto&& set : m_sets)
    {
        set = std::make_shared<descriptor_set>(*this, tph::descriptor_set{engine::instance().renderer(), m_pool, set_layout});
    }
}

descriptor_set_ptr descriptor_pool::allocate() noexcept
{
    for(auto&& set : m_sets)
    {
        if(set.use_count() == 1)
        {
            return set;
        }
    }

    return nullptr;
}

bool descriptor_pool::unused() const noexcept
{
    return std::all_of(std::begin(m_sets), std::end(m_sets), [](const descriptor_set_ptr& set)
    {
        return set.use_count() == 1;
    });
}

#ifdef CAPTAL_DEBUG
void descriptor_pool::set_name(std::string_view name)
{
    const std::string name_str{name};

    tph::set_object_name(engine::instance().renderer(), m_pool, name_str);

    for(std::size_t i{}; i < std::size(m_sets); ++i)
    {
        tph::set_object_name(engine::instance().renderer(), m_pool, name_str + " descriptor set #" + std::to_string(i));
    }
}
#endif

static std::vector<tph::descriptor_set_layout> make_set_layouts(const render_layout_info& info)
{
    std::vector<tph::descriptor_set_layout> output{};
    output.reserve(2);

    output.emplace_back(engine::instance().renderer(), info.view_bindings);
    output.emplace_back(engine::instance().renderer(), info.renderable_bindings);

    return output;
}

static std::vector<tph::descriptor_pool_size> make_pool_sizes(std::span<const tph::descriptor_set_layout_binding> bindings)
{
    std::vector<tph::descriptor_pool_size> output{};
    output.reserve(std::size(bindings));

    for(auto&& binding : bindings)
    {
        output.emplace_back(binding.type, binding.count * descriptor_pool::pool_size);
    }

    return output;
}

static std::vector<tph::push_constant_range> make_push_constant_ranges(const render_layout_info& info)
{
    std::vector<tph::push_constant_range> output{};
    output.reserve(std::size(info.view_push_constants) + std::size(info.renderable_push_constants));

    output.insert(std::end(output), std::begin(info.view_push_constants), std::end(info.view_push_constants));
    output.insert(std::end(output), std::begin(info.renderable_push_constants), std::end(info.renderable_push_constants));

    return output;
}

render_layout::render_layout(render_layout_info info)
:m_info{std::move(info)}
,m_set_layouts{make_set_layouts(m_info)}
,m_layout{engine::instance().renderer(), m_set_layouts, make_push_constant_ranges(m_info)}
{
    m_set_layout_data.reserve(2);

    m_set_layout_data.emplace_back(make_pool_sizes(info.view_bindings));
    m_set_layout_data.emplace_back(make_pool_sizes(info.renderable_bindings));
}

descriptor_set_ptr render_layout::make_set(std::uint32_t layout_index)
{
    std::lock_guard lock{m_mutex};

    auto& data{m_set_layout_data[layout_index]};

    for(auto&& pool : data.pools)
    {
        if(auto set{pool->allocate()}; set)
        {
            return set;
        }
    }

    tph::descriptor_pool pool{engine::instance().renderer(), data.sizes, static_cast<std::uint32_t>(descriptor_pool::pool_size)};
    data.pools.emplace_back(std::make_unique<descriptor_pool>(*this, std::move(pool)));

#ifdef CAPTAL_DEBUG
    if(!std::empty(m_name))
    {
        data.pools.back()->set_name(m_name + " descriptor set layout #" + std::to_string(layout_index) +  " descriptor pool #" + std::to_string(std::size(data.pools) - 1));
    }
#endif

    return data.pools.back()->allocate();
}

#ifdef CAPTAL_DEBUG
void render_layout::set_name(std::string_view name)
{
    m_name = name;

    tph::set_object_name(engine::instance().renderer(), m_set_layouts[0], m_name + " view descriptor set layout");
    tph::set_object_name(engine::instance().renderer(), m_set_layouts[1], m_name + " renderable descriptor set layout");
    tph::set_object_name(engine::instance().renderer(), m_layout, m_name + " pipeline layout");

    for(std::size_t i{}; i < std::size(m_set_layout_data); ++i)
    {
        for(std::size_t j{}; j < std::size(m_set_layout_data[i].pools); ++j)
        {
            m_set_layout_data[i].pools[j]->set_name(m_name + "descriptor set layout #" + std::to_string(i) + " descriptor pool #" + std::to_string(j));
        }
    }
}
#endif

static tph::graphics_pipeline_info make_info(const render_technique_info& info, render_technique_options options)
{
    tph::graphics_pipeline_info output{};

    if(std::empty(info.color_blend.attachments) && !static_cast<bool>(options & render_technique_options::no_default_color_blend_attachment))
    {
        output.color_blend.attachments.emplace_back();
    }
    else
    {
        output.color_blend = info.color_blend;
    }


    bool has_vertex{};
    bool has_fragment{};

    for(auto&& stage : info.stages)
    {
        output.stages.emplace_back(stage.shader, stage.name, stage.specialisation_info);

        has_vertex   = stage.shader.stage() == tph::shader_stage::vertex;
        has_fragment = stage.shader.stage() == tph::shader_stage::fragment;
    }

    if(!has_vertex)
    {
        output.stages.emplace_back(engine::instance().default_vertex_shader());
    }

    if(!has_fragment)
    {
        output.stages.emplace_back(engine::instance().default_fragment_shader());
    }

    output.vertex_input.bindings.emplace_back(0, sizeof(vertex));
    output.vertex_input.attributes.emplace_back(0, 0, tph::vertex_format::vec3f, offsetof(vertex, position));
    output.vertex_input.attributes.emplace_back(1, 0, tph::vertex_format::vec4f, offsetof(vertex, color));
    output.vertex_input.attributes.emplace_back(2, 0, tph::vertex_format::vec2f, offsetof(vertex, texture_coord));
    output.tesselation = info.tesselation;
    output.viewport.viewport_count = 1;
    output.rasterization = info.rasterization;
    output.multisample = info.multisample;
    output.depth_stencil = info.depth_stencil;
    output.dynamic_states.emplace_back(tph::dynamic_state::viewport);
    output.dynamic_states.emplace_back(tph::dynamic_state::scissor);

    return output;
}

render_technique::render_technique(const render_target_ptr& target, const render_technique_info& info, render_layout_ptr layout, render_technique_options options)
:m_layout{std::move(layout)}
,m_pipeline{engine::instance().renderer(), target->get_render_pass(), make_info(info, options), m_layout->pipeline_layout()}
{

}

#ifdef CAPTAL_DEBUG
void render_technique::set_name(std::string_view name)
{
    tph::set_object_name(engine::instance().renderer(), m_pipeline, std::string{name} + " pipeline");
}
#endif

}
