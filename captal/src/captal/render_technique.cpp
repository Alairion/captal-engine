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


descriptor_pool::descriptor_pool(render_layout& parent, tph::descriptor_set_layout& layout, tph::descriptor_pool pool)
:m_parent{&parent}
,m_pool{std::move(pool)}
{
    for(auto&& set : m_sets)
    {
        set = std::make_shared<descriptor_set>(*this, tph::descriptor_set{engine::instance().renderer(), m_pool, layout});
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

render_layout::layout_data render_layout::make_layout_data(const render_layout_info& info)
{
    layout_data output{};
    output.layout = tph::descriptor_set_layout{engine::instance().renderer(), info.bindings};
    output.sizes = make_pool_sizes(info.bindings);

    output.bindings = info.bindings;
    output.push_constants = info.push_constants;

    for(auto&& [index, binding] : info.default_bindings)
    {
        output.default_bindings.set(index, binding);
    }

    return output;
}

std::vector<std::reference_wrapper<tph::descriptor_set_layout>> render_layout::make_layout_refs(std::span<layout_data> layouts)
{
    std::vector<std::reference_wrapper<tph::descriptor_set_layout>> output{};
    output.reserve(std::size(layouts));

    for(auto&& layout : layouts)
    {
        output.emplace_back(layout.layout);
    }

    return output;
}

std::vector<tph::push_constant_range> render_layout::make_push_constant_ranges(std::span<const layout_data> layouts)
{
    std::size_t ranges_count{};
    for(auto&& layout : layouts)
    {
        ranges_count += std::size(layout.push_constants);
    }

    std::vector<tph::push_constant_range> output{};
    output.reserve(ranges_count);

    for(auto&& layout : layouts)
    {
        output.insert(std::end(output), std::begin(layout.push_constants), std::end(layout.push_constants));
    }

    return output;
}

render_layout::render_layout(const render_layout_info& view_info, const render_layout_info& renderable_info, std::span<const render_layout_info> user_info)
{
    m_layout_data.reserve(2 + std::size(user_info));
    m_layout_data.emplace_back(make_layout_data(view_info));
    m_layout_data.emplace_back(make_layout_data(renderable_info));

    for(auto&& user : user_info)
    {
        m_layout_data.emplace_back(make_layout_data(user));
    }

    auto descriptor_set_layouts{make_layout_refs(m_layout_data)};
    auto push_constant_ranges  {make_push_constant_ranges(m_layout_data)};

    m_layout = tph::pipeline_layout{engine::instance().renderer(), descriptor_set_layouts, push_constant_ranges};
}

descriptor_set_ptr render_layout::make_set(std::uint32_t layout_index)
{
    assert(layout_index < 2 && "cpt::render_layout does not support custom descriptor set layouts yet.");

    std::lock_guard lock{m_mutex};

    auto& data{m_layout_data[layout_index]};

    for(auto&& pool : data.pools)
    {
        if(auto set{pool->allocate()}; set)
        {
            return set;
        }
    }

    tph::descriptor_pool pool{engine::instance().renderer(), data.sizes, static_cast<std::uint32_t>(descriptor_pool::pool_size)};
    data.pools.emplace_back(std::make_unique<descriptor_pool>(*this, data.layout, std::move(pool)));

#ifdef CAPTAL_DEBUG
    if(!std::empty(m_name))
    {
        if(layout_index == view_index)
        {
            data.pools.back()->set_name(m_name + " view descriptor set layout descriptor pool #" + std::to_string(std::size(data.pools) - 1));
        }
        else if(layout_index == renderable_index)
        {
            data.pools.back()->set_name(m_name + " renderable descriptor set layout descriptor pool #" + std::to_string(std::size(data.pools) - 1));
        }
        else
        {
            data.pools.back()->set_name(m_name + " user descriptor set layout #" + std::to_string(layout_index - 2) + " descriptor pool #" + std::to_string(std::size(data.pools) - 1));
        }
    }
#endif

    return data.pools.back()->allocate();
}

#ifdef CAPTAL_DEBUG
void render_layout::set_name(std::string_view name)
{
    m_name = name;

    tph::set_object_name(engine::instance().renderer(), m_layout_data[0].layout, m_name + " view descriptor set layout");
    tph::set_object_name(engine::instance().renderer(), m_layout_data[1].layout, m_name + " renderable descriptor set layout");
    tph::set_object_name(engine::instance().renderer(), m_layout, m_name + " pipeline layout");

    for(std::size_t i{}; i < std::size(m_layout_data[0].pools); ++i)
    {
        m_layout_data[view_index].pools[i]->set_name(m_name + " view descriptor set layout descriptor pool #" + std::to_string(i));
    }

    for(std::size_t i{}; i < std::size(m_layout_data[1].pools); ++i)
    {
        m_layout_data[renderable_index].pools[i]->set_name(m_name + " renderable descriptor set layout descriptor pool #" + std::to_string(i));
    }

    if(std::size(m_layout_data) > 2)
    {
        for(std::size_t i{}; i < std::size(m_layout_data) - 2; ++i)
        {
            auto& layout{m_layout_data[i + 2]};

            for(std::size_t j{}; j < std::size(m_layout_data[1].pools); ++j)
            {
                layout.pools[i]->set_name(m_name + " user descriptor set layout #" + std::to_string(i) + " descriptor pool #" + std::to_string(j));
            }
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

        has_vertex   = has_vertex   || stage.shader.get().stage() == tph::shader_stage::vertex;
        has_fragment = has_fragment || stage.shader.get().stage() == tph::shader_stage::fragment;
    }

    if(!has_vertex)
    {
        output.stages.emplace_back(engine::instance().default_vertex_shader());
    }

    if(!has_fragment)
    {
        output.stages.emplace_back(engine::instance().default_fragment_shader());
    }

    output.vertex_input.bindings.emplace_back(0, static_cast<std::uint32_t>(sizeof(vertex)));
    output.vertex_input.attributes.emplace_back(0, 0, tph::vertex_format::vec3f, static_cast<std::uint32_t>(offsetof(vertex, position)));
    output.vertex_input.attributes.emplace_back(1, 0, tph::vertex_format::vec4f, static_cast<std::uint32_t>(offsetof(vertex, color)));
    output.vertex_input.attributes.emplace_back(2, 0, tph::vertex_format::vec2f, static_cast<std::uint32_t>(offsetof(vertex, texture_coord)));
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
:m_layout{layout ? std::move(layout) : engine::instance().default_render_layout()}
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
