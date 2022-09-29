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

#include "synchronization.hpp"

#include "vulkan/vulkan_functions.hpp"

#include "device.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

semaphore::semaphore(device& device)
:m_semaphore{device.context()}
{

}

void set_object_name(device& device, const semaphore& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_SEMAPHORE;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkSemaphore>(object));
    info.pObjectName = std::data(name);

    vulkan::check(device->vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(device), &info));
}

fence::fence(device& device, bool signaled)
{
    if(signaled)
    {
        m_fence = vulkan::fence{device.context(), VK_FENCE_CREATE_SIGNALED_BIT};
    }
    else
    {
        m_fence = vulkan::fence{device.context()};
    }
}

void fence::reset()
{
    VkFence native_fence{m_fence};
    vulkan::check(context()->vkResetFences(m_fence.device(), 1, &native_fence));
}

bool fence::wait_impl(std::uint64_t nanoseconds) const
{
    VkFence native_fence{m_fence};

    if(nanoseconds != 0)
    {
        const auto result{context()->vkWaitForFences(m_fence.device(), 1, &native_fence, VK_FALSE, nanoseconds)};

        if(result < 0)
            throw vulkan::error{result};

        return result != VK_TIMEOUT;
    }
    else
    {
        const auto result{context()->vkGetFenceStatus(m_fence.device(), native_fence)};

        if(result < 0)
            throw vulkan::error{result};

        return result != VK_NOT_READY;
    }
}

void set_object_name(device& device, const fence& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_FENCE;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkFence>(object));
    info.pObjectName = std::data(name);

    vulkan::check(device->vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(device), &info));
}

event::event(device& device)
:m_event{device.context()}
{

}

void event::set()
{
    vulkan::check(context()->vkSetEvent(m_event.device(), m_event));
}

void event::reset()
{
    vulkan::check(context()->vkResetEvent(m_event.device(), m_event));
}

void set_object_name(device& device, const event& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_EVENT;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkEvent>(object));
    info.pObjectName = std::data(name);

    vulkan::check(device->vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(device), &info));
}

}
