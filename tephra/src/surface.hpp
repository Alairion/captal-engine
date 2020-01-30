#ifndef TEPHRA_SURFACE_HPP_INCLUDED
#define TEPHRA_SURFACE_HPP_INCLUDED

#include "config.hpp"

#include "vulkan/vulkan.hpp"

namespace tph
{

class application;
class physical_device;
class renderer;

class surface
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr surface() = default;

#ifdef TPH_PLATFORM_ANDROID
    surface(application& application, const vulkan::android_surface_info& info);
#endif

#ifdef TPH_PLATFORM_IOS
    surface(application& application, const vulkan::ios_surface_info& info);
#endif

#ifdef TPH_PLATFORM_WIN32
    surface(application& application, const vulkan::win32_surface_info& info);
#endif

#ifdef TPH_PLATFORM_MACOS
    surface(application& application, const vulkan::macos_surface_info& info);
#endif

#ifdef TPH_PLATFORM_XLIB
    surface(application& application, const vulkan::xlib_surface_info& info);
#endif

#ifdef TPH_PLATFORM_XCB
    surface(application& application, const vulkan::xcb_surface_info& info);
#endif

#ifdef TPH_PLATFORM_WAYLAND
    surface(application& application, const vulkan::wayland_surface_info& info);
#endif

    explicit surface(vulkan::surface native_surface) noexcept
    :m_surface{std::move(native_surface)}
    {

    }

    ~surface() = default;

    surface(const surface&) = delete;
    surface& operator=(const surface&) = delete;

    surface(surface&&) noexcept = default;
    surface& operator=(surface&&) noexcept = default;

    std::pair<std::uint32_t, std::uint32_t> size(const physical_device& physical_device) const;
    std::pair<std::uint32_t, std::uint32_t> size(renderer& renderer) const;

private:
    vulkan::surface m_surface{};
};

template<>
inline VkSurfaceKHR underlying_cast(const surface& surf) noexcept
{
    return surf.m_surface;
}

}

#endif
