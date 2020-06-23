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

fence::fence(renderer& renderer, bool signaled)
:m_device{underlying_cast<VkDevice>(renderer)}
{
    if(signaled)
        m_fence = vulkan::fence{underlying_cast<VkDevice>(renderer), VK_FENCE_CREATE_SIGNALED_BIT};
    else
        m_fence = vulkan::fence{underlying_cast<VkDevice>(renderer)};
}

void fence::reset()
{
    VkFence native_fence{m_fence};
    if(auto result{vkResetFences(m_device, 1, &native_fence)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

bool fence::wait_impl(std::uint64_t nanoseconds) const
{
    VkFence native_fence{m_fence};
    const auto result{vkWaitForFences(m_device, 1, &native_fence, VK_FALSE, nanoseconds)};
    if(result < 0)
        throw vulkan::error{result};

    return result != VK_TIMEOUT;
}

event::event(renderer& renderer)
:m_device{underlying_cast<VkDevice>(renderer)}
,m_event{underlying_cast<VkDevice>(renderer)}
{

}

void event::set()
{
    if(auto result{vkSetEvent(m_device, m_event)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

void event::reset()
{
    if(auto result{vkResetEvent(m_device, m_event)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

}
