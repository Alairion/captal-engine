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

class renderer;
class render_target;
class shader;
class descriptor_set_layout;

struct push_constant_range
{
    shader_stage stages{};
    std::uint32_t offset{};
    std::uint32_t size{};
};

class pipeline_layout
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr pipeline_layout() = default;
    pipeline_layout(renderer& renderer, const std::vector<std::reference_wrapper<descriptor_set_layout>>& layouts = {}, const std::vector<push_constant_range>& ranges = {});

    ~pipeline_layout() = default;
    pipeline_layout(const pipeline_layout&) = delete;
    pipeline_layout& operator=(const pipeline_layout&) = delete;
    pipeline_layout(pipeline_layout&&) noexcept = default;
    pipeline_layout& operator=(pipeline_layout&&) noexcept = default;

private:
    vulkan::pipeline_layout m_pipeline_layout{};
};

template<>
inline VkPipelineLayout underlying_cast(const pipeline_layout& pipeline_layout) noexcept
{
   return pipeline_layout.m_pipeline_layout;
}

class pipeline_cache
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr pipeline_cache() = default;
    pipeline_cache(renderer& renderer);
    pipeline_cache(renderer& renderer, const std::filesystem::path& file);
    pipeline_cache(renderer& renderer, const std::string_view& data);
    pipeline_cache(renderer& renderer, std::istream& stream);

    ~pipeline_cache() = default;
    pipeline_cache(const pipeline_cache&) = delete;
    pipeline_cache& operator=(const pipeline_cache&) = delete;
    pipeline_cache(pipeline_cache&&) noexcept = default;
    pipeline_cache& operator=(pipeline_cache&&) noexcept = default;

    pipeline_cache& merge_with(pipeline_cache& other);
    pipeline_cache& merge_with(const std::vector<std::reference_wrapper<pipeline_cache>>& others);

    std::string data() const;

private:
    VkDevice m_device{};
    vulkan::pipeline_cache m_pipeline_cache{};
};

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
    shader& shader;
    std::string name{"main"};
    specialisation_info specialisation_info{};
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
    polygon_mode polygon_mode{polygon_mode::fill};
    cull_mode cull_mode{cull_mode::none};
    front_face front_face{front_face::counter_clockwise};
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
    compare_op compare_op{compare_op::never};
    std::uint32_t compare_mask{};
    std::uint32_t write_mask{};
    std::uint32_t reference{};
};

struct pipeline_depth_stencil
{
    bool depth_test{};
    bool depth_write{true};
    bool stencil_test{};
    bool depth_bounds_test{};
    compare_op depth_compare_op{compare_op::less_or_equal};
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
    logic_op logic_op{logic_op::set};
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

enum class pipeline_type : std::uint32_t
{
    graphics = VK_PIPELINE_BIND_POINT_GRAPHICS,
    compute = VK_PIPELINE_BIND_POINT_COMPUTE,
};

class pipeline
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr pipeline() = default;
    pipeline(renderer& renderer, render_target& render_target, const graphics_pipeline_info& info, const pipeline_layout& layout, optional_ref<pipeline_cache> cache = nullref, optional_ref<pipeline> parent = nullref);
    pipeline(renderer& renderer, render_target& render_target, std::uint32_t subpass, const graphics_pipeline_info& info, const pipeline_layout& layout, optional_ref<pipeline_cache> cache = nullref, optional_ref<pipeline> parent = nullref);
    pipeline(renderer& renderer, const compute_pipeline_info& info, const pipeline_layout& layout, optional_ref<pipeline_cache> cache = nullref, optional_ref<pipeline> parent = nullref);

    ~pipeline() = default;
    pipeline(const pipeline&) = delete;
    pipeline& operator=(const pipeline&) = delete;
    pipeline(pipeline&&) noexcept = default;
    pipeline& operator=(pipeline&&) noexcept = default;

    pipeline_type type() const noexcept
    {
        return m_type;
    }

private:
    vulkan::pipeline m_pipeline{};
    pipeline_type m_type{};
};

template<>
inline VkPipeline underlying_cast(const pipeline& pipeline) noexcept
{
   return pipeline.m_pipeline;
}

}

template<> struct tph::enable_enum_operations<tph::pipeline_options> {static constexpr bool value{true};};

#endif
