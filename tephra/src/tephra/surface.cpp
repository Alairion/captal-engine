#include "surface.hpp"

#include <captal_foundation/stack_allocator.hpp>

#include "vulkan/vulkan_functions.hpp"

#include "application.hpp"
#include "hardware.hpp"
#include "renderer.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

static surface_capabilities convert_capabilities(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    VkSurfaceCapabilitiesKHR capabilities{};
    if(auto result{vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities)}; result != VK_SUCCESS)
        throw vulkan::error{result};

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

static std::vector<texture_format> convert_formats(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    std::uint32_t count{};
    if(auto result{vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count, nullptr)}; result != VK_SUCCESS)
        throw vulkan::error{result};

    stack_memory_pool<512> pool{};
    auto formats{make_stack_vector<VkSurfaceFormatKHR>(pool)};
    formats.resize(count);
    if(auto result{vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count, std::data(formats))}; result != VK_SUCCESS)
        throw vulkan::error{result};

    std::vector<texture_format> output{};
    output.reserve(std::size(formats));
    for(auto format : formats)
    {
        output.emplace_back(static_cast<texture_format>(format.format));
    }

    return output;
}

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

surface_capabilities surface::capabilities(const physical_device& physical_device) const
{
    return convert_capabilities(underlying_cast<VkPhysicalDevice>(physical_device), m_surface);
}

surface_capabilities surface::capabilities(const renderer& renderer) const
{
    return convert_capabilities(underlying_cast<VkPhysicalDevice>(renderer), m_surface);
}

std::vector<texture_format> surface::formats(const physical_device& physical_device) const
{
    return convert_formats(underlying_cast<VkPhysicalDevice>(physical_device), m_surface);
}

std::vector<texture_format> surface::formats(const renderer& renderer) const
{
    return convert_formats(underlying_cast<VkPhysicalDevice>(renderer), m_surface);
}

void set_object_name(renderer& renderer, const surface& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_SURFACE_KHR;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkSurfaceKHR>(object));
    info.pObjectName = std::data(name);

    if(auto result{vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(renderer), &info)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

}
