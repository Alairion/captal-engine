#include "texture.hpp"

#include "vulkan/vulkan_functions.hpp"

#include "renderer.hpp"
#include "commands.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

static VkImageType image_type(std::uint32_t height, std::uint32_t depth)
{
    if(height == 1 && depth == 1)
        return VK_IMAGE_TYPE_1D;

    if(depth == 1)
        return VK_IMAGE_TYPE_2D;

    return VK_IMAGE_TYPE_3D;
}

static VkImageViewType image_view_type(std::uint32_t height, std::uint32_t depth)
{
    if(height == 1 && depth == 1)
        return VK_IMAGE_VIEW_TYPE_1D;

    if(depth == 1)
        return VK_IMAGE_VIEW_TYPE_2D;

    return VK_IMAGE_VIEW_TYPE_3D;
}

texture::texture(renderer& renderer, size_type width, size_type height, texture_usage usage)
:texture{renderer, width, height, 1, usage}
{

}

texture::texture(renderer& renderer, size_type width, size_type height, const sampling_options& options, texture_usage usage)
:texture{renderer, width, height, 1, options, usage}
{

}

texture::texture(renderer& renderer, size_type width, size_type height, size_type depth, texture_usage usage)
:m_width{width}
,m_height{height}
,m_depth{depth}
{
    m_image = vulkan::image{underlying_cast<VkDevice>(renderer), VkExtent3D{width, height, depth}, image_type(height, depth), VK_FORMAT_R8G8B8A8_UNORM, static_cast<VkImageUsageFlags>(usage)};
    m_memory = renderer.allocator().allocate_bound(m_image, vulkan::memory_resource_type::non_linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if(static_cast<bool>(usage & texture_usage::sampled) || static_cast<bool>(usage & texture_usage::color_attachment))
    {
        m_image_view = vulkan::image_view{underlying_cast<VkDevice>(renderer), m_image, image_view_type(height, depth),  VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT};
    }
}

texture::texture(renderer& renderer, size_type width, size_type height, size_type depth, const sampling_options& options, texture_usage usage)
:m_width{width}
,m_height{height}
,m_depth{depth}
{
    m_image = vulkan::image{underlying_cast<VkDevice>(renderer), VkExtent3D{width, height, depth}, image_type(height, depth), VK_FORMAT_R8G8B8A8_UNORM, static_cast<VkImageUsageFlags>(usage)};
    m_memory = renderer.allocator().allocate_bound(m_image, vulkan::memory_resource_type::non_linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if(static_cast<bool>(usage & texture_usage::sampled) || static_cast<bool>(usage & texture_usage::color_attachment))
    {
        m_image_view = vulkan::image_view{underlying_cast<VkDevice>(renderer), m_image, image_view_type(height, depth), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT};
    }

    if(static_cast<bool>(usage & texture_usage::sampled))
    {
        m_sampler = vulkan::sampler
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
}

void texture::transition(command_buffer& command_buffer, resource_access source_access, resource_access destination_access, pipeline_stage source_stage, pipeline_stage destination_stage, image_layout layout)
{
    if(m_layout != layout)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = static_cast<VkImageLayout>(m_layout);
        barrier.newLayout = static_cast<VkImageLayout>(layout);
        barrier.srcAccessMask = static_cast<VkAccessFlags>(source_access);
        barrier.dstAccessMask = static_cast<VkAccessFlags>(destination_access);
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(underlying_cast<VkCommandBuffer>(command_buffer),
                             static_cast<VkPipelineStageFlags>(source_stage), static_cast<VkPipelineStageFlags>(destination_stage),
                             0, 0, nullptr, 0, nullptr, 1, &barrier);

        m_layout = layout;
    }
}

void texture::transition(command_buffer& command_buffer, resource_access source_access, resource_access destination_access, pipeline_stage source_stage, pipeline_stage destination_stage, image_layout current_layout, image_layout next_layout)
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
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(underlying_cast<VkCommandBuffer>(command_buffer),
                         static_cast<VkPipelineStageFlags>(source_stage), static_cast<VkPipelineStageFlags>(destination_stage),
                         0, 0, nullptr, 0, nullptr, 1, &barrier);

    m_layout = next_layout;
}

}
