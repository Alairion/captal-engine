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

#ifndef TEPHRA_PIPELINE_HPP_INCLUDED
#define TEPHRA_PIPELINE_HPP_INCLUDED

#include "config.hpp"

#include <array>
#include <functional>
#include <filesystem>
#include <istream>

#include "vulkan/vulkan.hpp"

#include "enumerations.hpp"

namespace tph
{

class device;
class render_pass;
class shader;
class descriptor_set_layout;

struct push_constant_range
{
    shader_stage stages{};
    std::uint32_t offset{};
    std::uint32_t size{};
};

class TEPHRA_API pipeline_layout
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr pipeline_layout() = default;
    explicit pipeline_layout(device& device, std::span<descriptor_set_layout> layouts = {}, std::span<const push_constant_range> ranges = {});
    explicit pipeline_layout(device& device, std::span<std::reference_wrapper<descriptor_set_layout>> layouts = {}, std::span<const push_constant_range> ranges = {});

    explicit pipeline_layout(vulkan::pipeline_layout pipeline_layout) noexcept
    :m_pipeline_layout{std::move(pipeline_layout)}
    {

    }

    ~pipeline_layout() = default;
    pipeline_layout(const pipeline_layout&) = delete;
    pipeline_layout& operator=(const pipeline_layout&) = delete;
    pipeline_layout(pipeline_layout&&) noexcept = default;
    pipeline_layout& operator=(pipeline_layout&&) noexcept = default;

    vulkan::device_context context() const noexcept
    {
        return m_pipeline_layout.context();
    }

private:
    vulkan::pipeline_layout m_pipeline_layout{};
};

TEPHRA_API void set_object_name(device& device, const pipeline_layout& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const pipeline_layout& pipeline_layout) noexcept
{
    return pipeline_layout.m_pipeline_layout.device();
}

template<>
inline VkPipelineLayout underlying_cast(const pipeline_layout& pipeline_layout) noexcept
{
   return pipeline_layout.m_pipeline_layout;
}

class TEPHRA_API pipeline_cache
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr pipeline_cache() = default;
    explicit pipeline_cache(device& device);
    explicit pipeline_cache(device& device, const std::filesystem::path& file);
    explicit pipeline_cache(device& device, std::span<const std::uint8_t> data);
    explicit pipeline_cache(device& device, std::istream& stream);

    explicit pipeline_cache(vulkan::pipeline_cache pipeline_cache) noexcept
    :m_pipeline_cache{std::move(pipeline_cache)}
    {

    }

    ~pipeline_cache() = default;
    pipeline_cache(const pipeline_cache&) = delete;
    pipeline_cache& operator=(const pipeline_cache&) = delete;
    pipeline_cache(pipeline_cache&&) noexcept = default;
    pipeline_cache& operator=(pipeline_cache&&) noexcept = default;

    vulkan::device_context context() const noexcept
    {
        return m_pipeline_cache.context();
    }

    pipeline_cache& merge_with(pipeline_cache& other);
    pipeline_cache& merge_with(const std::vector<std::reference_wrapper<pipeline_cache>>& others);

    std::string data() const;

private:
    vulkan::pipeline_cache m_pipeline_cache{};
};

TEPHRA_API void set_object_name(device& device, const pipeline_cache& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const pipeline_cache& pipeline_cache) noexcept
{
    return pipeline_cache.m_pipeline_cache.device();
}

template<>
inline VkPipelineCache underlying_cast(const pipeline_cache& pipeline_cache) noexcept
{
   return pipeline_cache.m_pipeline_cache;
}

struct specialisation_map_entry
{
    std::uint32_t id{};
    std::uint32_t offset{};
    std::size_t size{};
};

struct specialisation_info
{
    std::vector<specialisation_map_entry> entries{};
    std::size_t size{};
    const void* data{};
};

struct pipeline_shader_stage
{
    std::reference_wrapper<tph::shader> shader;
    std::string name{"main"};
    tph::specialisation_info specialisation_info{};
};

struct vertex_input_binding
{
    std::uint32_t binding{};
    std::uint32_t stride{};
    vertex_input_rate input_rate{vertex_input_rate::vertex};
};

struct vertex_input_attribute
{
    std::uint32_t location{};
    std::uint32_t binding{};
    vertex_format format{};
    std::uint32_t offset{};
};

struct pipeline_vertex_input
{
    std::vector<vertex_input_binding> bindings{};
    std::vector<vertex_input_attribute> attributes{};
};

struct pipeline_input_assembly
{
    primitive_topology topology{primitive_topology::triangle};
    bool primitive_restart{};
};

struct pipeline_tessellation
{
    std::uint32_t patch_control_points{};
};

struct pipeline_viewport
{
    std::size_t viewport_count{};
    std::vector<viewport> viewports{};
    std::vector<scissor> scissors{};
};

struct pipeline_rasterization
{
    tph::polygon_mode polygon_mode{tph::polygon_mode::fill};
    tph::cull_mode cull_mode{tph::cull_mode::none};
    tph::front_face front_face{tph::front_face::counter_clockwise};
    float line_width{1.0f};
    bool depth_clamp{};
    bool rasterizer_discard{};
    bool depth_bias{};
    float depth_bias_constant_factor{};
    float depth_bias_clamp{};
    float depth_bias_slope_factor{};
};

struct pipeline_multisample
{
    tph::sample_count sample_count{tph::sample_count::msaa_x1};
    float sample_shading{};
    const std::uint32_t* sample_mask{};
    bool alpha_to_coverage{};
    bool alpha_to_one{};
};

struct stencil_op_description
{
    stencil_op fail_op{stencil_op::keep};
    stencil_op pass_op{stencil_op::keep};
    stencil_op depth_fail_op{stencil_op::keep};
    tph::compare_op compare_op{tph::compare_op::never};
    std::uint32_t compare_mask{};
    std::uint32_t write_mask{};
    std::uint32_t reference{};
};

struct pipeline_depth_stencil
{
    bool depth_test{};
    bool depth_write{};
    bool stencil_test{};
    bool depth_bounds_test{};
    compare_op depth_compare_op{compare_op::less};
    stencil_op_description front{};
    stencil_op_description back{};
    float min_depth_bounds{0.0f};
    float max_depth_bounds{1.0f};
};

struct pipeline_color_blend_attachment
{
    bool blend{true};
    blend_factor source_color_blend_factor{blend_factor::source_alpha};
    blend_factor destination_color_blend_factor{blend_factor::one_minus_source_alpha};
    blend_op color_blend_op{blend_op::add};
    blend_factor source_alpha_blend_factor{blend_factor::source_alpha};
    blend_factor destination_alpha_blend_factor{blend_factor::one_minus_source_alpha};
    blend_op alpha_blend_op{blend_op::add};
    color_component color_write_mask{color_component::r | color_component::g | color_component::b | color_component::a};
};

struct pipeline_color_blend
{
    bool logic_op_enable{};
    tph::logic_op logic_op{tph::logic_op::set};
    std::vector<pipeline_color_blend_attachment> attachments{};
    std::array<float, 4> blend_constants{};
};

enum class pipeline_options : std::uint32_t
{
    disable_optimization = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT,
    allow_derivatives = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
};

struct graphics_pipeline_info
{
    pipeline_options options{};
    std::vector<pipeline_shader_stage> stages{};
    pipeline_vertex_input vertex_input{};
    pipeline_input_assembly input_assembly{};
    pipeline_tessellation tesselation{};
    pipeline_viewport viewport{};
    pipeline_rasterization rasterization{};
    pipeline_multisample multisample{};
    pipeline_depth_stencil depth_stencil{};
    pipeline_color_blend color_blend{};
    std::vector<dynamic_state> dynamic_states{};
};

struct compute_pipeline_info
{
    pipeline_options options{};
    pipeline_shader_stage stage;
};

class TEPHRA_API pipeline
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr pipeline() = default;
    explicit pipeline(device& device, render_pass& render_pass, const graphics_pipeline_info& info, const pipeline_layout& layout, optional_ref<pipeline_cache> cache = nullref, optional_ref<pipeline> parent = nullref);
    explicit pipeline(device& device, render_pass& render_pass, std::uint32_t subpass, const graphics_pipeline_info& info, const pipeline_layout& layout, optional_ref<pipeline_cache> cache = nullref, optional_ref<pipeline> parent = nullref);
    explicit pipeline(device& device, const compute_pipeline_info& info, const pipeline_layout& layout, optional_ref<pipeline_cache> cache = nullref, optional_ref<pipeline> parent = nullref);

    explicit pipeline(vulkan::pipeline pipeline) noexcept
    :m_pipeline{std::move(pipeline)}
    {

    }

    ~pipeline() = default;
    pipeline(const pipeline&) = delete;
    pipeline& operator=(const pipeline&) = delete;
    pipeline(pipeline&&) noexcept = default;
    pipeline& operator=(pipeline&&) noexcept = default;

    vulkan::device_context context() const noexcept
    {
        return m_pipeline.context();
    }

    pipeline_type type() const noexcept
    {
        return m_type;
    }

private:
    vulkan::pipeline m_pipeline{};
    pipeline_type m_type{};
};

TEPHRA_API void set_object_name(device& device, const pipeline& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const pipeline& pipeline) noexcept
{
    return pipeline.m_pipeline.device();
}

template<>
inline VkPipeline underlying_cast(const pipeline& pipeline) noexcept
{
   return pipeline.m_pipeline;
}

}

template<> struct tph::enable_enum_operations<tph::pipeline_options> {static constexpr bool value{true};};

#endif
