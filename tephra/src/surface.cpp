#include "surface.hpp"

#include "vulkan/vulkan_functions.hpp"

#include "application.hpp"
#include "hardware.hpp"
#include "renderer.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

#ifdef TPH_PLATFORM_ANDROID
surface::surface(application& application, const vulkan::android_surface_info& info)
:m_surface{underlying_cast<VkInstance>(application), info}
{

}
#endif

#ifdef TPH_PLATFORM_IOS
surface::surface(application& application, const vulkan::ios_surface_info& info)
:m_surface{underlying_cast<VkInstance>(application), info}
{

}
#endif

#ifdef TPH_PLATFORM_WIN32
surface::surface(application& application, const vulkan::win32_surface_info& info)
:m_surface{underlying_cast<VkInstance>(application), info}
{

}
#endif

#ifdef TPH_PLATFORM_MACOS
surface::surface(application& application, const vulkan::macos_surface_info& info)
:m_surface{underlying_cast<VkInstance>(application), info}
{

}
#endif

#ifdef TPH_PLATFORM_XLIB
surface::surface(application& application, const vulkan::xlib_surface_info& info)
:m_surface{underlying_cast<VkInstance>(application), info}
{

}
#endif

#ifdef TPH_PLATFORM_XCB
surface::surface(application& application, const vulkan::xcb_surface_info& info)
:m_surface{underlying_cast<VkInstance>(application), info}
{

}
#endif

#ifdef TPH_PLATFORM_WAYLAND
surface::surface(application& application, const vulkan::wayland_surface_info& info)
:m_surface{underlying_cast<VkInstance>(application), info}
{

}
#endif

std::pair<std::uint32_t, std::uint32_t> surface::size(const physical_device& physical_device) const
{
    VkSurfaceCapabilitiesKHR capabilities{};
    if(auto result{vkGetPhysicalDeviceSurfaceCapabilitiesKHR(underlying_cast<VkPhysicalDevice>(physical_device), m_surface, &capabilities)}; result != VK_SUCCESS)
        throw vulkan::error{result};

    if(capabilities.currentExtent.width == 0xFFFFFFFF || capabilities.currentExtent.height == 0xFFFFFFFF)
        return std::make_pair(capabilities.maxImageExtent.width, capabilities.maxImageExtent.height);

    return std::make_pair(capabilities.currentExtent.width, capabilities.currentExtent.height);
}

std::pair<std::uint32_t, std::uint32_t> surface::size(renderer& renderer) const
{
    VkSurfaceCapabilitiesKHR capabilities{};
    if(auto result{vkGetPhysicalDeviceSurfaceCapabilitiesKHR(underlying_cast<VkPhysicalDevice>(renderer), m_surface, &capabilities)}; result != VK_SUCCESS)
        throw vulkan::error{result};

    if(capabilities.currentExtent.width == 0xFFFFFFFF || capabilities.currentExtent.height == 0xFFFFFFFF)
        return std::make_pair(capabilities.maxImageExtent.width, capabilities.maxImageExtent.height);

    return std::make_pair(capabilities.currentExtent.width, capabilities.currentExtent.height);
}

}
