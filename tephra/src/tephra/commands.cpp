#include "commands.hpp"

#include <captal_foundation/stack_allocator.hpp>

#include "vulkan/vulkan_functions.hpp"

#include "renderer.hpp"
#include "buffer.hpp"
#include "pipeline.hpp"
#include "render_target.hpp"
#include "image.hpp"
#include "texture.hpp"
#include "descriptor.hpp"
#include "synchronization.hpp"
#include "query.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

command_pool::command_pool(renderer& renderer, command_pool_options options)
:command_pool{renderer, queue::graphics, options}
{

}

command_pool::command_pool(renderer& renderer, queue queue, command_pool_options options)
:m_pool{underlying_cast<VkDevice>(renderer), renderer.queue_family_index(queue), static_cast<VkCommandPoolCreateFlags>(options)}
{

}

void command_pool::reset(command_pool_reset_options options)
{
    if(auto result{vkResetCommandPool(m_pool.device(), m_pool, static_cast<VkCommandPoolResetFlags>(options))}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

void command_pool::trim() noexcept
{
    vkTrimCommandPool(m_pool.device(), m_pool, 0);
}

void set_object_name(renderer& renderer, const command_pool& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_COMMAND_POOL;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkCommandPool>(object));
    info.pObjectName = std::data(name);

    if(auto result{vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(renderer), &info)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

void set_object_name(renderer& renderer, const command_buffer& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkCommandBuffer>(object));
    info.pObjectName = std::data(name);

    if(auto result{vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(renderer), &info)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

namespace cmd
{

command_buffer begin(command_pool& pool, command_buffer_level level, command_buffer_options options)
{
    vulkan::command_buffer buffer{underlying_cast<VkDevice>(pool), underlying_cast<VkCommandPool>(pool), static_cast<VkCommandBufferLevel>(level)};

    VkCommandBufferInheritanceInfo inheritance_info{};
    inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = static_cast<VkCommandBufferUsageFlags>(options);
    begin_info.pInheritanceInfo = &inheritance_info;

    if(auto result{vkBeginCommandBuffer(buffer, &begin_info)}; result != VK_SUCCESS)
        throw vulkan::error{result};

    return command_buffer{std::move(buffer)};
}

command_buffer begin(command_pool& pool, render_pass& render_pass, optional_ref<framebuffer> framebuffer, command_buffer_options options)
{
    vulkan::command_buffer buffer{underlying_cast<VkDevice>(pool), underlying_cast<VkCommandPool>(pool), VK_COMMAND_BUFFER_LEVEL_SECONDARY};

    VkCommandBufferInheritanceInfo inheritance_info{};
    inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritance_info.renderPass = underlying_cast<VkRenderPass>(render_pass);
    inheritance_info.framebuffer = framebuffer.has_value() ? underlying_cast<VkFramebuffer>(*framebuffer) : VkFramebuffer{};

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = static_cast<VkCommandBufferUsageFlags>(options) | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    begin_info.pInheritanceInfo = &inheritance_info;

    if(auto result{vkBeginCommandBuffer(buffer, &begin_info)}; result != VK_SUCCESS)
        throw vulkan::error{result};

    return command_buffer{std::move(buffer)};
}

void begin(command_buffer& buffer, command_buffer_reset_options reset, command_buffer_options options)
{
    if(auto result{vkResetCommandBuffer(underlying_cast<VkCommandBuffer>(buffer), static_cast<VkCommandBufferResetFlags>(reset))}; result != VK_SUCCESS)
        throw vulkan::error{result};

    VkCommandBufferInheritanceInfo inheritance_info{};
    inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = static_cast<VkCommandBufferUsageFlags>(options);
    begin_info.pInheritanceInfo = &inheritance_info;

    if(auto result{vkBeginCommandBuffer(underlying_cast<VkCommandBuffer>(buffer), &begin_info)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

void begin(command_buffer& buffer, render_pass& render_pass, optional_ref<framebuffer> framebuffer, command_buffer_reset_options reset, command_buffer_options options)
{
    if(auto result{vkResetCommandBuffer(underlying_cast<VkCommandBuffer>(buffer), static_cast<VkCommandBufferResetFlags>(reset))}; result != VK_SUCCESS)
        throw vulkan::error{result};

    VkCommandBufferInheritanceInfo inheritance_info{};
    inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritance_info.renderPass = underlying_cast<VkRenderPass>(render_pass);
    inheritance_info.framebuffer = framebuffer.has_value() ? underlying_cast<VkFramebuffer>(*framebuffer) : VkFramebuffer{};

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = static_cast<VkCommandBufferUsageFlags>(options) | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    begin_info.pInheritanceInfo = &inheritance_info;

    if(auto result{vkBeginCommandBuffer(underlying_cast<VkCommandBuffer>(buffer), &begin_info)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

void copy(command_buffer& command_buffer, buffer& source, buffer& destination, const buffer_copy& region) noexcept
{
    VkBufferCopy native_region{};
    native_region.srcOffset = region.source_offset;
    native_region.dstOffset = region.destination_offset;
    native_region.size = region.size;

    vkCmdCopyBuffer(underlying_cast<VkCommandBuffer>(command_buffer),
                    underlying_cast<VkBuffer>(source),
                    underlying_cast<VkBuffer>(destination),
                    1, &native_region);
}

void copy(command_buffer& command_buffer, buffer& source, buffer& destination, std::span<const buffer_copy> regions)
{
    stack_memory_pool<1024 * 2> pool{};
    auto native_regions{make_stack_vector<VkBufferCopy>(pool)};
    native_regions.reserve(std::size(regions));

    for(auto&& region : regions)
    {
        VkBufferCopy& native_region{native_regions.emplace_back()};
        native_region.srcOffset = region.source_offset;
        native_region.dstOffset = region.destination_offset;
        native_region.size = region.size;
    }

    vkCmdCopyBuffer(underlying_cast<VkCommandBuffer>(command_buffer),
                    underlying_cast<VkBuffer>(source),
                    underlying_cast<VkBuffer>(destination),
                    static_cast<std::uint32_t>(std::size(native_regions)), std::data(native_regions));
}

void copy(command_buffer& command_buffer, buffer& source, image& destination, const buffer_image_copy& region) noexcept
{
    assert((source.size() - region.buffer_offset) >= destination.byte_size() && "tph::cmd::copy called with too small buffer.");

    VkBufferCopy native_region{};
    native_region.srcOffset = region.buffer_offset;
    native_region.size = destination.byte_size();

    vkCmdCopyBuffer(underlying_cast<VkCommandBuffer>(command_buffer),
                    underlying_cast<VkBuffer>(source),
                    underlying_cast<VkBuffer>(destination),
                    1, &native_region);

}

void copy(command_buffer& command_buffer, buffer& source, texture& destination, const buffer_texture_copy& region) noexcept
{
    VkBufferImageCopy native_region{};
    native_region.bufferOffset = region.buffer_offset;
    native_region.bufferRowLength = region.buffer_image_width;
    native_region.bufferImageHeight = region.buffer_image_height;
    native_region.imageSubresource.aspectMask = static_cast<VkImageAspectFlags>(destination.aspect());
    native_region.imageSubresource.mipLevel = 0;
    native_region.imageSubresource.baseArrayLayer = 0;
    native_region.imageSubresource.layerCount = 1;
    native_region.imageOffset.x = region.texture_offset.x;
    native_region.imageOffset.y = region.texture_offset.y;
    native_region.imageOffset.z = region.texture_offset.z;
    native_region.imageExtent.width = region.texture_size.width;
    native_region.imageExtent.height = region.texture_size.height;
    native_region.imageExtent.depth = region.texture_size.depth;

    vkCmdCopyBufferToImage(underlying_cast<VkCommandBuffer>(command_buffer),
                           underlying_cast<VkBuffer>(source),
                           underlying_cast<VkImage>(destination), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &native_region);
}

void copy(command_buffer& command_buffer, buffer& source, texture& destination, std::span<const buffer_texture_copy> regions)
{
    stack_memory_pool<1024 * 2> pool{};
    auto native_regions{make_stack_vector<VkBufferImageCopy>(pool)};
    native_regions.reserve(std::size(regions));

    for(auto&& region : regions)
    {
        VkBufferImageCopy& native_region{native_regions.emplace_back()};
        native_region.bufferOffset = region.buffer_offset;
        native_region.bufferRowLength = region.buffer_image_width;
        native_region.bufferImageHeight = region.buffer_image_height;
        native_region.imageSubresource.aspectMask = static_cast<VkImageAspectFlags>(destination.aspect());
        native_region.imageSubresource.mipLevel = 0;
        native_region.imageSubresource.baseArrayLayer = 0;
        native_region.imageSubresource.layerCount = 1;
        native_region.imageOffset.x = region.texture_offset.x;
        native_region.imageOffset.y = region.texture_offset.y;
        native_region.imageOffset.z = region.texture_offset.z;
        native_region.imageExtent.width = region.texture_size.width;
        native_region.imageExtent.height = region.texture_size.height;
        native_region.imageExtent.depth = region.texture_size.depth;
    }

    vkCmdCopyBufferToImage(underlying_cast<VkCommandBuffer>(command_buffer),
                           underlying_cast<VkBuffer>(source),
                           underlying_cast<VkImage>(destination), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<std::uint32_t>(std::size(native_regions)), std::data(native_regions));
}

void copy(command_buffer& command_buffer, image& source, buffer& destination, const buffer_image_copy& region) noexcept
{
    assert((destination.size() - region.buffer_offset) >= source.byte_size() && "tph::cmd::copy called with too small buffer.");

    VkBufferCopy native_region{};
    native_region.srcOffset = region.buffer_offset;
    native_region.size = source.byte_size();

    vkCmdCopyBuffer(underlying_cast<VkCommandBuffer>(command_buffer),
                    underlying_cast<VkBuffer>(source),
                    underlying_cast<VkBuffer>(destination),
                    1, &native_region);
}

void copy(command_buffer& command_buffer, image& source, texture& destination, const image_texture_copy& region) noexcept
{
    VkBufferImageCopy native_region{};
    native_region.bufferRowLength = static_cast<std::uint32_t>(source.width());
    native_region.bufferImageHeight = static_cast<std::uint32_t>(source.height());
    native_region.imageSubresource.aspectMask = static_cast<VkImageAspectFlags>(destination.aspect());
    native_region.imageSubresource.mipLevel = 0;
    native_region.imageSubresource.baseArrayLayer = 0;
    native_region.imageSubresource.layerCount = 1;
    native_region.imageOffset.x = region.texture_offset.x;
    native_region.imageOffset.y = region.texture_offset.y;
    native_region.imageOffset.z = region.texture_offset.z;
    native_region.imageExtent.width = region.texture_size.width;
    native_region.imageExtent.height = region.texture_size.height;
    native_region.imageExtent.depth = region.texture_size.depth;

    vkCmdCopyBufferToImage(underlying_cast<VkCommandBuffer>(command_buffer),
                           underlying_cast<VkBuffer>(source),
                           underlying_cast<VkImage>(destination), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &native_region);
}

void copy(command_buffer& command_buffer, image& source, texture& destination, std::span<const image_texture_copy> regions)
{
    stack_memory_pool<1024 * 2> pool{};
    auto native_regions{make_stack_vector<VkBufferImageCopy>(pool)};
    native_regions.reserve(std::size(regions));

    for(auto&& region : regions)
    {
        VkBufferImageCopy& native_region{native_regions.emplace_back()};
        native_region.bufferRowLength = static_cast<std::uint32_t>(source.width());
        native_region.bufferImageHeight = static_cast<std::uint32_t>(source.height());
        native_region.imageSubresource.aspectMask = static_cast<VkImageAspectFlags>(destination.aspect());
        native_region.imageSubresource.mipLevel = 0;
        native_region.imageSubresource.baseArrayLayer = 0;
        native_region.imageSubresource.layerCount = 1;
        native_region.imageOffset.x = region.texture_offset.x;
        native_region.imageOffset.y = region.texture_offset.y;
        native_region.imageOffset.z = region.texture_offset.z;
        native_region.imageExtent.width = region.texture_size.width;
        native_region.imageExtent.height = region.texture_size.height;
        native_region.imageExtent.depth = region.texture_size.depth;
    }

    vkCmdCopyBufferToImage(underlying_cast<VkCommandBuffer>(command_buffer),
                           underlying_cast<VkBuffer>(source),
                           underlying_cast<VkImage>(destination), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<std::uint32_t>(std::size(native_regions)), std::data(native_regions));
}

void copy(command_buffer& command_buffer, texture& source, buffer& destination, const buffer_texture_copy& region) noexcept
{
    VkBufferImageCopy native_region{};
    native_region.bufferOffset = region.buffer_offset;
    native_region.bufferRowLength = region.buffer_image_width;
    native_region.bufferImageHeight = region.buffer_image_height;
    native_region.imageSubresource.aspectMask = static_cast<VkImageAspectFlags>(source.aspect());
    native_region.imageSubresource.mipLevel = 0;
    native_region.imageSubresource.baseArrayLayer = 0;
    native_region.imageSubresource.layerCount = 1;
    native_region.imageOffset.x = region.texture_offset.x;
    native_region.imageOffset.y = region.texture_offset.y;
    native_region.imageOffset.z = region.texture_offset.z;
    native_region.imageExtent.width = region.texture_size.width;
    native_region.imageExtent.height = region.texture_size.height;
    native_region.imageExtent.depth = region.texture_size.depth;

    vkCmdCopyImageToBuffer(underlying_cast<VkCommandBuffer>(command_buffer),
                           underlying_cast<VkImage>(source), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           underlying_cast<VkBuffer>(destination),
                           1, &native_region);
}

void copy(command_buffer& command_buffer, texture& source, buffer& destination, std::span<const buffer_texture_copy> regions)
{
    stack_memory_pool<1024 * 2> pool{};
    auto native_regions{make_stack_vector<VkBufferImageCopy>(pool)};
    native_regions.reserve(std::size(regions));

    for(auto&& region : regions)
    {
        VkBufferImageCopy& native_region{native_regions.emplace_back()};
        native_region.bufferOffset = region.buffer_offset;
        native_region.bufferRowLength = region.buffer_image_width;
        native_region.bufferImageHeight = region.buffer_image_height;
        native_region.imageSubresource.aspectMask = static_cast<VkImageAspectFlags>(source.aspect());
        native_region.imageSubresource.mipLevel = 0;
        native_region.imageSubresource.baseArrayLayer = 0;
        native_region.imageSubresource.layerCount = 1;
        native_region.imageOffset.x = region.texture_offset.x;
        native_region.imageOffset.y = region.texture_offset.y;
        native_region.imageOffset.z = region.texture_offset.z;
        native_region.imageExtent.width = region.texture_size.width;
        native_region.imageExtent.height = region.texture_size.height;
        native_region.imageExtent.depth = region.texture_size.depth;
    }

    vkCmdCopyImageToBuffer(underlying_cast<VkCommandBuffer>(command_buffer),
                           underlying_cast<VkImage>(source), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           underlying_cast<VkBuffer>(destination),
                           static_cast<std::uint32_t>(std::size(native_regions)), std::data(native_regions));
}

void copy(command_buffer& command_buffer, texture& source, image& destination, const image_texture_copy& region) noexcept
{
    VkBufferImageCopy native_region{};
    native_region.bufferRowLength = static_cast<std::uint32_t>(destination.width());
    native_region.bufferImageHeight = static_cast<std::uint32_t>(destination.height());
    native_region.imageSubresource.aspectMask = static_cast<VkImageAspectFlags>(source.aspect());
    native_region.imageSubresource.mipLevel = 0;
    native_region.imageSubresource.baseArrayLayer = 0;
    native_region.imageSubresource.layerCount = 1;
    native_region.imageOffset.x = region.texture_offset.x;
    native_region.imageOffset.y = region.texture_offset.y;
    native_region.imageOffset.z = region.texture_offset.z;
    native_region.imageExtent.width = region.texture_size.width;
    native_region.imageExtent.height = region.texture_size.height;
    native_region.imageExtent.depth = region.texture_size.depth;

    vkCmdCopyImageToBuffer(underlying_cast<VkCommandBuffer>(command_buffer),
                           underlying_cast<VkImage>(source), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           underlying_cast<VkBuffer>(destination),
                           1, &native_region);
}

void copy(command_buffer& command_buffer, texture& source, image& destination, std::span<const image_texture_copy> regions)
{
    stack_memory_pool<1024 * 2> pool{};
    auto native_regions{make_stack_vector<VkBufferImageCopy>(pool)};
    native_regions.reserve(std::size(regions));

    for(auto&& region : regions)
    {
        VkBufferImageCopy& native_region{native_regions.emplace_back()};
        native_region.bufferRowLength = static_cast<std::uint32_t>(destination.width());
        native_region.bufferImageHeight = static_cast<std::uint32_t>(destination.height());
        native_region.imageSubresource.aspectMask = static_cast<VkImageAspectFlags>(source.aspect());
        native_region.imageSubresource.mipLevel = 0;
        native_region.imageSubresource.baseArrayLayer = 0;
        native_region.imageSubresource.layerCount = 1;
        native_region.imageOffset.x = region.texture_offset.x;
        native_region.imageOffset.y = region.texture_offset.y;
        native_region.imageOffset.z = region.texture_offset.z;
        native_region.imageExtent.width = region.texture_size.width;
        native_region.imageExtent.height = region.texture_size.height;
        native_region.imageExtent.depth = region.texture_size.depth;
    }

    vkCmdCopyImageToBuffer(underlying_cast<VkCommandBuffer>(command_buffer),
                           underlying_cast<VkImage>(source), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           underlying_cast<VkBuffer>(destination),
                           static_cast<std::uint32_t>(std::size(native_regions)), std::data(native_regions));
}

void copy(command_buffer& command_buffer, texture& source, texture& destination, const texture_copy& region) noexcept
{
    VkImageCopy native_region{};
    native_region.srcSubresource.aspectMask = static_cast<VkImageAspectFlags>(source.aspect());
    native_region.srcSubresource.mipLevel = 0;
    native_region.srcSubresource.baseArrayLayer = 0;
    native_region.srcSubresource.layerCount = 1;
    native_region.srcOffset.x = region.source_offset.x;
    native_region.srcOffset.y = region.source_offset.y;
    native_region.srcOffset.z = region.source_offset.z;
    native_region.dstSubresource.aspectMask = static_cast<VkImageAspectFlags>(destination.aspect());
    native_region.dstSubresource.mipLevel = 0;
    native_region.dstSubresource.baseArrayLayer = 0;
    native_region.dstSubresource.layerCount = 1;
    native_region.dstOffset.x = region.destination_offset.x;
    native_region.dstOffset.y = region.destination_offset.y;
    native_region.dstOffset.z = region.destination_offset.z;
    native_region.extent.width = region.size.width;
    native_region.extent.height = region.size.height;
    native_region.extent.depth = region.size.depth;

    vkCmdCopyImage(underlying_cast<VkCommandBuffer>(command_buffer),
                   underlying_cast<VkImage>(source), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   underlying_cast<VkImage>(destination), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &native_region);
}

void copy(command_buffer& command_buffer, texture& source, texture& destination, std::span<const texture_copy> regions)
{
    stack_memory_pool<1024 * 2> pool{};
    auto native_regions{make_stack_vector<VkImageCopy>(pool)};
    native_regions.reserve(std::size(regions));

    for(auto&& region : regions)
    {
        VkImageCopy& native_region{native_regions.emplace_back()};
        native_region.srcSubresource.aspectMask = static_cast<VkImageAspectFlags>(source.aspect());
        native_region.srcSubresource.mipLevel = 0;
        native_region.srcSubresource.baseArrayLayer = 0;
        native_region.srcSubresource.layerCount = 1;
        native_region.srcOffset.x = region.source_offset.x;
        native_region.srcOffset.y = region.source_offset.y;
        native_region.srcOffset.z = region.source_offset.z;
        native_region.dstSubresource.aspectMask = static_cast<VkImageAspectFlags>(destination.aspect());
        native_region.dstSubresource.mipLevel = 0;
        native_region.dstSubresource.baseArrayLayer = 0;
        native_region.dstSubresource.layerCount = 1;
        native_region.dstOffset.x = region.destination_offset.x;
        native_region.dstOffset.y = region.destination_offset.y;
        native_region.dstOffset.z = region.destination_offset.z;
        native_region.extent.width = region.size.width;
        native_region.extent.height = region.size.height;
        native_region.extent.depth = region.size.depth;
    }

    vkCmdCopyImage(underlying_cast<VkCommandBuffer>(command_buffer),
                   underlying_cast<VkImage>(source), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   underlying_cast<VkImage>(destination), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   static_cast<std::uint32_t>(std::size(native_regions)), std::data(native_regions));
}

void copy(command_buffer& command_buffer, buffer& source, buffer& destination) noexcept
{
    assert(source.size() <= destination.size() && "tph::cmd::copy called with too small destination buffer.");

    VkBufferCopy native_region{};
    native_region.size = source.size();

    vkCmdCopyBuffer(underlying_cast<VkCommandBuffer>(command_buffer),
                    underlying_cast<VkBuffer>(source),
                    underlying_cast<VkBuffer>(destination),
                    1, &native_region);
}

void copy(command_buffer& command_buffer, image& source, image& destination) noexcept
{
    assert((source.width() == destination.width() && source.height() <= destination.height()) && "tph::cmd::copy called with images of different size.");

    VkBufferCopy native_region{};
    native_region.size = source.byte_size();

    vkCmdCopyBuffer(underlying_cast<VkCommandBuffer>(command_buffer),
                    underlying_cast<VkBuffer>(source),
                    underlying_cast<VkBuffer>(destination),
                    1, &native_region);
}

void copy(command_buffer& command_buffer, image& source, texture& destination) noexcept
{
    VkBufferImageCopy native_region{};
    native_region.bufferRowLength = static_cast<std::uint32_t>(source.width());
    native_region.bufferImageHeight = static_cast<std::uint32_t>(source.height());
    native_region.imageSubresource.aspectMask = static_cast<VkImageAspectFlags>(destination.aspect());
    native_region.imageSubresource.mipLevel = 0;
    native_region.imageSubresource.layerCount = 1;
    native_region.imageSubresource.baseArrayLayer = 0;
    native_region.imageExtent.width = destination.width();
    native_region.imageExtent.height = destination.height();
    native_region.imageExtent.depth = destination.depth();

    vkCmdCopyBufferToImage(underlying_cast<VkCommandBuffer>(command_buffer),
                           underlying_cast<VkBuffer>(source),
                           underlying_cast<VkImage>(destination), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &native_region);
}

void copy(command_buffer& command_buffer, texture& source, image& destination) noexcept
{
    VkBufferImageCopy native_region{};
    native_region.bufferRowLength = static_cast<std::uint32_t>(destination.width());
    native_region.bufferImageHeight = static_cast<std::uint32_t>(destination.height());
    native_region.imageSubresource.aspectMask = static_cast<VkImageAspectFlags>(source.aspect());
    native_region.imageSubresource.mipLevel = 0;
    native_region.imageSubresource.layerCount = 1;
    native_region.imageSubresource.baseArrayLayer = 0;
    native_region.imageExtent.width = source.width();
    native_region.imageExtent.height = source.height();
    native_region.imageExtent.depth = source.depth();

    vkCmdCopyImageToBuffer(underlying_cast<VkCommandBuffer>(command_buffer),
                           underlying_cast<VkImage>(source), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           underlying_cast<VkBuffer>(destination),
                           1, &native_region);
}

void copy(command_buffer& command_buffer, texture& source, texture& destination) noexcept
{
    assert((source.width() <= destination.width() && source.height() <= destination.height() && source.depth() <= destination.depth()) && "tph::cmd::copy called with too small destination texture");

    VkImageCopy native_region{};
    native_region.srcSubresource.aspectMask = static_cast<VkImageAspectFlags>(source.aspect());
    native_region.srcSubresource.mipLevel = 0;
    native_region.srcSubresource.layerCount = 1;
    native_region.srcSubresource.baseArrayLayer = 0;
    native_region.dstSubresource.aspectMask = static_cast<VkImageAspectFlags>(destination.aspect());
    native_region.dstSubresource.mipLevel = 0;
    native_region.dstSubresource.layerCount = 1;
    native_region.dstSubresource.baseArrayLayer = 0;
    native_region.extent.width = source.width();
    native_region.extent.height = source.height();
    native_region.extent.depth = source.depth();

    vkCmdCopyImage(underlying_cast<VkCommandBuffer>(command_buffer),
                   underlying_cast<VkImage>(source), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   underlying_cast<VkImage>(destination), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &native_region);
}

void blit(command_buffer& command_buffer, texture& source, texture& destination, filter filter, const texture_blit& region) noexcept
{
    VkImageBlit native_region{};
    native_region.srcOffsets[0].x = region.source_offset.x;
    native_region.srcOffsets[0].y = region.source_offset.y;
    native_region.srcOffsets[0].z = region.source_offset.z;
    native_region.srcOffsets[1].x = region.source_offset.x + region.source_size.width;
    native_region.srcOffsets[1].y = region.source_offset.y + region.source_size.height;
    native_region.srcOffsets[1].z = region.source_offset.z + region.source_size.depth;
    native_region.srcSubresource.aspectMask = static_cast<VkImageAspectFlags>(source.aspect());
    native_region.srcSubresource.mipLevel = 0;
    native_region.srcSubresource.layerCount = 1;
    native_region.srcSubresource.baseArrayLayer = 0;
    native_region.dstOffsets[0].x = region.destination_offset.x;
    native_region.dstOffsets[0].y = region.destination_offset.y;
    native_region.dstOffsets[0].z = region.destination_offset.z;
    native_region.dstOffsets[1].x = region.destination_offset.x + region.destination_size.width;
    native_region.dstOffsets[1].y = region.destination_offset.y + region.destination_size.height;
    native_region.dstOffsets[1].z = region.destination_offset.z + region.destination_size.depth;
    native_region.dstSubresource.aspectMask = static_cast<VkImageAspectFlags>(destination.aspect());
    native_region.dstSubresource.mipLevel = 0;
    native_region.dstSubresource.layerCount = 1;
    native_region.dstSubresource.baseArrayLayer = 0;

    vkCmdBlitImage(underlying_cast<VkCommandBuffer>(command_buffer),
                   underlying_cast<VkImage>(source), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   underlying_cast<VkImage>(destination), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &native_region, static_cast<VkFilter>(filter));
}

void blit(command_buffer& command_buffer, texture& source, texture& destination, filter filter, std::span<const texture_blit> regions)
{
    stack_memory_pool<1024 * 2> pool{};
    auto native_regions{make_stack_vector<VkImageBlit>(pool)};
    native_regions.reserve(std::size(regions));

    for(auto&& region : regions)
    {
        VkImageBlit& native_region{native_regions.emplace_back()};
        native_region.srcOffsets[0].x = region.source_offset.x;
        native_region.srcOffsets[0].y = region.source_offset.y;
        native_region.srcOffsets[0].z = region.source_offset.z;
        native_region.srcOffsets[1].x = region.source_offset.x + region.source_size.width;
        native_region.srcOffsets[1].y = region.source_offset.y + region.source_size.height;
        native_region.srcOffsets[1].z = region.source_offset.z + region.source_size.depth;
        native_region.srcSubresource.aspectMask = static_cast<VkImageAspectFlags>(source.aspect());
        native_region.srcSubresource.mipLevel = 0;
        native_region.srcSubresource.layerCount = 1;
        native_region.srcSubresource.baseArrayLayer = 0;
        native_region.dstOffsets[0].x = region.destination_offset.x;
        native_region.dstOffsets[0].y = region.destination_offset.y;
        native_region.dstOffsets[0].z = region.destination_offset.z;
        native_region.dstOffsets[1].x = region.destination_offset.x + region.destination_size.width;
        native_region.dstOffsets[1].y = region.destination_offset.y + region.destination_size.height;
        native_region.dstOffsets[1].z = region.destination_offset.z + region.destination_size.depth;
        native_region.dstSubresource.aspectMask = static_cast<VkImageAspectFlags>(destination.aspect());
        native_region.dstSubresource.mipLevel = 0;
        native_region.dstSubresource.layerCount = 1;
        native_region.dstSubresource.baseArrayLayer = 0;
    }

    vkCmdBlitImage(underlying_cast<VkCommandBuffer>(command_buffer),
                   underlying_cast<VkImage>(source), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   underlying_cast<VkImage>(destination), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   static_cast<std::uint32_t>(std::size(native_regions)), std::data(native_regions), static_cast<VkFilter>(filter));
}

void blit(command_buffer& command_buffer, texture& source, texture& destination, filter filter) noexcept
{
    VkImageBlit native_region{};
    native_region.srcOffsets[0].x = 0;
    native_region.srcOffsets[0].y = 0;
    native_region.srcOffsets[0].z = 0;
    native_region.srcOffsets[1].x = source.width();
    native_region.srcOffsets[1].y = source.height();
    native_region.srcOffsets[1].z = source.depth();
    native_region.srcSubresource.aspectMask = static_cast<VkImageAspectFlags>(source.aspect());
    native_region.srcSubresource.mipLevel = 0;
    native_region.srcSubresource.layerCount = 1;
    native_region.srcSubresource.baseArrayLayer = 0;
    native_region.dstOffsets[0].x = 0;
    native_region.dstOffsets[0].y = 0;
    native_region.dstOffsets[0].z = 0;
    native_region.dstOffsets[1].x = destination.width();
    native_region.dstOffsets[1].y = destination.height();
    native_region.dstOffsets[1].z = destination.depth();
    native_region.dstSubresource.aspectMask = static_cast<VkImageAspectFlags>(destination.aspect());
    native_region.dstSubresource.mipLevel = 0;
    native_region.dstSubresource.layerCount = 1;
    native_region.dstSubresource.baseArrayLayer = 0;

    vkCmdBlitImage(underlying_cast<VkCommandBuffer>(command_buffer),
                   underlying_cast<VkImage>(source), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   underlying_cast<VkImage>(destination), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &native_region, static_cast<VkFilter>(filter));
}

void transition(command_buffer& command_buffer, texture& texture, resource_access source_access, resource_access destination_access, pipeline_stage source_stage, pipeline_stage destination_stage, texture_layout current_layout, texture_layout next_layout) noexcept
{
    texture.transition(command_buffer, source_access, destination_access, source_stage, destination_stage, current_layout, next_layout);
}

void pipeline_barrier(command_buffer& command_buffer, pipeline_stage source_stage, pipeline_stage destination_stage) noexcept
{
    vkCmdPipelineBarrier(underlying_cast<VkCommandBuffer>(command_buffer),
                         static_cast<VkPipelineStageFlags>(source_stage), static_cast<VkPipelineStageFlags>(destination_stage), 0,
                         0, nullptr, 0, nullptr, 0, nullptr);
}

void pipeline_barrier(command_buffer& command_buffer, resource_access source_access, resource_access destination_access, pipeline_stage source_stage, pipeline_stage destination_stage) noexcept
{
    VkMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.srcAccessMask = static_cast<VkAccessFlags>(source_access);
    barrier.dstAccessMask = static_cast<VkAccessFlags>(destination_access);

    vkCmdPipelineBarrier(underlying_cast<VkCommandBuffer>(command_buffer),
                         static_cast<VkPipelineStageFlags>(source_stage), static_cast<VkPipelineStageFlags>(destination_stage), 0,
                         1, &barrier, 0, nullptr, 0, nullptr);
}

void update_buffer(command_buffer& command_buffer, tph::buffer& buffer, std::uint64_t offset, std::uint64_t size, const void* data) noexcept
{
    vkCmdUpdateBuffer(underlying_cast<VkCommandBuffer>(command_buffer), underlying_cast<VkBuffer>(buffer), offset, size, data);
}

void fill_buffer(command_buffer& command_buffer, tph::buffer& buffer, std::uint64_t offset, std::uint64_t size, std::uint32_t value) noexcept
{
    vkCmdFillBuffer(underlying_cast<VkCommandBuffer>(command_buffer), underlying_cast<VkBuffer>(buffer), offset, size, value);
}

void push_constants(command_buffer& command_buffer, pipeline_layout& layout, shader_stage stages, std::uint32_t offset, std::uint32_t size, const void* data) noexcept
{
    vkCmdPushConstants(underlying_cast<VkCommandBuffer>(command_buffer), underlying_cast<VkPipelineLayout>(layout), static_cast<VkShaderStageFlags>(stages), offset, size, data);
}

void begin_render_pass(command_buffer& command_buffer, const render_pass& render_pass, const framebuffer& framebuffer, render_pass_content content) noexcept
{
    begin_render_pass(command_buffer, render_pass, framebuffer, scissor{0, 0, framebuffer.width(), framebuffer.height()}, content);
}

void begin_render_pass(command_buffer& command_buffer, const render_pass& render_pass, const framebuffer& framebuffer, const scissor& area, render_pass_content content) noexcept
{
    stack_memory_pool<1024 * 2> pool{};
    auto clear_values{make_stack_vector<VkClearValue>(pool)};
    clear_values.reserve(std::size(framebuffer.clear_values()));

    for(auto&& value : framebuffer.clear_values())
    {
        VkClearValue& native_value{clear_values.emplace_back()};

        if(std::holds_alternative<clear_color_value>(value))
        {
            auto&& color{std::get<clear_color_value>(value)};
            native_value.color = VkClearColorValue{{color.red, color.green, color.blue, color.alpha}};
        }
        else if(std::holds_alternative<clear_depth_stencil_value>(value))
        {
            auto&& depth_stencil{std::get<clear_depth_stencil_value>(value)};
            native_value.depthStencil = VkClearDepthStencilValue{depth_stencil.depth, depth_stencil.stencil};
        }
    }

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = underlying_cast<VkRenderPass>(render_pass);
    render_pass_info.framebuffer = underlying_cast<VkFramebuffer>(framebuffer);
    render_pass_info.renderArea.offset = VkOffset2D{area.x, area.x};
    render_pass_info.renderArea.extent = VkExtent2D{area.width, area.height};
    render_pass_info.clearValueCount = static_cast<std::uint32_t>(std::size(clear_values));
    render_pass_info.pClearValues = std::data(clear_values);

    vkCmdBeginRenderPass(underlying_cast<VkCommandBuffer>(command_buffer), &render_pass_info, static_cast<VkSubpassContents>(content));
}

void next_subpass(command_buffer& command_buffer, render_pass_content content) noexcept
{
    vkCmdNextSubpass(underlying_cast<VkCommandBuffer>(command_buffer), static_cast<VkSubpassContents>(content));
}

void end_render_pass(command_buffer& command_buffer) noexcept
{
    vkCmdEndRenderPass(underlying_cast<VkCommandBuffer>(command_buffer));
}

void bind_pipeline(command_buffer& command_buffer, pipeline& pipeline) noexcept
{
    vkCmdBindPipeline(underlying_cast<VkCommandBuffer>(command_buffer), static_cast<VkPipelineBindPoint>(pipeline.type()), underlying_cast<VkPipeline>(pipeline));
}

void bind_vertex_buffer(command_buffer& command_buffer, buffer& buffer, std::uint64_t offset) noexcept
{
    VkBuffer native_buffer{underlying_cast<VkBuffer>(buffer)};
    vkCmdBindVertexBuffers(underlying_cast<VkCommandBuffer>(command_buffer), 0, 1, &native_buffer, &offset);
}

void bind_index_buffer(command_buffer& command_buffer, buffer& buffer, std::uint64_t offset, index_type type) noexcept
{
    vkCmdBindIndexBuffer(underlying_cast<VkCommandBuffer>(command_buffer), underlying_cast<VkBuffer>(buffer), offset, static_cast<VkIndexType>(type));
}

void reset_event(command_buffer& command_buffer, event& event, pipeline_stage stage) noexcept
{
    vkCmdResetEvent(underlying_cast<VkCommandBuffer>(command_buffer), underlying_cast<VkEvent>(event), static_cast<VkPipelineStageFlags>(stage));
}

void set_event(command_buffer& command_buffer, event& event, pipeline_stage stage) noexcept
{
    vkCmdSetEvent(underlying_cast<VkCommandBuffer>(command_buffer), underlying_cast<VkEvent>(event), static_cast<VkPipelineStageFlags>(stage));
}

void bind_descriptor_set(command_buffer& command_buffer, descriptor_set& descriptor_set, pipeline_layout& layout, pipeline_type bind_point) noexcept
{
    VkDescriptorSet native_descriptor_set{underlying_cast<VkDescriptorSet>(descriptor_set)};
    vkCmdBindDescriptorSets(underlying_cast<VkCommandBuffer>(command_buffer), static_cast<VkPipelineBindPoint>(bind_point), underlying_cast<VkPipelineLayout>(layout), 0, 1, &native_descriptor_set, 0, nullptr);
}

void set_viewport(command_buffer& command_buffer, const viewport& viewport, std::uint32_t index) noexcept
{
    VkViewport native_viewport{};
    native_viewport.x = viewport.x;
    native_viewport.y = viewport.y;
    native_viewport.width = viewport.width;
    native_viewport.height = viewport.height;
    native_viewport.minDepth = viewport.min_depth;
    native_viewport.maxDepth = viewport.max_depth;

    vkCmdSetViewport(underlying_cast<VkCommandBuffer>(command_buffer), index, 1, &native_viewport);
}

void set_scissor(command_buffer& command_buffer, const scissor& scissor, std::uint32_t index) noexcept
{
    VkRect2D native_scissor{};
    native_scissor.offset.x = scissor.x;
    native_scissor.offset.y = scissor.y;
    native_scissor.extent.width = scissor.width;
    native_scissor.extent.height = scissor.height;

    vkCmdSetScissor(underlying_cast<VkCommandBuffer>(command_buffer), index, 1, &native_scissor);
}

void set_line_width(command_buffer& command_buffer, float width) noexcept
{
    vkCmdSetLineWidth(underlying_cast<VkCommandBuffer>(command_buffer), width);
}

void set_depth_bias(command_buffer& command_buffer, float constant_factor, float clamp, float slope_factor) noexcept
{
    vkCmdSetDepthBias(underlying_cast<VkCommandBuffer>(command_buffer), constant_factor, clamp, slope_factor);
}

void set_blend_constants(command_buffer& command_buffer, float red, float green, float blue, float alpha) noexcept
{
    const std::array color{red, green, blue, alpha};

    vkCmdSetBlendConstants(underlying_cast<VkCommandBuffer>(command_buffer), std::data(color));
}

void set_depth_bounds(command_buffer& command_buffer, float min, float max) noexcept
{
    vkCmdSetDepthBounds(underlying_cast<VkCommandBuffer>(command_buffer), min, max);
}

void set_stencil_compare_mask(command_buffer& command_buffer, stencil_face face, std::uint32_t compare_mask) noexcept
{
    vkCmdSetStencilCompareMask(underlying_cast<VkCommandBuffer>(command_buffer), static_cast<VkStencilFaceFlagBits>(face), compare_mask);
}

void set_stencil_reference(command_buffer& command_buffer, stencil_face face, std::uint32_t reference) noexcept
{
    vkCmdSetStencilReference(underlying_cast<VkCommandBuffer>(command_buffer), static_cast<VkStencilFaceFlagBits>(face), reference);
}

void set_stencil_write_mask(command_buffer& command_buffer, stencil_face face, std::uint32_t write_mask) noexcept
{
    vkCmdSetStencilWriteMask(underlying_cast<VkCommandBuffer>(command_buffer), static_cast<VkStencilFaceFlagBits>(face), write_mask);
}

void draw(command_buffer& command_buffer, std::uint32_t vertex_count, std::uint32_t instance_count, std::uint32_t first_vertex, std::uint32_t first_instance) noexcept
{
    vkCmdDraw(underlying_cast<VkCommandBuffer>(command_buffer), vertex_count, instance_count, first_vertex, first_instance);
}

void draw_indexed(command_buffer& command_buffer, std::uint32_t index_count, std::uint32_t instance_count, std::uint32_t first_index, std::uint32_t first_vertex, std::uint32_t first_instance) noexcept
{
    vkCmdDrawIndexed(underlying_cast<VkCommandBuffer>(command_buffer), index_count, instance_count, first_index, first_vertex, first_instance);
}

void draw_indirect(command_buffer& command_buffer, buffer& buffer, std::uint64_t offset, std::uint32_t draw_count, std::uint32_t stride) noexcept
{
    vkCmdDrawIndirect(underlying_cast<VkCommandBuffer>(command_buffer), underlying_cast<VkBuffer>(buffer), offset, draw_count, stride);
}

void draw_indexed_indirect(command_buffer& command_buffer, buffer& buffer, std::uint64_t offset, std::uint32_t draw_count, std::uint32_t stride) noexcept
{
    vkCmdDrawIndexedIndirect(underlying_cast<VkCommandBuffer>(command_buffer), underlying_cast<VkBuffer>(buffer), offset, draw_count, stride);
}

void dispatch(command_buffer& command_buffer, std::uint32_t group_count_x, std::uint32_t group_count_y, std::uint32_t group_count_z) noexcept
{
    vkCmdDispatch(underlying_cast<VkCommandBuffer>(command_buffer), group_count_x, group_count_y, group_count_z);
}

void dispatch_indirect(command_buffer& command_buffer, buffer& buffer, std::uint64_t offset) noexcept
{
    vkCmdDispatchIndirect(underlying_cast<VkCommandBuffer>(command_buffer), underlying_cast<VkBuffer>(buffer), offset);
}

void reset_query_pool(command_buffer& command_buffer, query_pool& pool, std::uint32_t first, std::uint32_t count) noexcept
{
    vkCmdResetQueryPool(underlying_cast<VkCommandBuffer>(command_buffer), underlying_cast<VkQueryPool>(pool), first, count);
}

void write_timestamp(command_buffer& command_buffer, query_pool& pool, std::uint32_t query, pipeline_stage stage) noexcept
{
    vkCmdWriteTimestamp(underlying_cast<VkCommandBuffer>(command_buffer), static_cast<VkPipelineStageFlagBits>(stage), underlying_cast<VkQueryPool>(pool), query);
}

void begin_query(command_buffer& command_buffer, query_pool& pool, std::uint32_t query, query_control options) noexcept
{
    vkCmdBeginQuery(underlying_cast<VkCommandBuffer>(command_buffer), underlying_cast<VkQueryPool>(pool), query, static_cast<VkQueryControlFlags>(options));
}

void end_query(command_buffer& command_buffer, query_pool& pool, std::uint32_t query) noexcept
{
    vkCmdEndQuery(underlying_cast<VkCommandBuffer>(command_buffer), underlying_cast<VkQueryPool>(pool), query);
}

void copy_query_pool_results(command_buffer& command_buffer, query_pool& pool, std::uint32_t first, std::uint32_t count, buffer& destination, std::uint64_t offset, std::uint64_t stride, query_results options) noexcept
{
    vkCmdCopyQueryPoolResults(underlying_cast<VkCommandBuffer>(command_buffer), underlying_cast<VkQueryPool>(pool), first, count, underlying_cast<VkBuffer>(destination), offset, stride, static_cast<VkQueryResultFlags>(options));
}

void begin_label(command_buffer& command_buffer, const std::string& name, float red, float green, float blue, float alpha) noexcept
{
    VkDebugUtilsLabelEXT label{};
    label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    label.pLabelName = std::data(name);
    label.color[0] = red;
    label.color[1] = green;
    label.color[2] = blue;
    label.color[3] = alpha;

    vkCmdBeginDebugUtilsLabelEXT(underlying_cast<VkCommandBuffer>(command_buffer), &label);
}

void end_label(command_buffer& command_buffer) noexcept
{
    vkCmdEndDebugUtilsLabelEXT(underlying_cast<VkCommandBuffer>(command_buffer));
}

void insert_label(command_buffer& command_buffer, const std::string& name, float red, float green, float blue, float alpha) noexcept
{
    VkDebugUtilsLabelEXT label{};
    label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    label.pLabelName = std::data(name);
    label.color[0] = red;
    label.color[1] = green;
    label.color[2] = blue;
    label.color[3] = alpha;

    vkCmdInsertDebugUtilsLabelEXT(underlying_cast<VkCommandBuffer>(command_buffer), &label);
}

void end(command_buffer& command_buffer)
{
    if(auto result{vkEndCommandBuffer(underlying_cast<VkCommandBuffer>(command_buffer))}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

void execute(command_buffer& buffer, command_buffer& secondary_buffer) noexcept
{
    VkCommandBuffer native_secondary_buffer{underlying_cast<VkCommandBuffer>(secondary_buffer)};

    vkCmdExecuteCommands(underlying_cast<VkCommandBuffer>(buffer), 1, &native_secondary_buffer);
}

void execute(command_buffer& buffer, std::span<const command_buffer> secondary_buffers)
{
    stack_memory_pool<1024 * 2> pool{};
    auto native_secondary_buffers{make_stack_vector<VkCommandBuffer>(pool)};
    native_secondary_buffers.reserve(std::size(secondary_buffers));

    for(const command_buffer& secondary_buffer : secondary_buffers)
    {
        native_secondary_buffers.emplace_back(underlying_cast<VkCommandBuffer>(secondary_buffer));
    }

    vkCmdExecuteCommands(underlying_cast<VkCommandBuffer>(buffer), static_cast<std::uint32_t>(std::size(native_secondary_buffers)), std::data(native_secondary_buffers));
}

void execute(command_buffer& buffer, std::span<const std::reference_wrapper<command_buffer>> secondary_buffers)
{
    stack_memory_pool<1024 * 2> pool{};
    auto native_secondary_buffers{make_stack_vector<VkCommandBuffer>(pool)};
    native_secondary_buffers.reserve(std::size(secondary_buffers));

    for(const command_buffer& secondary_buffer : secondary_buffers)
    {
        native_secondary_buffers.emplace_back(underlying_cast<VkCommandBuffer>(secondary_buffer));
    }

    vkCmdExecuteCommands(underlying_cast<VkCommandBuffer>(buffer), static_cast<std::uint32_t>(std::size(native_secondary_buffers)), std::data(native_secondary_buffers));
}

}

void submit(renderer& renderer, const submit_info& info, optional_ref<fence> fence)
{
    submit(renderer, queue::graphics, info, fence);
}

void submit(renderer& renderer, std::span<const submit_info> submits, optional_ref<fence> fence)
{
    submit(renderer, queue::graphics, submits, fence);
}

void submit(renderer& renderer, queue queue, const submit_info& info, optional_ref<fence> fence)
{
    assert(std::size(info.wait_semaphores) == std::size(info.wait_stages) && "tph::submit_info::wait_semaphores and tph::submit_info::wait_stages must have the same size.");

    stack_memory_pool<1024 * 2> pool{};

    auto wait_semaphores{make_stack_vector<VkSemaphore>(pool)};
    wait_semaphores.reserve(std::size(info.wait_semaphores));
    for(const semaphore& semaphore : info.wait_semaphores)
    {
        wait_semaphores.emplace_back(underlying_cast<VkSemaphore>(semaphore));
    }

    auto wait_stages{make_stack_vector<VkPipelineStageFlags>(pool)};
    wait_stages.reserve(std::size(info.wait_stages));
    for(pipeline_stage wait_stage : info.wait_stages)
    {
        wait_stages.emplace_back(static_cast<VkPipelineStageFlags>(wait_stage));
    }

    auto command_buffers{make_stack_vector<VkCommandBuffer>(pool)};
    command_buffers.reserve(std::size(info.command_buffers));
    for(const command_buffer& command_buffer : info.command_buffers)
    {
        command_buffers.emplace_back(underlying_cast<VkCommandBuffer>(command_buffer));
    }

    auto signal_semaphores{make_stack_vector<VkSemaphore>(pool)};
    signal_semaphores.reserve(std::size(info.signal_semaphores));
    for(const semaphore& signal_semaphore : info.signal_semaphores)
    {
        signal_semaphores.emplace_back(underlying_cast<VkSemaphore>(signal_semaphore));
    }

    VkSubmitInfo native_submit{};
    native_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    native_submit.waitSemaphoreCount = static_cast<std::uint32_t>(std::size(wait_semaphores));
    native_submit.pWaitSemaphores = std::data(wait_semaphores);
    native_submit.pWaitDstStageMask = std::data(wait_stages);
    native_submit.commandBufferCount = static_cast<std::uint32_t>(std::size(command_buffers));
    native_submit.pCommandBuffers = std::data(command_buffers);
    native_submit.signalSemaphoreCount = static_cast<std::uint32_t>(std::size(signal_semaphores));
    native_submit.pSignalSemaphores = std::data(signal_semaphores);

    VkFence native_fence{fence.has_value() ? underlying_cast<VkFence>(fence.value()) : VkFence{}};

    if(auto result{vkQueueSubmit(underlying_cast<VkQueue>(renderer, queue), 1, &native_submit, native_fence)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

void submit(renderer& renderer, queue queue, std::span<const submit_info> submits, optional_ref<fence> fence)
{
    constexpr std::size_t pool_size{1024 * 4};

    struct temp_submit_info
    {
        stack_vector<VkSemaphore, pool_size> wait_semaphores{};
        stack_vector<VkPipelineStageFlags, pool_size> wait_stages{};
        stack_vector<VkCommandBuffer, pool_size> command_buffers{};
        stack_vector<VkSemaphore, pool_size> signal_semaphores{};
    };

    stack_memory_pool<pool_size> pool{};

    auto temp_submits{make_stack_vector<temp_submit_info>(pool)};
    temp_submits.reserve(std::size(submits));

    for(const auto& submit : submits)
    {
        assert(std::size(submit.wait_semaphores) == std::size(submit.wait_stages) && "tph::submit_info::wait_semaphores and tph::submit_info::wait_stages must have the same size.");

        auto wait_semaphores{make_stack_vector<VkSemaphore>(pool)};
        wait_semaphores.reserve(std::size(submit.wait_semaphores));
        for(const semaphore& semaphore : submit.wait_semaphores)
        {
            wait_semaphores.emplace_back(underlying_cast<VkSemaphore>(semaphore));
        }

        auto wait_stages{make_stack_vector<VkPipelineStageFlags>(pool)};
        wait_stages.reserve(std::size(submit.wait_stages));
        for(pipeline_stage wait_stage : submit.wait_stages)
        {
            wait_stages.emplace_back(static_cast<VkPipelineStageFlags>(wait_stage));
        }

        auto command_buffers{make_stack_vector<VkCommandBuffer>(pool)};
        command_buffers.reserve(std::size(submit.command_buffers));
        for(const command_buffer& command_buffer : submit.command_buffers)
        {
            command_buffers.emplace_back(underlying_cast<VkCommandBuffer>(command_buffer));
        }

        auto signal_semaphores{make_stack_vector<VkSemaphore>(pool)};
        signal_semaphores.reserve(std::size(submit.signal_semaphores));
        for(const semaphore& signal_semaphore : submit.signal_semaphores)
        {
            signal_semaphores.emplace_back(underlying_cast<VkSemaphore>(signal_semaphore));
        }

        temp_submits.emplace_back(temp_submit_info{std::move(wait_semaphores), std::move(wait_stages), std::move(command_buffers), std::move(signal_semaphores)});
    }

    auto native_submits{make_stack_vector<VkSubmitInfo>(pool)};
    native_submits.reserve(std::size(temp_submits));

    for(const auto& temp_submit : temp_submits)
    {
        VkSubmitInfo& native_submit{native_submits.emplace_back()};
        native_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        native_submit.waitSemaphoreCount = static_cast<std::uint32_t>(std::size(temp_submit.wait_semaphores));
        native_submit.pWaitSemaphores = std::data(temp_submit.wait_semaphores);
        native_submit.pWaitDstStageMask = std::data(temp_submit.wait_stages);
        native_submit.commandBufferCount = static_cast<std::uint32_t>(std::size(temp_submit.command_buffers));
        native_submit.pCommandBuffers = std::data(temp_submit.command_buffers);
        native_submit.signalSemaphoreCount = static_cast<std::uint32_t>(std::size(temp_submit.signal_semaphores));
        native_submit.pSignalSemaphores = std::data(temp_submit.signal_semaphores);
    }

    VkFence native_fence{fence.has_value() ? underlying_cast<VkFence>(fence.value()) : VkFence{}};

    if(auto result{vkQueueSubmit(underlying_cast<VkQueue>(renderer, queue), static_cast<std::uint32_t>(std::size(native_submits)), std::data(native_submits), native_fence)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

}
