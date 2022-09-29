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

#ifndef TEPHRA_TEXTURE_HPP_INCLUDED
#define TEPHRA_TEXTURE_HPP_INCLUDED

#include "config.hpp"

#include <optional>

#include "vulkan/vulkan.hpp"
#include "vulkan/memory.hpp"

#include "enumerations.hpp"

namespace tph
{

class device;
class command_buffer;
class render_target;
class swapchain;

enum class texture_usage : std::uint32_t
{
    transfer_source = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    transfer_destination = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    sampled = VK_IMAGE_USAGE_SAMPLED_BIT,
    storage = VK_IMAGE_USAGE_STORAGE_BIT,
    color_attachment = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    depth_stencil_attachment = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    transient_attachment = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
    input_attachment = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
};

struct texture_info
{
    texture_format format{};
    texture_usage usage{};
    std::uint32_t mip_levels{1};
    std::uint32_t array_layers{1};
    tph::sample_count sample_count{tph::sample_count::msaa_x1};
};

inline texture_aspect aspect_from_format(texture_format format) noexcept
{
    switch(format)
    {
        case texture_format::d16_unorm:          [[fallthrough]];
        case texture_format::x8_d24_unorm_pack:  [[fallthrough]];
        case texture_format::d32_sfloat:         return texture_aspect::depth;
        case texture_format::s8_uint:            return texture_aspect::stencil;
        case texture_format::d16_unorm_s8_uint:  [[fallthrough]];
        case texture_format::d24_unorm_s8_uint:  [[fallthrough]];
        case texture_format::d32_sfloat_s8_uint: return texture_aspect::depth | texture_aspect::stencil;
        default:                                 return texture_aspect::color;
    }
}

struct cubemap_t{};
inline constexpr cubemap_t cubemap{};

class TEPHRA_API texture
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

    friend class swapchain;

public:
    constexpr texture() = default;
    explicit texture(device& device, std::uint32_t width, const texture_info& info);
    explicit texture(device& device, std::uint32_t width, std::uint32_t height, const texture_info& info);
    explicit texture(device& device, std::uint32_t width, std::uint32_t height, std::uint32_t depth, const texture_info& info);
    explicit texture(device& device, cubemap_t, std::uint32_t size, const texture_info& info);

    explicit texture(vulkan::image image, vulkan::memory_heap_chunk memory,
                     std::uint32_t dimensions, std::uint32_t width, std::uint32_t height, std::uint32_t depth, bool is_cubemap,
                     texture_format format, std::uint32_t mip_levels, std::uint32_t array_layers, tph::sample_count sample_count) noexcept
    :m_image{std::move(image)}
    ,m_memory{std::move(memory)}
    ,m_dimensions{dimensions}
    ,m_width{width}
    ,m_height{height}
    ,m_depth{depth}
    ,m_cubemap{is_cubemap}
    ,m_format{format}
    ,m_aspect{aspect_from_format(format)}
    ,m_mip_levels{mip_levels}
    ,m_array_layers{array_layers}
    ,m_sample_count{sample_count}
    {

    }

    ~texture() = default;
    texture(const texture&) = delete;
    texture& operator=(const texture&) = delete;
    texture(texture&& other) noexcept = default;
    texture& operator=(texture&& other) noexcept = default;

    vulkan::device_context context() const noexcept
    {
        return m_image.context();
    }

    std::uint32_t dimensions() const noexcept
    {
        return m_dimensions;
    }

    std::uint32_t width() const noexcept
    {
        return m_width;
    }

    std::uint32_t height() const noexcept
    {
        return m_height;
    }

    std::uint32_t depth() const noexcept
    {
        return m_depth;
    }

    bool is_cubemap() const noexcept
    {
        return m_cubemap;
    }

    texture_format format() const noexcept
    {
        return m_format;
    }

    texture_aspect aspect() const noexcept
    {
        return m_aspect;
    }

    std::uint32_t mip_levels() const noexcept
    {
        return m_mip_levels;
    }

    std::uint32_t array_layers() const noexcept
    {
        return m_array_layers;
    }

    tph::sample_count sample_count() const noexcept
    {
        return m_sample_count;
    }

private:
    vulkan::image m_image{};
    vulkan::memory_heap_chunk m_memory{};
    std::uint32_t m_dimensions{};
    std::uint32_t m_width{};
    std::uint32_t m_height{};
    std::uint32_t m_depth{};
    bool m_cubemap{};
    texture_format m_format{texture_format::undefined};
    texture_aspect m_aspect{};
    std::uint32_t m_mip_levels{};
    std::uint32_t m_array_layers{};
    tph::sample_count m_sample_count{};
};

TEPHRA_API void set_object_name(device& device, const texture& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const texture& texture) noexcept
{
    return texture.m_image.device();
}

template<>
inline VkImage underlying_cast(const texture& texture) noexcept
{
     return texture.m_image;
}

struct component_mapping
{
    component_swizzle r{component_swizzle::identity};
    component_swizzle g{component_swizzle::identity};
    component_swizzle b{component_swizzle::identity};
    component_swizzle a{component_swizzle::identity};
};

class TEPHRA_API texture_view
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    static constexpr component_mapping identity_mapping{};

public:
    texture_view() = default;
    texture_view(device& device, const texture& texture, const component_mapping& mapping = identity_mapping);
    texture_view(device& device, const texture& texture, const texture_subresource_range& subresource_range, const component_mapping& mapping = identity_mapping);

    texture_view(vulkan::image_view image_view) noexcept
    :m_image_view{std::move(image_view)}
    {

    }

    ~texture_view() = default;
    texture_view(const texture_view&) = delete;
    texture_view& operator=(const texture_view&) = delete;
    texture_view(texture_view&& other) noexcept = default;
    texture_view& operator=(texture_view&& other) noexcept = default;

    vulkan::device_context context() const noexcept
    {
        return m_image_view.context();
    }

private:
    vulkan::image_view m_image_view{};
};

TEPHRA_API void set_object_name(device& device, const texture_view& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const texture_view& view) noexcept
{
    return view.m_image_view.device();
}

template<>
inline VkImageView underlying_cast(const texture_view& view) noexcept
{
     return view.m_image_view;
}

enum class address_mode : std::uint32_t
{
    repeat = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    mirrored =  VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
    clamp_to_edge = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    clamp_to_border = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
    miror_clamp_to_edge = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
};

enum class border_color : std::uint32_t
{
    transparent = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
    black = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
    white = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
};

enum class mipmap_mode : std::uint32_t
{
    nearest = VK_SAMPLER_MIPMAP_MODE_NEAREST,
    linear = VK_SAMPLER_MIPMAP_MODE_LINEAR,
};

struct sampler_info
{
    filter mag_filter{filter::nearest};
    filter min_filter{filter::nearest};
    tph::mipmap_mode mipmap_mode{tph::mipmap_mode::nearest};
    tph::address_mode address_mode{tph::address_mode::clamp_to_border};
    tph::border_color border_color{tph::border_color::transparent};
    float mip_lod_bias{0.0f};
    std::uint32_t anisotropy_level{1};
    bool compare{false};
    tph::compare_op compare_op{tph::compare_op::always};
    float min_lod{0.0f};
    float max_lod{0.0f};
    bool unnormalized_coordinates{false};
};

class TEPHRA_API sampler
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    sampler() = default;
    sampler(device& device, const sampler_info& info);

    sampler(vulkan::sampler sampler) noexcept
    :m_sampler{std::move(sampler)}
    {

    }

    ~sampler() = default;
    sampler(const sampler&) = delete;
    sampler& operator=(const sampler&) = delete;
    sampler(sampler&& other) noexcept = default;
    sampler& operator=(sampler&& other) noexcept = default;

    vulkan::device_context context() const noexcept
    {
        return m_sampler.context();
    }

private:
    vulkan::sampler m_sampler{};
};

TEPHRA_API void set_object_name(device& device, const sampler& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const sampler& sampler) noexcept
{
    return sampler.m_sampler.device();
}

template<>
inline VkSampler underlying_cast(const sampler& sampler) noexcept
{
     return sampler.m_sampler;
}


}

template<> struct tph::enable_enum_operations<tph::texture_usage> {static constexpr bool value{true};};

#endif
