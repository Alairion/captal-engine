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

#include "surface.hpp"

#include <captal_foundation/stack_allocator.hpp>

#include "vulkan/vulkan_functions.hpp"

#include "application.hpp"
#include "hardware.hpp"
#include "device.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

static surface_capabilities convert_capabilities(const vulkan::instance_context& context, VkPhysicalDevice phydev, VkSurfaceKHR surf)
{
    VkSurfaceCapabilitiesKHR capabilities{};
    vulkan::check(context->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phydev, surf, &capabilities));

    surface_capabilities output{};
    output.min_image_count = capabilities.minImageCount;
    output.max_image_count = capabilities.maxImageCount;
    output.current_width = capabilities.currentExtent.width;
    output.current_height = capabilities.currentExtent.height;
    output.min_width = capabilities.minImageExtent.width;
    output.min_height = capabilities.minImageExtent.height;
    output.max_width = capabilities.maxImageExtent.width;
    output.max_height = capabilities.maxImageExtent.height;
    output.max_array_layers = capabilities.maxImageArrayLayers;
    output.supported_transforms = static_cast<surface_transform>(capabilities.supportedTransforms);
    output.current_transform = static_cast<surface_transform>(capabilities.currentTransform);
    output.supported_composites = static_cast<surface_composite>(capabilities.supportedCompositeAlpha);
    output.supported_usages = static_cast<texture_usage>(capabilities.supportedUsageFlags);

    return output;
}

static std::vector<texture_format> convert_formats(const vulkan::instance_context& context, VkPhysicalDevice phydev, VkSurfaceKHR surf)
{
    std::uint32_t count{};
    vulkan::check(context->vkGetPhysicalDeviceSurfaceFormatsKHR(phydev, surf, &count, nullptr));

    stack_memory_pool<512> pool{};
    auto formats{make_stack_vector<VkSurfaceFormatKHR>(pool)};
    formats.resize(count);
    vulkan::check(context->vkGetPhysicalDeviceSurfaceFormatsKHR(phydev, surf, &count, std::data(formats)));

    std::vector<texture_format> output{};
    output.reserve(std::size(formats));
    for(auto format : formats)
    {
        output.emplace_back(static_cast<texture_format>(format.format));
    }

    return output;
}

#ifdef TPH_PLATFORM_ANDROID
surface::surface(application& app, const vulkan::android_surface_info& info)
:m_surface{app.context(), info}
{

}
#endif

#ifdef TPH_PLATFORM_IOS
surface::surface(application& app, const vulkan::ios_surface_info& info)
:m_surface{app.context(), info}
{

}
#endif

#ifdef TPH_PLATFORM_WIN32
surface::surface(application& app, const vulkan::win32_surface_info& info)
:m_surface{app.context(), info}
{

}
#endif

#ifdef TPH_PLATFORM_MACOS
surface::surface(application& app, const vulkan::macos_surface_info& info)
:m_surface{app.context(), info}
{

}
#endif

#ifdef TPH_PLATFORM_XLIB
surface::surface(application& app, const vulkan::xlib_surface_info& info)
:m_surface{app.context(), info}
{

}
#endif

#ifdef TPH_PLATFORM_XCB
surface::surface(application& app, const vulkan::xcb_surface_info& info)
:m_surface{app.context(), info}
{

}
#endif

#ifdef TPH_PLATFORM_WAYLAND
surface::surface(application& app, const vulkan::wayland_surface_info& info)
:m_surface{app.context(), info}
{

}
#endif

surface_capabilities surface::capabilities(const physical_device& phydev) const
{
    return convert_capabilities(context(), underlying_cast<VkPhysicalDevice>(phydev), m_surface);
}

surface_capabilities surface::capabilities(const device& dev) const
{
    return convert_capabilities(context(), underlying_cast<VkPhysicalDevice>(dev), m_surface);
}

std::vector<texture_format> surface::formats(const physical_device& phydev) const
{
    return convert_formats(context(), underlying_cast<VkPhysicalDevice>(phydev), m_surface);
}

std::vector<texture_format> surface::formats(const device& dev) const
{
    return convert_formats(context(), underlying_cast<VkPhysicalDevice>(dev), m_surface);
}

void set_object_name(device& dev, const surface& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_SURFACE_KHR;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkSurfaceKHR>(object));
    info.pObjectName = std::data(name);

    vulkan::check(dev->vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(dev), &info));
}

}
