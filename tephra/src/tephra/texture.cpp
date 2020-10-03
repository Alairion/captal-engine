#include "texture.hpp"

#include "vulkan/vulkan_functions.hpp"
#include "vulkan/helper.hpp"

#include "renderer.hpp"
#include "commands.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

static VkComponentMapping make_mapping(const component_mapping& mapping)
{
    return VkComponentMapping
    {
        static_cast<VkComponentSwizzle>(mapping.r),
        static_cast<VkComponentSwizzle>(mapping.g),
        static_cast<VkComponentSwizzle>(mapping.b),
        static_cast<VkComponentSwizzle>(mapping.a)
    };
}

static vulkan::sampler make_sampler(renderer& renderer, const sampling_options& options)
{
    return vulkan::sampler
    {
        underlying_cast<VkDevice>(renderer),
        static_cast<VkFilter>(options.magnification_filter),
        static_cast<VkFilter>(options.minification_filter),
        static_cast<VkSamplerAddressMode>(options.address_mode),
        static_cast<VkBool32>(options.compare),
        static_cast<VkCompareOp>(options.compare_op),
        static_cast<VkBool32>(!options.normalized_coordinates),
        static_cast<float>(options.anisotropy_level)
    };
}

static bool need_image_view(texture_usage usage) noexcept
{
    return static_cast<bool>(usage & texture_usage::sampled)
        || static_cast<bool>(usage & texture_usage::color_attachment)
        || static_cast<bool>(usage & texture_usage::depth_stencil_attachment)
        || static_cast<bool>(usage & texture_usage::input_attachment);
}

texture_aspect aspect_from_format(texture_format format) noexcept
{
    switch(format)
    {
        case texture_format::d16_unorm:          [[fallthrough]];
        case texture_format::x8_d24_unorm_pack:  [[fallthrough]];
        case texture_format::d32_sfloat:         return texture_aspect::depth;
        case texture_format::s8_uint:            return texture_aspect::stencil;
        case texture_format::d16_unorm_s8_uint:  [[fallthrough]];
        case texture_format::d24_unorm_s8_uint:  [[fallthrough]];
        case texture_format::d32_sfloat_s8_uint: return texture_aspect::depth | texture_aspect::stencil;
        default:                                 return texture_aspect::color;
    }
}

texture::texture(renderer& renderer, std::uint32_t width, const texture_info& info)
:m_format{info.format}
,m_aspect{aspect_from_format(m_format)}
,m_width{width}
,m_height{1}
,m_depth{1}
{
    m_image = vulkan::image
    {
        underlying_cast<VkDevice>(renderer),
        VkExtent3D{width, 1, 1},
        VK_IMAGE_TYPE_1D,
        static_cast<VkFormat>(m_format),
        static_cast<VkImageUsageFlags>(info.usage),
        VK_IMAGE_TILING_OPTIMAL,
        static_cast<VkSampleCountFlagBits>(info.sample_count)
    };

    m_memory = renderer.allocator().allocate_bound(m_image, vulkan::memory_resource_type::non_linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if(need_image_view(info.usage))
    {
        m_image_view = vulkan::image_view
        {
            underlying_cast<VkDevice>(renderer),
            m_image,
            VK_IMAGE_VIEW_TYPE_1D,
            static_cast<VkFormat>(m_format),
            make_mapping(info.components),
            static_cast<VkImageAspectFlags>(m_aspect)
        };
    }
}

texture::texture(renderer& renderer, std::uint32_t width, const texture_info& info, const sampling_options& options)
:m_format{info.format}
,m_aspect{aspect_from_format(m_format)}
,m_width{width}
,m_height{1}
,m_depth{1}
{
    m_image = vulkan::image
    {
        underlying_cast<VkDevice>(renderer),
        VkExtent3D{width, 1, 1},
        VK_IMAGE_TYPE_1D,
        static_cast<VkFormat>(m_format),
        static_cast<VkImageUsageFlags>(info.usage),
        VK_IMAGE_TILING_OPTIMAL,
        static_cast<VkSampleCountFlagBits>(info.sample_count)
    };

    m_memory = renderer.allocator().allocate_bound(m_image, vulkan::memory_resource_type::non_linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if(need_image_view(info.usage))
    {
        m_image_view = vulkan::image_view
        {
            underlying_cast<VkDevice>(renderer),
            m_image,
            VK_IMAGE_VIEW_TYPE_1D,
            static_cast<VkFormat>(m_format),
            make_mapping(info.components),
            static_cast<VkImageAspectFlags>(m_aspect)
        };
    }

    if(static_cast<bool>(info.usage & texture_usage::sampled))
    {
        m_sampler = make_sampler(renderer, options);
    }
}

texture::texture(renderer& renderer, std::uint32_t width, std::uint32_t height, const texture_info& info)
:m_format{info.format}
,m_aspect{aspect_from_format(m_format)}
,m_width{width}
,m_height{height}
,m_depth{1}
{
    m_image = vulkan::image
    {
        underlying_cast<VkDevice>(renderer),
        VkExtent3D{width, height, 1},
        VK_IMAGE_TYPE_2D,
        static_cast<VkFormat>(m_format),
        static_cast<VkImageUsageFlags>(info.usage),
        VK_IMAGE_TILING_OPTIMAL,
        static_cast<VkSampleCountFlagBits>(info.sample_count)
    };

    m_memory = renderer.allocator().allocate_bound(m_image, vulkan::memory_resource_type::non_linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if(need_image_view(info.usage))
    {
        m_image_view = vulkan::image_view
        {
            underlying_cast<VkDevice>(renderer),
            m_image,
            VK_IMAGE_VIEW_TYPE_2D,
            static_cast<VkFormat>(m_format),
            make_mapping(info.components),
            static_cast<VkImageAspectFlags>(m_aspect)
        };
    }
}

texture::texture(renderer& renderer, std::uint32_t width, std::uint32_t height, const texture_info& info, const sampling_options& options)
:m_format{info.format}
,m_aspect{aspect_from_format(m_format)}
,m_width{width}
,m_height{height}
,m_depth{1}
{
    m_image = vulkan::image
    {
        underlying_cast<VkDevice>(renderer),
        VkExtent3D{width, height, 1},
        VK_IMAGE_TYPE_2D,
        static_cast<VkFormat>(m_format),
        static_cast<VkImageUsageFlags>(info.usage),
        VK_IMAGE_TILING_OPTIMAL,
        static_cast<VkSampleCountFlagBits>(info.sample_count)
    };

    m_memory = renderer.allocator().allocate_bound(m_image, vulkan::memory_resource_type::non_linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if(need_image_view(info.usage))
    {
        m_image_view = vulkan::image_view
        {
            underlying_cast<VkDevice>(renderer),
            m_image,
            VK_IMAGE_VIEW_TYPE_2D,
            static_cast<VkFormat>(m_format),
            make_mapping(info.components),
            static_cast<VkImageAspectFlags>(m_aspect)
        };
    }

    if(static_cast<bool>(info.usage & texture_usage::sampled))
    {
        m_sampler = make_sampler(renderer, options);
    }
}

texture::texture(renderer& renderer, std::uint32_t width, std::uint32_t height, std::uint32_t depth, const texture_info& info)
:m_format{info.format}
,m_aspect{aspect_from_format(m_format)}
,m_width{width}
,m_height{height}
,m_depth{depth}
{
    m_image = vulkan::image
    {
        underlying_cast<VkDevice>(renderer),
        VkExtent3D{width, height, depth},
        VK_IMAGE_TYPE_3D,
        static_cast<VkFormat>(m_format),
        static_cast<VkImageUsageFlags>(info.usage),
        VK_IMAGE_TILING_OPTIMAL,
        static_cast<VkSampleCountFlagBits>(info.sample_count)
    };

    m_memory = renderer.allocator().allocate_bound(m_image, vulkan::memory_resource_type::non_linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if(need_image_view(info.usage))
    {
        m_image_view = vulkan::image_view
        {
            underlying_cast<VkDevice>(renderer),
            m_image,
            VK_IMAGE_VIEW_TYPE_3D,
            static_cast<VkFormat>(m_format),
            make_mapping(info.components),
            static_cast<VkImageAspectFlags>(m_aspect)
        };
    }
}

texture::texture(renderer& renderer, std::uint32_t width, std::uint32_t height, std::uint32_t depth, const texture_info& info, const sampling_options& options)
:m_format{info.format}
,m_aspect{aspect_from_format(m_format)}
,m_width{width}
,m_height{height}
,m_depth{depth}
{
    m_image = vulkan::image
    {
        underlying_cast<VkDevice>(renderer),
        VkExtent3D{width, height, depth},
        VK_IMAGE_TYPE_3D,
        static_cast<VkFormat>(m_format),
        static_cast<VkImageUsageFlags>(info.usage),
        VK_IMAGE_TILING_OPTIMAL,
        static_cast<VkSampleCountFlagBits>(info.sample_count)
    };

    m_memory = renderer.allocator().allocate_bound(m_image, vulkan::memory_resource_type::non_linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if(need_image_view(info.usage))
    {
        m_image_view = vulkan::image_view
        {
            underlying_cast<VkDevice>(renderer),
            m_image,
            VK_IMAGE_VIEW_TYPE_3D,
            static_cast<VkFormat>(m_format),
            make_mapping(info.components),
            static_cast<VkImageAspectFlags>(m_aspect)
        };
    }

    if(static_cast<bool>(info.usage & texture_usage::sampled))
    {
        m_sampler = make_sampler(renderer, options);
    }
}

void texture::transition(command_buffer& command_buffer, resource_access source_access, resource_access destination_access, pipeline_stage source_stage, pipeline_stage destination_stage, texture_layout current_layout, texture_layout next_layout)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = static_cast<VkImageLayout>(current_layout);
    barrier.newLayout = static_cast<VkImageLayout>(next_layout);
    barrier.srcAccessMask = static_cast<VkAccessFlags>(source_access);
    barrier.dstAccessMask = static_cast<VkAccessFlags>(destination_access);
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = static_cast<VkImageAspectFlags>(m_aspect);
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(underlying_cast<VkCommandBuffer>(command_buffer),
                         static_cast<VkPipelineStageFlags>(source_stage), static_cast<VkPipelineStageFlags>(destination_stage),
                         0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void set_object_name(renderer& renderer, const texture& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_IMAGE;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkImage>(object));
    info.pObjectName = std::data(name);

    if(auto result{vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(renderer), &info)}; result != VK_SUCCESS)
        throw vulkan::error{result};

    if(auto view{underlying_cast<VkImageView>(object)}; view)
    {
        const std::string real_name{name + " image view"};

        VkDebugUtilsObjectNameInfoEXT info{};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        info.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
        info.objectHandle = reinterpret_cast<std::uint64_t>(view);
        info.pObjectName = std::data(real_name);

        if(auto result{vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(renderer), &info)}; result != VK_SUCCESS)
            throw vulkan::error{result};
    }

    if(auto sampler{underlying_cast<VkSampler>(object)}; sampler)
    {
        const std::string real_name{name + " sampler"};

        VkDebugUtilsObjectNameInfoEXT info{};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        info.objectType = VK_OBJECT_TYPE_SAMPLER;
        info.objectHandle = reinterpret_cast<std::uint64_t>(sampler);
        info.pObjectName = std::data(real_name);

        if(auto result{vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(renderer), &info)}; result != VK_SUCCESS)
            throw vulkan::error{result};
    }
}

}
