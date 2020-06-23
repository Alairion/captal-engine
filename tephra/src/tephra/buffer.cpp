#include "buffer.hpp"

#include "renderer.hpp"

namespace tph
{

static constexpr buffer_usage not_extension{~(buffer_usage::device_only | buffer_usage::staging)};

buffer::buffer(renderer& renderer, std::uint64_t size, buffer_usage usage)
:m_buffer{underlying_cast<VkDevice>(renderer), size, static_cast<VkBufferUsageFlags>(usage & not_extension)}
,m_size{size}
{
    if(static_cast<bool>(usage & buffer_usage::device_only))
    {
        m_memory = renderer.allocator().allocate_bound(m_buffer, vulkan::memory_resource_type::linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }
    else
    {
        if(static_cast<bool>(usage & buffer_usage::staging))
        {
            m_memory = renderer.allocator().allocate_bound(m_buffer, vulkan::memory_resource_type::linear, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        }
        else
        {
            m_memory = renderer.allocator().allocate_bound(m_buffer, vulkan::memory_resource_type::linear, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
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

void buffer::unmap()
{
    m_memory.unmap();
}

}
