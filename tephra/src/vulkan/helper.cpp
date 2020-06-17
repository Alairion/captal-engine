#include "helper.hpp"

#include <stdexcept>

#include "vulkan_functions.hpp"

using namespace tph::vulkan::functions;

namespace tph::vulkan
{

std::uint32_t find_memory_type(VkPhysicalDevice physical_device, std::uint32_t type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal)
{
    VkPhysicalDeviceMemoryProperties memory_properties{};
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

    return find_memory_type(memory_properties, type, minimal, optimal);
}

std::uint32_t find_memory_type(const VkPhysicalDeviceMemoryProperties& memory_properties, std::uint32_t type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal)
{
    if(optimal != 0)
    {
        for(std::uint32_t i{}; i < memory_properties.memoryTypeCount; ++i)
            if(type & (1 << i) && memory_properties.memoryTypes[i].propertyFlags == optimal)
                return i;

        for(std::uint32_t i{}; i < memory_properties.memoryTypeCount; ++i)
            if(type & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & optimal) == optimal)
                return i;
    }

    for(std::uint32_t i{}; i < memory_properties.memoryTypeCount; ++i)
        if(type & (1 << i) && memory_properties.memoryTypes[i].propertyFlags == minimal)
            return i;

    for(std::uint32_t i{}; i < memory_properties.memoryTypeCount; ++i)
        if(type & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & minimal) == minimal)
            return i;

    throw std::runtime_error{"Can not find a suitable memory type."};
}

VkFormat find_format(VkPhysicalDevice physical_device, std::span<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for(VkFormat format : candidates)
    {
        VkFormatProperties props{};
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

        if(tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
        else if(tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error{"Can not find suitable format."};
}

}
