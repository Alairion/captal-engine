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

#include "helper.hpp"

#include <stdexcept>

#include "vulkan_functions.hpp"

using namespace tph::vulkan::functions;

namespace tph::vulkan
{

std::uint32_t find_memory_type(const instance_context& context, VkPhysicalDevice physical_device, std::uint32_t type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal)
{
    VkPhysicalDeviceMemoryProperties memory_properties{};
    context->vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

    return find_memory_type(memory_properties, type, minimal, optimal);
}

std::uint32_t find_memory_type(const VkPhysicalDeviceMemoryProperties& properties, std::uint32_t type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal)
{
    if(optimal != 0)
    {
        for(std::uint32_t i{}; i < properties.memoryTypeCount; ++i)
        {
            if(type & (1 << i) && properties.memoryTypes[i].propertyFlags == optimal)
            {
                return i;
            }
        }

        for(std::uint32_t i{}; i < properties.memoryTypeCount; ++i)
        {
            if(type & (1 << i) && (properties.memoryTypes[i].propertyFlags & optimal) == optimal)
            {
                return i;
            }
        }
    }

    for(std::uint32_t i{}; i < properties.memoryTypeCount; ++i)
    {
        if(type & (1 << i) && properties.memoryTypes[i].propertyFlags == minimal)
        {
            return i;
        }
    }

    for(std::uint32_t i{}; i < properties.memoryTypeCount; ++i)
    {
        if(type & (1 << i) && (properties.memoryTypes[i].propertyFlags & minimal) == minimal)
        {
            return i;
        }
    }

    throw std::runtime_error{"Can not find a suitable memory type."};
}

VkFormat find_format(const instance_context& context, VkPhysicalDevice physical_device, std::span<const VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for(VkFormat format : candidates)
    {
        VkFormatProperties props{};
        context->vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

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
