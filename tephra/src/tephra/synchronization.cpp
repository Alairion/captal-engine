#include "synchronization.hpp"

#include "vulkan/vulkan_functions.hpp"

#include "renderer.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

semaphore::semaphore(renderer& renderer)
:m_semaphore{underlying_cast<VkDevice>(renderer)}
{

}

void set_object_name(renderer& renderer, const semaphore& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_SEMAPHORE;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkSemaphore>(object));
    info.pObjectName = std::data(name);

    if(auto result{vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(renderer), &info)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

fence::fence(renderer& renderer, bool signaled)
{
    if(signaled)
        m_fence = vulkan::fence{underlying_cast<VkDevice>(renderer), VK_FENCE_CREATE_SIGNALED_BIT};
    else
        m_fence = vulkan::fence{underlying_cast<VkDevice>(renderer)};
}

void fence::reset()
{
    VkFence native_fence{m_fence};
    if(auto result{vkResetFences(m_fence.device(), 1, &native_fence)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

bool fence::wait_impl(std::uint64_t nanoseconds) const
{
    VkFence native_fence{m_fence};
    const auto result{vkWaitForFences(m_fence.device(), 1, &native_fence, VK_FALSE, nanoseconds)};
    if(result < 0)
        throw vulkan::error{result};

    return result != VK_TIMEOUT;
}

void set_object_name(renderer& renderer, const fence& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_FENCE;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkFence>(object));
    info.pObjectName = std::data(name);

    if(auto result{vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(renderer), &info)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

event::event(renderer& renderer)
:m_event{underlying_cast<VkDevice>(renderer)}
{

}

void event::set()
{
    if(auto result{vkSetEvent(m_event.device(), m_event)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

void event::reset()
{
    if(auto result{vkResetEvent(m_event.device(), m_event)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

void set_object_name(renderer& renderer, const event& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_EVENT;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkEvent>(object));
    info.pObjectName = std::data(name);

    if(auto result{vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(renderer), &info)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

}
