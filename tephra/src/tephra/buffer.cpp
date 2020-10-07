#include "buffer.hpp"

#include "vulkan/vulkan_functions.hpp"

#include "renderer.hpp"

using namespace tph::vulkan::functions;

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

void set_object_name(renderer& renderer, const buffer& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_BUFFER;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkBuffer>(object));
    info.pObjectName = std::data(name);

    if(auto result{vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(renderer), &info)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

}
