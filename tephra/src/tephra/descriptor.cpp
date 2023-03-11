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

#include "descriptor.hpp"

#include <cassert>

#include <captal_foundation/stack_allocator.hpp>

#include "vulkan/vulkan_functions.hpp"

#include "device.hpp"
#include "buffer.hpp"
#include "texture.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

descriptor_set_layout::descriptor_set_layout(device& dev, std::span<const descriptor_set_layout_binding> bindings)
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

    m_layout = vulkan::descriptor_set_layout{dev.context(), native_bindings};
}

void set_object_name(device& dev, const descriptor_set_layout& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkDescriptorSetLayout>(object));
    info.pObjectName = std::data(name);

    vulkan::check(dev->vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(dev), &info));
}

descriptor_pool::descriptor_pool(device& dev, std::span<const descriptor_pool_size> sizes, std::optional<std::uint32_t> max_sets)
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

    m_descriptor_pool = vulkan::descriptor_pool{dev.context(), native_sizes, *max_sets};
}

void set_object_name(device& dev, const descriptor_pool& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_DESCRIPTOR_POOL;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkDescriptorPool>(object));
    info.pObjectName = std::data(name);

    vulkan::check(dev->vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(dev), &info));
}

descriptor_set::descriptor_set(device& dev, descriptor_pool& pool, descriptor_set_layout& layout)
:m_descriptor_set{dev.context(), underlying_cast<VkDescriptorPool>(pool), underlying_cast<VkDescriptorSetLayout>(layout)}
{

}

void set_object_name(device& dev, const descriptor_set& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkDescriptorSet>(object));
    info.pObjectName = std::data(name);

    vulkan::check(dev->vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(dev), &info));
}

void write_descriptors(device& dev, std::span<const descriptor_write> writes)
{
    update_descriptors(dev, writes, std::span<const descriptor_copy>{});
}

void copy_descriptors(device& dev, std::span<const descriptor_copy> copies)
{
    update_descriptors(dev, std::span<const descriptor_write>{}, copies);
}

void update_descriptors(device& dev, std::span<const descriptor_write> writes, std::span<const descriptor_copy> copies)
{
    std::size_t image_count{};
    std::size_t buffer_count{};

    for(auto&& write : writes)
    {
        assert(!std::holds_alternative<std::monostate>(write.info) && "tph::write_descriptors contains undefined write target.");

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
        native_write.dstSet = underlying_cast<VkDescriptorSet>(write.descriptor_set.get());
        native_write.dstBinding = write.binding;
        native_write.dstArrayElement = write.array_index;
        native_write.descriptorType = static_cast<VkDescriptorType>(write.type);
        native_write.descriptorCount = 1;

        if(std::holds_alternative<descriptor_texture_info>(write.info))
        {
            auto& write_info{std::get<descriptor_texture_info>(write.info)};

            VkDescriptorImageInfo image_info{};
            image_info.sampler = write_info.sampler ? underlying_cast<VkSampler>(*write_info.sampler) : VkSampler{};
            image_info.imageView = write_info.texture_view ? underlying_cast<VkImageView>(*write_info.texture_view) : VkImageView{};
            image_info.imageLayout = static_cast<VkImageLayout>(write_info.layout);

            native_write.pImageInfo = &native_images.emplace_back(image_info);
        }
        else if(std::holds_alternative<descriptor_buffer_info>(write.info))
        {
            auto& write_info{std::get<descriptor_buffer_info>(write.info)};

            VkDescriptorBufferInfo buffer_info{};
            buffer_info.buffer = underlying_cast<VkBuffer>(write_info.buffer.get());
            buffer_info.offset = write_info.offset;
            buffer_info.range = write_info.size;

            native_write.pBufferInfo = &buffers_image.emplace_back(buffer_info);
        }

        native_writes.emplace_back(native_write);
    }

    auto native_copies{make_stack_vector<VkCopyDescriptorSet>(pool)};
    native_copies.reserve(std::size(copies));

    for(auto&& copy : copies)
    {
        VkCopyDescriptorSet& native_copy{native_copies.emplace_back()};
        native_copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
        native_copy.srcSet = underlying_cast<VkDescriptorSet>(copy.source_set.get());
        native_copy.srcBinding = copy.source_binding;
        native_copy.srcArrayElement = copy.source_array_index;
        native_copy.dstSet = underlying_cast<VkDescriptorSet>(copy.dest_set.get());
        native_copy.dstBinding = copy.dest_binding;
        native_copy.dstArrayElement = copy.dest_array_index;
        native_copy.descriptorCount = copy.count;
    }

    dev->vkUpdateDescriptorSets(underlying_cast<VkDevice>(dev), static_cast<std::uint32_t>(std::size(native_writes)), std::data(native_writes), static_cast<std::uint32_t>(std::size(native_copies)), std::data(native_copies));
}

}
