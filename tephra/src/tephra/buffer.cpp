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

#include "buffer.hpp"

#include "vulkan/vulkan_functions.hpp"

#include "device.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

static constexpr buffer_usage not_extension{~(buffer_usage::device_only | buffer_usage::staging)};

buffer::buffer(device& dev, std::uint64_t size, buffer_usage usage)
:m_buffer{dev.context(), size, static_cast<VkBufferUsageFlags>(usage & not_extension)}
,m_size{size}
{
    if(static_cast<bool>(usage & buffer_usage::device_only))
    {
        m_memory = dev.allocator().allocate_bound(m_buffer, vulkan::memory_resource_type::linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }
    else
    {
        if(static_cast<bool>(usage & buffer_usage::staging))
        {
            m_memory = dev.allocator().allocate_bound(m_buffer, vulkan::memory_resource_type::linear, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        }
        else
        {
            m_memory = dev.allocator().allocate_bound(m_buffer, vulkan::memory_resource_type::linear, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
        }
    }
}

std::uint8_t* buffer::map()
{
    return reinterpret_cast<std::uint8_t*>(m_memory.map());
}

const std::uint8_t* buffer::map() const
{
    return reinterpret_cast<const std::uint8_t*>(m_memory.map());
}

void buffer::unmap() noexcept
{
    m_memory.unmap();
}

void set_object_name(device& dev, const buffer& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_BUFFER;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkBuffer>(object));
    info.pObjectName = std::data(name);

    vulkan::check(dev->vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(dev), &info));
}

}
