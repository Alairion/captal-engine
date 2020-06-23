#ifndef TEPHRA_VULKAN_HELPER_HPP_INCLUDED
#define TEPHRA_VULKAN_HELPER_HPP_INCLUDED

#include <functional>
#include <span>

#include "vulkan.hpp"

namespace tph::vulkan
{

TEPHRA_API std::uint32_t find_memory_type(VkPhysicalDevice physical_device, std::uint32_t type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal);
TEPHRA_API std::uint32_t find_memory_type(const VkPhysicalDeviceMemoryProperties& memory_properties, std::uint32_t type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal);
TEPHRA_API VkFormat find_format(VkPhysicalDevice physical_device, std::span<const VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

}

#endif
