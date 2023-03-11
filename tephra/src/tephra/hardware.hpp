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

#ifndef TEPHRA_HARDWARE_HPP_INCLUDED
#define TEPHRA_HARDWARE_HPP_INCLUDED

#include "config.hpp"

#include <array>
#include <string>

#include "vulkan/vulkan.hpp"

#include "enumerations.hpp"
#include "surface.hpp"

namespace tph
{

class application;
class surface;

enum class physical_device_type : std::uint32_t
{
    unknown = VK_PHYSICAL_DEVICE_TYPE_OTHER,
    integrated = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
    discrete = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
    virtualised = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
    cpu = VK_PHYSICAL_DEVICE_TYPE_CPU,
};

struct physical_device_properties
{
    physical_device_type type{};
    tph::version api_version{};
    std::string name{};
    std::array<std::uint8_t, 16> uuid{};
};

struct physical_device_features
{
    bool robust_buffer_access{};
    bool full_draw_index_uint32{};
    bool image_cube_array{};
    bool independent_blend{};
    bool geometry_shader{};
    bool tessellation_shader{};
    bool sample_shading{};
    bool dual_src_blend{};
    bool logic_op{};
    bool multi_draw_indirect{};
    bool draw_indirect_first_instance{};
    bool depth_clamp{};
    bool depth_bias_clamp{};
    bool fill_mode_non_solid{};
    bool depth_bounds{};
    bool wide_lines{};
    bool large_points{};
    bool alpha_to_one{};
    bool multi_viewport{};
    bool sampler_anisotropy{};
    bool occlusion_query_precise{};
    bool pipeline_statistics_query{};
    bool vertex_pipeline_stores_and_atomics{};
    bool fragment_stores_and_atomics{};
    bool shader_tessellation_and_geometry_point_size{};
    bool shader_image_gather_extended{};
    bool shader_storage_image_extended_formats{};
    bool shader_storage_image_multisample{};
    bool shader_storage_image_read_without_format{};
    bool shader_storage_image_write_without_format{};
    bool shader_uniform_buffer_array_dynamic_indexing{};
    bool shader_sampled_image_array_dynamic_indexing{};
    bool shader_storage_buffer_array_dynamic_indexing{};
    bool shader_storage_image_array_dynamic_indexing{};
    bool shader_clip_distance{};
    bool shader_cull_distance{};
    bool shader_float64{};
    bool shader_int64{};
    bool shader_int16{};
    bool shader_resource_residency{};
    bool shader_resource_min_lod{};
    bool variable_multisample_rate{};
    bool inherited_queries{};
};

struct physical_device_limits
{
    std::uint32_t                max_1d_texture_size{};
    std::uint32_t                max_2d_texture_size{};
    std::uint32_t                max_3d_texture_size{};
    std::uint32_t                max_cube_texture_size{};
    std::uint32_t                max_texture_array_layers{};
    std::uint32_t                max_texel_buffer_elements{};
    std::uint32_t                max_uniform_buffer_range{};
    std::uint32_t                max_storage_buffer_range{};
    std::uint32_t                max_push_constants_size{};
    std::uint32_t                max_memory_allocation_count{};
    std::uint32_t                max_sampler_allocation_count{};
    std::uint32_t                max_bound_descriptor_sets{};
    std::uint32_t                max_per_stage_descriptor_samplers{};
    std::uint32_t                max_per_stage_descriptor_uniform_buffers{};
    std::uint32_t                max_per_stage_descriptor_storage_buffers{};
    std::uint32_t                max_per_stage_descriptor_sampled_images{};
    std::uint32_t                max_per_stage_descriptor_storage_images{};
    std::uint32_t                max_per_stage_descriptor_input_attachments{};
    std::uint32_t                max_per_stage_resources{};
    std::uint32_t                max_descriptor_set_samplers{};
    std::uint32_t                max_descriptor_set_uniform_buffers{};
    std::uint32_t                max_descriptor_set_uniform_buffers_dynamic{};
    std::uint32_t                max_descriptor_set_storage_buffers{};
    std::uint32_t                max_descriptor_set_storage_buffers_dynamic{};
    std::uint32_t                max_descriptor_set_sampled_images{};
    std::uint32_t                max_descriptor_set_storage_images{};
    std::uint32_t                max_descriptor_set_input_attachments{};
    std::uint32_t                max_vertex_input_attributes{};
    std::uint32_t                max_vertex_input_bindings{};
    std::uint32_t                max_vertex_input_attribute_offset{};
    std::uint32_t                max_vertex_input_binding_stride{};
    std::uint32_t                max_vertex_output_components{};
    std::uint32_t                max_tessellation_generation_level{};
    std::uint32_t                max_tessellation_patch_size{};
    std::uint32_t                max_tessellation_control_per_vertex_input_components{};
    std::uint32_t                max_tessellation_control_per_vertex_output_components{};
    std::uint32_t                max_tessellation_control_per_patch_output_components{};
    std::uint32_t                max_tessellation_control_total_output_components{};
    std::uint32_t                max_tessellation_evaluation_input_components{};
    std::uint32_t                max_tessellation_evaluation_output_components{};
    std::uint32_t                max_geometry_shader_invocations{};
    std::uint32_t                max_geometry_input_components{};
    std::uint32_t                max_geometry_output_components{};
    std::uint32_t                max_geometry_output_vertices{};
    std::uint32_t                max_geometry_total_output_components{};
    std::uint32_t                max_fragment_input_components{};
    std::uint32_t                max_fragment_output_attachments{};
    std::uint32_t                max_fragment_dual_src_attachments{};
    std::uint32_t                max_fragment_combined_output_resources{};
    std::uint32_t                max_compute_shared_memory_size{};
    std::array<std::uint32_t, 3> max_compute_work_group_count{};
    std::uint32_t                max_compute_work_group_invocations{};
    std::array<std::uint32_t, 3> max_compute_work_group_size{};
    std::uint32_t                sub_pixel_precision_bits{};
    std::uint32_t                sub_texel_precision_bits{};
    std::uint32_t                mipmap_precision_bits{};
    std::uint32_t                max_draw_indexed_index_value{};
    std::uint32_t                max_draw_indirect_count{};
    float                        max_sampler_lod_bias{};
    float                        max_sampler_anisotropy{};
    std::uint32_t                max_viewports{};
    std::array<std::uint32_t, 2> max_viewport_dimensions{};
    std::array<float, 2>         viewport_bounds_range{};
    std::uint32_t                viewport_sub_pixel_bits{};
    std::uint64_t                min_texel_buffer_offset_alignment{};
    std::uint64_t                min_uniform_buffer_alignment{};
    std::uint64_t                min_storage_buffer_alignment{};
    std::uint32_t                max_framebuffer_width{};
    std::uint32_t                max_framebuffer_height{};
    std::uint32_t                max_framebuffer_layers{};
    sample_count                 framebuffer_color_sample_counts{};
    sample_count                 framebuffer_depth_sample_counts{};
    sample_count                 framebuffer_stencil_sample_counts{};
    sample_count                 framebuffer_no_attachments_sample_counts{};
    std::uint32_t                max_color_attachments{};
    sample_count                 sampled_image_color_sample_counts{};
    sample_count                 sampled_image_integer_sample_counts{};
    sample_count                 sampled_image_depth_sample_counts{};
    sample_count                 sampled_image_stencil_sample_counts{};
    sample_count                 storage_image_sample_counts{};
    std::uint32_t                max_sample_mask_words{};
    bool                         timestamp_compute_and_graphics{};
    float                        timestamp_period{};
    std::uint32_t                max_clip_distances{};
    std::uint32_t                max_cull_distances{};
    std::uint32_t                max_combined_clip_and_cull_distances{};
    std::array<float, 2>         point_size_range{};
    std::array<float, 2>         line_width_range{};
    float                        point_size_granularity{};
    float                        line_width_granularity{};
    bool                         strict_lines{};
    bool                         standard_sample_locations{};
    std::uint64_t                optimal_buffer_copy_offset_alignment{};
    std::uint64_t                optimal_buffer_copy_row_pitch_alignment{};
};

struct physical_device_memory_properties
{
    std::uint64_t device_shared{};
    std::uint64_t device_local{};
    std::uint64_t host_shared{};
};

struct physical_device_surface_capabilities
{
    std::uint32_t min_image_count{};
    std::uint32_t max_image_count{};
    std::vector<present_mode> present_modes{};
};

struct physical_device_format_properties
{
    format_feature linear{};
    format_feature optimal{};
    format_feature buffer{};
};

enum class driver_id : std::uint32_t
{
    unknown = 0,
    amd_proprietary = VK_DRIVER_ID_AMD_PROPRIETARY,
    amd_open_source = VK_DRIVER_ID_AMD_OPEN_SOURCE,
    mesa_radv = VK_DRIVER_ID_MESA_RADV,
    nvidia_proprietary = VK_DRIVER_ID_NVIDIA_PROPRIETARY,
    intel_proprietary_windows = VK_DRIVER_ID_INTEL_PROPRIETARY_WINDOWS,
    intel_open_source_mesa = VK_DRIVER_ID_INTEL_OPEN_SOURCE_MESA,
    imagination_proprietary = VK_DRIVER_ID_IMAGINATION_PROPRIETARY,
    qualcomm_proprietary = VK_DRIVER_ID_QUALCOMM_PROPRIETARY,
    arm_proprietary = VK_DRIVER_ID_ARM_PROPRIETARY,
    google_swift_shader = VK_DRIVER_ID_GOOGLE_SWIFTSHADER,
    ggp_proprietary = VK_DRIVER_ID_GGP_PROPRIETARY,
    broadcom_proprietary = VK_DRIVER_ID_BROADCOM_PROPRIETARY,
    mesa_llvmpipe = VK_DRIVER_ID_MESA_LLVMPIPE,
    moltenvk = VK_DRIVER_ID_MOLTENVK,
};

struct physical_device_driver
{
    driver_id id{};
    std::string name{};
    std::string info{};
};

class physical_device;

namespace vulkan
{

physical_device make_physical_device(const vulkan::instance_context& context, VkPhysicalDevice phydev, tph::version instance_version) noexcept;

}

class TEPHRA_API physical_device
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

    friend physical_device vulkan::make_physical_device(const vulkan::instance_context& context, VkPhysicalDevice phydev, tph::version instance_version) noexcept;

public:
    constexpr physical_device() = default;
    ~physical_device() = default;
    physical_device(const physical_device&) = delete;
    physical_device& operator=(const physical_device&) = delete;
    physical_device(physical_device&&) noexcept = default;
    physical_device& operator=(physical_device&&) noexcept = default;

    bool support_presentation(const surface& surf) const;
    physical_device_surface_capabilities surface_capabilities(const surface& surf) const;
    physical_device_format_properties format_properties(texture_format format) const noexcept;
    bool support_texture_format(texture_format format, format_feature features) const;

    const physical_device_properties& properties() const noexcept
    {
        return m_properties;
    }

    const physical_device_features& features() const noexcept
    {
        return m_features;
    }

    const physical_device_limits& limits() const noexcept
    {
        return m_limits;
    }

    const physical_device_memory_properties memory_properties() const noexcept
    {
        return m_memory_properties;
    }

    const std::optional<physical_device_driver>& driver() const noexcept
    {
        return m_driver;
    }

private:
    vulkan::instance_context m_context{};
    VkPhysicalDevice m_physical_device{};
    physical_device_properties m_properties{};
    physical_device_features m_features{};
    physical_device_limits m_limits{};
    physical_device_memory_properties m_memory_properties{};
    std::optional<physical_device_driver> m_driver{};
};

template<>
inline VkPhysicalDevice underlying_cast(const physical_device& phydev) noexcept
{
    return phydev.m_physical_device;
}

TEPHRA_API bool default_physical_device_comparator(const physical_device& left, const physical_device& right) noexcept;

}

#endif
