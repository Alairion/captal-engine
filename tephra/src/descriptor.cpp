#include "descriptor.hpp"

#include "vulkan/vulkan_functions.hpp"

#include "renderer.hpp"
#include "buffer.hpp"
#include "texture.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

descriptor_set_layout::descriptor_set_layout(renderer& renderer, const std::vector<descriptor_set_layout_binding>& bindings)
{
    std::vector<VkDescriptorSetLayoutBinding> native_bindings{};
    native_bindings.reserve(std::size(bindings));

    for(auto&& binding : bindings)
    {
        VkDescriptorSetLayoutBinding native_binding{};
        native_binding.stageFlags = static_cast<VkShaderStageFlags>(binding.stages);
        native_binding.binding = binding.binding;
        native_binding.descriptorType = static_cast<VkDescriptorType>(binding.type);
        native_binding.descriptorCount = binding.count;

        native_bindings.push_back(native_binding);
    }

    m_layout = vulkan::descriptor_set_layout{underlying_cast<VkDevice>(renderer), native_bindings};
}

descriptor_pool::descriptor_pool(renderer& renderer, const std::vector<descriptor_pool_size>& sizes, std::optional<std::uint32_t> max_sets)
{
    std::vector<VkDescriptorPoolSize> native_sizes{};
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
            total_size += size.count;

        max_sets = total_size;
    }

    m_descriptor_pool = vulkan::descriptor_pool{underlying_cast<VkDevice>(renderer), native_sizes, max_sets.value()};
}

descriptor_set::descriptor_set(renderer& renderer, descriptor_pool& pool, descriptor_set_layout& layout)
:m_descriptor_set{underlying_cast<VkDevice>(renderer), underlying_cast<VkDescriptorPool>(pool), underlying_cast<VkDescriptorSetLayout>(layout)}
{

}

void write_descriptor(renderer& renderer, descriptor_set& descriptor_set, std::uint32_t binding, buffer& buffer, uint64_t offset, uint64_t size)
{
    write_descriptor(renderer, descriptor_set, binding, 0, buffer, offset, size);
}

void write_descriptor(renderer& renderer, descriptor_set& descriptor_set, std::uint32_t binding, texture& texture)
{
    write_descriptor(renderer, descriptor_set, binding, 0, texture);
}

void write_descriptor(renderer& renderer, descriptor_set& descriptor_set, std::uint32_t binding, std::uint32_t array_index, buffer& buffer, std::uint64_t offset, std::uint64_t size)
{
    VkDescriptorBufferInfo descriptor_ubo{};
    descriptor_ubo.buffer = underlying_cast<VkBuffer>(buffer);
    descriptor_ubo.offset = offset;
    descriptor_ubo.range = size;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = underlying_cast<VkDescriptorSet>(descriptor_set);
    write.dstBinding = binding;
    write.dstArrayElement = array_index;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &descriptor_ubo;

    vkUpdateDescriptorSets(underlying_cast<VkDevice>(renderer), 1, &write, 0, nullptr);
}

void write_descriptor(renderer& renderer, descriptor_set& descriptor_set, std::uint32_t binding, std::uint32_t array_index, texture& texture)
{
    VkDescriptorImageInfo descriptor_image{};
    descriptor_image.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptor_image.imageView = underlying_cast<VkImageView>(texture);
    descriptor_image.sampler = underlying_cast<VkSampler>(texture);

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = underlying_cast<VkDescriptorSet>(descriptor_set);
    write.dstBinding = binding;
    write.dstArrayElement = array_index;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &descriptor_image;

    vkUpdateDescriptorSets(underlying_cast<VkDevice>(renderer), 1, &write, 0, nullptr);
}

}
