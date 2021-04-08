#include "descriptor.hpp"

#include <cassert>

#include <captal_foundation/stack_allocator.hpp>

#include "vulkan/vulkan_functions.hpp"

#include "renderer.hpp"
#include "buffer.hpp"
#include "texture.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

descriptor_set_layout::descriptor_set_layout(renderer& renderer, std::span<const descriptor_set_layout_binding> bindings)
{
    stack_memory_pool<1024 * 2> pool{};

    auto native_bindings{make_stack_vector<VkDescriptorSetLayoutBinding>(pool)};
    native_bindings.reserve(std::size(bindings));

    for(auto&& binding : bindings)
    {
        VkDescriptorSetLayoutBinding native_binding{};
        native_binding.stageFlags = static_cast<VkShaderStageFlags>(binding.stages);
        native_binding.binding = binding.binding;
        native_binding.descriptorType = static_cast<VkDescriptorType>(binding.type);
        native_binding.descriptorCount = binding.count;

        native_bindings.emplace_back(native_binding);
    }

    m_layout = vulkan::descriptor_set_layout{underlying_cast<VkDevice>(renderer), native_bindings};
}

void set_object_name(renderer& renderer, const descriptor_set_layout& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkDescriptorSetLayout>(object));
    info.pObjectName = std::data(name);

    if(auto result{vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(renderer), &info)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

descriptor_pool::descriptor_pool(renderer& renderer, std::span<const descriptor_pool_size> sizes, std::optional<std::uint32_t> max_sets)
{
    stack_memory_pool<1024> pool{};

    auto native_sizes{make_stack_vector<VkDescriptorPoolSize>(pool)};
    native_sizes.reserve(std::size(sizes));

    for(auto&& size : sizes)
    {
        auto& native_size{native_sizes.emplace_back()};
        native_size.type = static_cast<VkDescriptorType>(size.type);
        native_size.descriptorCount = size.count;
    }

    if(!max_sets.has_value())
    {
        std::uint32_t total_size{};
        for(auto&& size : sizes)
        {
            total_size += size.count;
        }

        max_sets = total_size;
    }

    m_descriptor_pool = vulkan::descriptor_pool{underlying_cast<VkDevice>(renderer), native_sizes, *max_sets};
}

void set_object_name(renderer& renderer, const descriptor_pool& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_DESCRIPTOR_POOL;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkDescriptorPool>(object));
    info.pObjectName = std::data(name);

    if(auto result{vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(renderer), &info)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

descriptor_set::descriptor_set(renderer& renderer, descriptor_pool& pool, descriptor_set_layout& layout)
:m_descriptor_set{underlying_cast<VkDevice>(renderer), underlying_cast<VkDescriptorPool>(pool), underlying_cast<VkDescriptorSetLayout>(layout)}
{

}

void set_object_name(renderer& renderer, const descriptor_set& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkDescriptorSet>(object));
    info.pObjectName = std::data(name);

    if(auto result{vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(renderer), &info)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

void write_descriptor(renderer& renderer, descriptor_set& descriptor_set, std::uint32_t binding, std::uint32_t array_index, descriptor_type type, buffer& buffer, std::uint64_t offset, std::uint64_t size)
{
    VkDescriptorBufferInfo buffer_info{};
    buffer_info.buffer = underlying_cast<VkBuffer>(buffer);
    buffer_info.offset = offset;
    buffer_info.range = size;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = underlying_cast<VkDescriptorSet>(descriptor_set);
    write.dstBinding = binding;
    write.dstArrayElement = array_index;
    write.descriptorType = static_cast<VkDescriptorType>(type);
    write.descriptorCount = 1;
    write.pBufferInfo = &buffer_info;

    vkUpdateDescriptorSets(underlying_cast<VkDevice>(renderer), 1, &write, 0, nullptr);
}

void write_descriptor(renderer& renderer, descriptor_set& descriptor_set, std::uint32_t binding, std::uint32_t array_index, descriptor_type type, texture& texture, texture_layout layout)
{
    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = static_cast<VkImageLayout>(layout);
    image_info.imageView = underlying_cast<VkImageView>(texture);
    image_info.sampler = underlying_cast<VkSampler>(texture); //will be null if the texture has no sampler

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = underlying_cast<VkDescriptorSet>(descriptor_set);
    write.dstBinding = binding;
    write.dstArrayElement = array_index;
    write.descriptorType = static_cast<VkDescriptorType>(type);
    write.descriptorCount = 1;
    write.pImageInfo = &image_info;

    vkUpdateDescriptorSets(underlying_cast<VkDevice>(renderer), 1, &write, 0, nullptr);
}

void write_descriptors(renderer& renderer, std::span<const descriptor_write> writes)
{
    std::size_t image_count{};
    std::size_t buffer_count{};

    for(auto&& write : writes)
    {
        assert(!std::holds_alternative<std::monostate>(write.info) && "tph::write_descriptors contains underfined write target.");

        if(std::holds_alternative<descriptor_texture_info>(write.info))
        {
            ++image_count;
        }
        else if(std::holds_alternative<descriptor_buffer_info>(write.info))
        {
            ++buffer_count;
        }
    }

    stack_memory_pool<1024 * 4> pool{};

    auto native_images{make_stack_vector<VkDescriptorImageInfo>(pool)};
    native_images.reserve(image_count);
    auto buffers_image{make_stack_vector<VkDescriptorBufferInfo>(pool)};
    buffers_image.reserve(buffer_count);

    auto native_writes{make_stack_vector<VkWriteDescriptorSet>(pool)};
    native_writes.reserve(std::size(writes));

    for(auto&& write : writes)
    {
        VkWriteDescriptorSet native_write{};
        native_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        native_write.dstSet = underlying_cast<VkDescriptorSet>(write.descriptor_set);
        native_write.dstBinding = write.binding;
        native_write.dstArrayElement = write.array_index;
        native_write.descriptorType = static_cast<VkDescriptorType>(write.type);
        native_write.descriptorCount = 1;

        if(std::holds_alternative<descriptor_texture_info>(write.info))
        {
            auto& write_info{std::get<descriptor_texture_info>(write.info)};

            VkDescriptorImageInfo image_info{};
            image_info.imageLayout = static_cast<VkImageLayout>(write_info.layout);
            image_info.imageView = underlying_cast<VkImageView>(write_info.texture);
            image_info.sampler = underlying_cast<VkSampler>(write_info.texture); //will be null if the texture has no sampler

            native_write.pImageInfo = &native_images.emplace_back(image_info);
        }
        else if(std::holds_alternative<descriptor_buffer_info>(write.info))
        {
            auto& write_info{std::get<descriptor_buffer_info>(write.info)};

            VkDescriptorBufferInfo buffer_info{};
            buffer_info.buffer = underlying_cast<VkBuffer>(write_info.buffer);
            buffer_info.offset = write_info.offset;
            buffer_info.range = write_info.size;

            native_write.pBufferInfo = &buffers_image.emplace_back(buffer_info);
        }

        native_writes.emplace_back(native_write);
    }

    vkUpdateDescriptorSets(underlying_cast<VkDevice>(renderer), static_cast<std::uint32_t>(std::size(native_writes)), std::data(native_writes), 0, nullptr);
}

}
