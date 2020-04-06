#ifndef TEPHRA_VULKAN_HELPER_HPP_INCLUDED
#define TEPHRA_VULKAN_HELPER_HPP_INCLUDED

#include <functional>

#include "vulkan.hpp"

namespace tph::vulkan
{

std::uint32_t find_memory_type(VkPhysicalDevice physical_device, std::uint32_t type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal);
std::uint32_t find_memory_type(const VkPhysicalDeviceMemoryProperties& memory_properties, std::uint32_t type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal);

VkFormat find_format(VkPhysicalDevice physical_device, std::initializer_list<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

template<typename T>
T align_down(T offset, T alignment) noexcept
{
    return offset & ~(alignment - 1);
}

template<typename T>
T align_up(T offset, T alignment) noexcept
{
    return align_down(offset + alignment - 1, alignment);
}

template<typename T>
std::size_t hash(const T& value)
{
    return std::hash<T>{}(value);
}

inline std::size_t combine_hash(std::size_t first, std::size_t second)
{
    return first ^ (second << 1);
}

}

#endif
