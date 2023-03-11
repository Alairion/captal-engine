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

#ifndef TEPHRA_VULKAN_HELPER_HPP_INCLUDED
#define TEPHRA_VULKAN_HELPER_HPP_INCLUDED

#include <functional>
#include <span>

#include "vulkan.hpp"

namespace tph::vulkan
{

TEPHRA_API std::uint32_t find_memory_type(const instance_context& context, VkPhysicalDevice physical_device, std::uint32_t type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal);
TEPHRA_API std::uint32_t find_memory_type(const VkPhysicalDeviceMemoryProperties& memory_properties, std::uint32_t type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal);
TEPHRA_API VkFormat find_format(const instance_context& context, VkPhysicalDevice physical_device, std::span<const VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

}

#endif
