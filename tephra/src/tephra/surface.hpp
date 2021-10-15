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

#ifndef TEPHRA_SURFACE_HPP_INCLUDED
#define TEPHRA_SURFACE_HPP_INCLUDED

#include "config.hpp"

#include "vulkan/vulkan.hpp"

#include "texture.hpp"

namespace tph
{

class application;
class physical_device;
class renderer;

enum class surface_transform : std::uint32_t
{
    identity = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
    rotate_90 = VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR,
    rotate_180 = VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR,
    rotate_270 = VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR,
    horizontal_mirror = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR,
    horizontal_mirror_90 = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR,
    horizontal_mirror_180 = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR,
    horizontal_mirror_270 = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR,
    inherit = VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR,
};

enum class surface_composite : std::uint32_t
{
    opaque = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    pre_multiplied = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
    post_multiplied = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
    inherit = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
};

struct surface_capabilities
{
    std::uint32_t min_image_count{};
    std::uint32_t max_image_count{};
    std::uint32_t current_width{};
    std::uint32_t current_height{};
    std::uint32_t min_width{};
    std::uint32_t min_height{};
    std::uint32_t max_width{};
    std::uint32_t max_height{};
    std::uint32_t max_array_layers{};
    surface_transform supported_transforms{};
    surface_transform current_transform{};
    surface_composite supported_composites{};
    texture_usage supported_usages{};
};

class TEPHRA_API surface
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr surface() = default;

#ifdef TPH_PLATFORM_ANDROID
    explicit surface(application& application, const vulkan::android_surface_info& info);
#endif

#ifdef TPH_PLATFORM_IOS
    explicit surface(application& application, const vulkan::ios_surface_info& info);
#endif

#ifdef TPH_PLATFORM_WIN32
    explicit surface(application& application, const vulkan::win32_surface_info& info);
#endif

#ifdef TPH_PLATFORM_MACOS
    explicit surface(application& application, const vulkan::macos_surface_info& info);
#endif

#ifdef TPH_PLATFORM_XLIB
    explicit surface(application& application, const vulkan::xlib_surface_info& info);
#endif

#ifdef TPH_PLATFORM_XCB
    explicit surface(application& application, const vulkan::xcb_surface_info& info);
#endif

#ifdef TPH_PLATFORM_WAYLAND
    explicit surface(application& application, const vulkan::wayland_surface_info& info);
#endif

    explicit surface(vulkan::surface surface) noexcept
    :m_surface{std::move(surface)}
    {

    }

    ~surface() = default;
    surface(const surface&) = delete;
    surface& operator=(const surface&) = delete;
    surface(surface&&) noexcept = default;
    surface& operator=(surface&&) noexcept = default;

    surface_capabilities capabilities(const physical_device& physical_device) const;
    surface_capabilities capabilities(const renderer& renderer) const;
    std::vector<texture_format> formats(const physical_device& physical_device) const;
    std::vector<texture_format> formats(const renderer& renderer) const;

private:
    vulkan::surface m_surface{};
};

TEPHRA_API void set_object_name(renderer& renderer, const surface& object, const std::string& name);

template<>
inline VkInstance underlying_cast(const surface& surface) noexcept
{
    return surface.m_surface.instance();
}

template<>
inline VkSurfaceKHR underlying_cast(const surface& surf) noexcept
{
    return surf.m_surface;
}

}

template<> struct tph::enable_enum_operations<tph::surface_transform> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::surface_composite> {static constexpr bool value{true};};

#endif
