#include "hardware.hpp"

#include <algorithm>
#include <unordered_map>

#include "vulkan/vulkan_functions.hpp"

#include "application.hpp"
#include "renderer.hpp"
#include "surface.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

namespace vulkan
{

static physical_device_properties make_properties(const VkPhysicalDeviceProperties& properties) noexcept
{
    physical_device_properties output{};
    output.name = properties.deviceName;
    output.api_version.major = static_cast<std::uint16_t>(VK_VERSION_MAJOR(properties.apiVersion));
    output.api_version.minor = static_cast<std::uint16_t>(VK_VERSION_MINOR(properties.apiVersion));
    output.api_version.patch = VK_VERSION_PATCH(properties.apiVersion);
    output.type = static_cast<physical_device_type>(properties.deviceType);
    std::copy(std::cbegin(properties.pipelineCacheUUID), std::cend(properties.pipelineCacheUUID), std::begin(output.uuid));

    return output;
}

static physical_device_features make_features(const VkPhysicalDeviceFeatures& features) noexcept
{
    physical_device_features output{};

    output.robust_buffer_access                         = static_cast<bool>(features.robustBufferAccess);
    output.full_draw_index_uint32                       = static_cast<bool>(features.fullDrawIndexUint32);
    output.image_cube_array                             = static_cast<bool>(features.imageCubeArray);
    output.independent_blend                            = static_cast<bool>(features.independentBlend);
    output.geometry_shader                              = static_cast<bool>(features.geometryShader);
    output.tessellation_shader                          = static_cast<bool>(features.tessellationShader);
    output.sample_shading                               = static_cast<bool>(features.sampleRateShading);
    output.dual_source_blend                            = static_cast<bool>(features.dualSrcBlend);
    output.logic_op                                     = static_cast<bool>(features.logicOp);
    output.multi_draw_indirect                          = static_cast<bool>(features.multiDrawIndirect);
    output.draw_indirect_first_instance                 = static_cast<bool>(features.drawIndirectFirstInstance);
    output.depth_clamp                                  = static_cast<bool>(features.depthClamp);
    output.depth_bias_clamp                             = static_cast<bool>(features.depthBiasClamp);
    output.fill_mode_non_solid                          = static_cast<bool>(features.fillModeNonSolid);
    output.depth_bounds                                 = static_cast<bool>(features.depthBounds);
    output.wide_lines                                   = static_cast<bool>(features.wideLines);
    output.large_points                                 = static_cast<bool>(features.largePoints);
    output.alpha_to_one                                 = static_cast<bool>(features.alphaToOne);
    output.multi_viewport                               = static_cast<bool>(features.multiViewport);
    output.sampler_anisotropy                           = static_cast<bool>(features.samplerAnisotropy);
    output.occlusion_query_precise                      = static_cast<bool>(features.occlusionQueryPrecise);
    output.pipeline_statistics_query                    = static_cast<bool>(features.pipelineStatisticsQuery);
    output.vertex_pipeline_stores_and_atomics           = static_cast<bool>(features.vertexPipelineStoresAndAtomics);
    output.fragment_stores_and_atomics                  = static_cast<bool>(features.fragmentStoresAndAtomics);
    output.shader_tessellation_and_geometry_point_size  = static_cast<bool>(features.shaderTessellationAndGeometryPointSize);
    output.shader_image_gather_extended                 = static_cast<bool>(features.shaderImageGatherExtended);
    output.shader_storage_image_extended_formats        = static_cast<bool>(features.shaderStorageImageExtendedFormats);
    output.shader_storage_image_multisample             = static_cast<bool>(features.shaderStorageImageMultisample);
    output.shader_storage_image_read_without_format     = static_cast<bool>(features.shaderStorageImageReadWithoutFormat);
    output.shader_storage_image_write_without_format    = static_cast<bool>(features.shaderStorageImageWriteWithoutFormat);
    output.shader_uniform_buffer_array_dynamic_indexing = static_cast<bool>(features.shaderUniformBufferArrayDynamicIndexing);
    output.shader_sampled_image_array_dynamic_indexing  = static_cast<bool>(features.shaderSampledImageArrayDynamicIndexing);
    output.shader_storage_buffer_array_dynamic_indexing = static_cast<bool>(features.shaderStorageBufferArrayDynamicIndexing);
    output.shader_storage_image_array_dynamic_indexing  = static_cast<bool>(features.shaderStorageImageArrayDynamicIndexing);
    output.shader_clip_distance                         = static_cast<bool>(features.shaderClipDistance);
    output.shader_cull_distance                         = static_cast<bool>(features.shaderCullDistance);
    output.shader_float64                               = static_cast<bool>(features.shaderFloat64);
    output.shader_int64                                 = static_cast<bool>(features.shaderInt64);
    output.shader_int16                                 = static_cast<bool>(features.shaderInt16);
    output.shader_resource_residency                    = static_cast<bool>(features.shaderResourceResidency);
    output.shader_resource_min_lod                      = static_cast<bool>(features.shaderResourceMinLod);
    output.variable_multisample_rate                    = static_cast<bool>(features.variableMultisampleRate);
    output.inherited_queries                            = static_cast<bool>(features.inheritedQueries);

    return output;
}

static physical_device_limits make_limits(const VkPhysicalDeviceLimits& limits) noexcept
{
    physical_device_limits output{};

    output.max_1d_texture_size                                   = limits.maxImageDimension1D;
    output.max_2d_texture_size                                   = limits.maxImageDimension2D;
    output.max_3d_texture_size                                   = limits.maxImageDimension3D;
    output.max_cube_texture_size                                 = limits.maxImageDimensionCube;
    output.max_texture_array_layers                              = limits.maxImageArrayLayers;
    output.max_texel_buffer_elements                             = limits.maxTexelBufferElements;
    output.max_uniform_buffer_range                              = limits.maxUniformBufferRange;
    output.max_storage_buffer_range                              = limits.maxStorageBufferRange;
    output.max_push_constants_size                               = limits.maxPushConstantsSize;
    output.max_memory_allocation_count                           = limits.maxMemoryAllocationCount;
    output.max_sampler_allocation_count                          = limits.maxSamplerAllocationCount;
    output.max_bound_descriptor_sets                             = limits.maxBoundDescriptorSets;
    output.max_per_stage_descriptor_samplers                     = limits.maxPerStageDescriptorSamplers;
    output.max_per_stage_descriptor_uniform_buffers              = limits.maxPerStageDescriptorUniformBuffers;
    output.max_per_stage_descriptor_storage_buffers              = limits.maxPerStageDescriptorStorageBuffers;
    output.max_per_stage_descriptor_sampled_images               = limits.maxPerStageDescriptorSampledImages;
    output.max_per_stage_descriptor_storage_images               = limits.maxPerStageDescriptorStorageImages;
    output.max_per_stage_descriptor_input_attachments            = limits.maxPerStageDescriptorInputAttachments;
    output.max_per_stage_resources                               = limits.maxPerStageResources;
    output.max_descriptor_set_samplers                           = limits.maxDescriptorSetSamplers;
    output.max_descriptor_set_uniform_buffers                    = limits.maxDescriptorSetUniformBuffers;
    output.max_descriptor_set_uniform_buffers_dynamic            = limits.maxDescriptorSetUniformBuffersDynamic;
    output.max_descriptor_set_storage_buffers                    = limits.maxDescriptorSetStorageBuffers;
    output.max_descriptor_set_storage_buffers_dynamic            = limits.maxDescriptorSetStorageBuffersDynamic;
    output.max_descriptor_set_sampled_images                     = limits.maxDescriptorSetSampledImages;
    output.max_descriptor_set_storage_images                     = limits.maxDescriptorSetStorageImages;
    output.max_descriptor_set_input_attachments                  = limits.maxDescriptorSetInputAttachments;
    output.max_vertex_input_attributes                           = limits.maxVertexInputAttributes;
    output.max_vertex_input_bindings                             = limits.maxVertexInputBindings;
    output.max_vertex_input_attribute_offset                     = limits.maxVertexInputAttributeOffset;
    output.max_vertex_input_binding_stride                       = limits.maxVertexInputBindingStride;
    output.max_vertex_output_components                          = limits.maxVertexOutputComponents;
    output.max_tessellation_generation_level                     = limits.maxTessellationGenerationLevel;
    output.max_tessellation_patch_size                           = limits.maxTessellationPatchSize;
    output.max_tessellation_control_per_vertex_input_components  = limits.maxTessellationControlPerVertexInputComponents;
    output.max_tessellation_control_per_vertex_output_components = limits.maxTessellationControlPerVertexOutputComponents;
    output.max_tessellation_control_per_patch_output_components  = limits.maxTessellationControlPerPatchOutputComponents;
    output.max_tessellation_control_total_output_components      = limits.maxTessellationControlTotalOutputComponents;
    output.max_tessellation_evaluation_input_components          = limits.maxTessellationEvaluationInputComponents;
    output.max_tessellation_evaluation_output_components         = limits.maxTessellationEvaluationOutputComponents;
    output.max_geometry_shader_invocations                       = limits.maxGeometryShaderInvocations;
    output.max_geometry_input_components                         = limits.maxGeometryInputComponents;
    output.max_geometry_output_components                        = limits.maxGeometryOutputComponents;
    output.max_geometry_output_vertices                          = limits.maxGeometryOutputVertices;
    output.max_geometry_total_output_components                  = limits.maxGeometryTotalOutputComponents;
    output.max_fragment_input_components                         = limits.maxFragmentInputComponents;
    output.max_fragment_output_attachments                       = limits.maxFragmentOutputAttachments;
    output.max_fragment_dual_source_attachments                  = limits.maxFragmentDualSrcAttachments;
    output.max_fragment_combined_output_resources                = limits.maxFragmentCombinedOutputResources;
    output.max_compute_shared_memory_size                        = limits.maxComputeSharedMemorySize;
    output.max_compute_work_group_count                          = std::to_array(limits.maxComputeWorkGroupCount);
    output.max_compute_work_group_invocations                    = limits.maxComputeWorkGroupInvocations;
    output.max_compute_work_group_size                           = std::to_array(limits.maxComputeWorkGroupSize);
    output.sub_pixel_precision_bits                              = limits.subPixelPrecisionBits;
    output.sub_texel_precision_bits                              = limits.subTexelPrecisionBits;
    output.mipmap_precision_bits                                 = limits.mipmapPrecisionBits;
    output.max_draw_indexed_index_value                          = limits.maxDrawIndexedIndexValue;
    output.max_draw_indirect_count                               = limits.maxDrawIndirectCount;
    output.max_sampler_lod_bias                                  = limits.maxSamplerLodBias;
    output.max_sampler_anisotropy                                = limits.maxSamplerAnisotropy;
    output.max_viewports                                         = limits.maxViewports;
    output.max_viewport_dimensions                               = std::to_array(limits.maxViewportDimensions);
    output.viewport_bounds_range                                 = std::to_array(limits.viewportBoundsRange);
    output.viewport_sub_pixel_bits                               = limits.viewportSubPixelBits;
    output.min_texel_buffer_offset_alignment                     = limits.minTexelBufferOffsetAlignment;
    output.min_uniform_buffer_alignment                          = limits.minUniformBufferOffsetAlignment;
    output.min_storage_buffer_alignment                          = limits.minStorageBufferOffsetAlignment;
    output.max_framebuffer_width                                 = limits.maxFramebufferWidth;
    output.max_framebuffer_height                                = limits.maxFramebufferHeight;
    output.max_framebuffer_layers                                = limits.maxFramebufferLayers;
    output.framebuffer_color_sample_counts                       = static_cast<sample_count>(limits.framebufferColorSampleCounts);
    output.framebuffer_depth_sample_counts                       = static_cast<sample_count>(limits.framebufferDepthSampleCounts);
    output.framebuffer_stencil_sample_counts                     = static_cast<sample_count>(limits.framebufferStencilSampleCounts);
    output.framebuffer_no_attachments_sample_counts              = static_cast<sample_count>(limits.framebufferNoAttachmentsSampleCounts);
    output.max_color_attachments                                 = limits.maxColorAttachments;
    output.sampled_image_color_sample_counts                     = static_cast<sample_count>(limits.sampledImageColorSampleCounts);
    output.sampled_image_integer_sample_counts                   = static_cast<sample_count>(limits.sampledImageIntegerSampleCounts);
    output.sampled_image_depth_sample_counts                     = static_cast<sample_count>(limits.sampledImageDepthSampleCounts);
    output.sampled_image_stencil_sample_counts                   = static_cast<sample_count>(limits.sampledImageStencilSampleCounts);
    output.storage_image_sample_counts                           = static_cast<sample_count>(limits.storageImageSampleCounts);
    output.max_sample_mask_words                                 = limits.maxSampleMaskWords;
    output.timestamp_compute_and_graphics                        = limits.timestampComputeAndGraphics;
    output.timestamp_period                                      = limits.timestampPeriod;
    output.max_clip_distances                                    = limits.maxClipDistances;
    output.max_cull_distances                                    = limits.maxCullDistances;
    output.max_combined_clip_and_cull_distances                  = limits.maxCombinedClipAndCullDistances;
    output.point_size_range                                      = std::to_array(limits.pointSizeRange);
    output.line_width_range                                      = std::to_array(limits.lineWidthRange);
    output.point_size_granularity                                = limits.pointSizeGranularity;
    output.line_width_granularity                                = limits.lineWidthGranularity;
    output.strict_lines                                          = limits.strictLines;
    output.standard_sample_locations                             = limits.standardSampleLocations;
    output.optimal_buffer_copy_offset_alignment                  = limits.optimalBufferCopyOffsetAlignment;
    output.optimal_buffer_copy_row_pitch_alignment               = limits.optimalBufferCopyRowPitchAlignment;

    return output;
}

static physical_device_memory_properties make_memory_properties(VkPhysicalDevice physical_device) noexcept
{
    VkPhysicalDeviceMemoryProperties properties{};
    vkGetPhysicalDeviceMemoryProperties(physical_device, &properties);

    std::vector<VkMemoryPropertyFlags> heaps_flags{};
    heaps_flags.resize(properties.memoryHeapCount);

    for(std::uint32_t i{}; i < properties.memoryTypeCount; ++i)
    {
        heaps_flags[properties.memoryTypes[i].heapIndex] |= properties.memoryTypes[i].propertyFlags;
    }

    physical_device_memory_properties output{};

    for(std::size_t i{}; i < std::size(heaps_flags); ++i)
    {
        if((heaps_flags[i] & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        {
            output.device_shared += properties.memoryHeaps[i].size;
        }
        else if(heaps_flags[i] & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            output.device_local += properties.memoryHeaps[i].size;
        }
        else if(heaps_flags[i] & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            output.host_shared += properties.memoryHeaps[i].size;
        }
    }

    return output;
}

static physical_device_driver make_driver(const VkPhysicalDeviceDriverProperties& driver)
{
    physical_device_driver output{};

    output.id = static_cast<driver_id>(driver.driverID);
    output.name = driver.driverName;
    output.info = driver.driverInfo;

    return output;
}

physical_device make_physical_device(VkPhysicalDevice device, tph::version instance_version) noexcept
{
    if(instance_version >= tph::version{1, 2})
    {
        VkPhysicalDeviceDriverProperties driver{};
        driver.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;

        VkPhysicalDeviceProperties2 properties{};
        properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        properties.pNext = &driver;

        vkGetPhysicalDeviceProperties2(device, &properties);

        VkPhysicalDeviceFeatures features{};
        vkGetPhysicalDeviceFeatures(device, &features);

        physical_device output{};

        output.m_physical_device = device;
        output.m_properties = make_properties(properties.properties);
        output.m_features = make_features(features);
        output.m_limits = make_limits(properties.properties.limits);
        output.m_memory_properties = make_memory_properties(device);
        output.m_driver = make_driver(driver);

        return output;
    }
    else
    {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device, &properties);

        VkPhysicalDeviceFeatures features{};
        vkGetPhysicalDeviceFeatures(device, &features);

        physical_device output{};

        output.m_physical_device = device;
        output.m_properties = make_properties(properties);
        output.m_features = make_features(features);
        output.m_limits = make_limits(properties.limits);
        output.m_memory_properties = make_memory_properties(device);

        return output;
    }
}

}

bool physical_device::support_presentation(const surface& surface) const
{
    std::uint32_t count{};
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &count, nullptr);

    for(std::uint32_t i{}; i < count; ++i)
    {
        VkBool32 support{};
        if(auto result{vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_device, i, underlying_cast<VkSurfaceKHR>(surface), &support)}; result != VK_SUCCESS)
            throw vulkan::error{result};

        if(support)
        {
            return true;
        }
    }

    return false;
}

physical_device_surface_capabilities physical_device::surface_capabilities(const surface& surface) const
{
    std::uint32_t count{};
    if(auto result{vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, underlying_cast<VkSurfaceKHR>(surface), &count, nullptr)}; result != VK_SUCCESS)
        throw vulkan::error{result};

    std::vector<VkPresentModeKHR> present_modes{};
    present_modes.resize(count);
    if(auto result{vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, underlying_cast<VkSurfaceKHR>(surface), &count, std::data(present_modes))}; result != VK_SUCCESS)
        throw vulkan::error{result};

    VkSurfaceCapabilitiesKHR capabilities{};
    if(auto result{vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, underlying_cast<VkSurfaceKHR>(surface), &capabilities)}; result != VK_SUCCESS)
        throw vulkan::error{result};

    physical_device_surface_capabilities output{};

    output.min_image_count = capabilities.minImageCount;
    output.max_image_count = capabilities.maxImageCount == 0 ? std::numeric_limits<std::uint32_t>::max() : capabilities.maxImageCount;

    output.present_modes.reserve(std::size(present_modes));
    for(auto mode : present_modes)
    {
        output.present_modes.emplace_back(static_cast<present_mode>(mode));
    }

    return output;
}

physical_device_format_properties physical_device::format_properties(texture_format format) const noexcept
{
    VkFormatProperties properties{};
    vkGetPhysicalDeviceFormatProperties(m_physical_device, static_cast<VkFormat>(format), &properties);

    physical_device_format_properties output{};
    output.linear = static_cast<format_feature>(properties.linearTilingFeatures);
    output.optimal = static_cast<format_feature>(properties.optimalTilingFeatures);
    output.buffer = static_cast<format_feature>(properties.bufferFeatures);

    return output;
}

bool physical_device::support_texture_format(texture_format format, format_feature features) const
{
    VkFormatProperties properties{};
    vkGetPhysicalDeviceFormatProperties(m_physical_device, static_cast<VkFormat>(format), &properties);

    return (static_cast<format_feature>(properties.optimalTilingFeatures) & features) == features;
}

bool default_physical_device_comparator(const physical_device& left, const physical_device& right) noexcept
{
    const auto physical_device_score = [](const physical_device& device) noexcept -> std::int64_t
    {
        std::int64_t score{};

        score += device.memory_properties().device_shared / 1048576;
        score += device.memory_properties().device_local / 1048576;
        score += device.memory_properties().host_shared / 1048576;

        if(device.properties().type == physical_device_type::discrete)
        {
            score *= 2;
        }

        return score;
    };

    return physical_device_score(left) > physical_device_score(right);
}

}
