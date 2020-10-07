#ifndef TEPHRA_TEXTURE_HPP_INCLUDED
#define TEPHRA_TEXTURE_HPP_INCLUDED

#include "config.hpp"

#include <optional>

#include "vulkan/vulkan.hpp"
#include "vulkan/memory.hpp"

#include "enumerations.hpp"

namespace tph
{

class renderer;
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

struct component_mapping
{
    component_swizzle r{component_swizzle::identity};
    component_swizzle g{component_swizzle::identity};
    component_swizzle b{component_swizzle::identity};
    component_swizzle a{component_swizzle::identity};
};

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

struct sampling_options
{
    filter magnification_filter{filter::nearest};
    filter minification_filter{filter::nearest};
    address_mode address_mode{address_mode::clamp_to_border};
    border_color border_color{border_color::transparent};
    std::uint32_t anisotropy_level{1};
    bool compare{false};
    compare_op compare_op{compare_op::always};
    bool normalized_coordinates{true};
};

struct texture_info
{
    texture_format format{};
    texture_usage usage{};
    component_mapping components{};
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

class TEPHRA_API texture
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

    friend class render_target;
    friend class swapchain;

public:
    constexpr texture() = default;
    explicit texture(renderer& renderer, std::uint32_t width, const texture_info& info);
    explicit texture(renderer& renderer, std::uint32_t width, const texture_info& info, const sampling_options& options);
    explicit texture(renderer& renderer, std::uint32_t width, std::uint32_t height, const texture_info& info);
    explicit texture(renderer& renderer, std::uint32_t width, std::uint32_t height, const texture_info& info, const sampling_options& options);
    explicit texture(renderer& renderer, std::uint32_t width, std::uint32_t height, std::uint32_t depth, const texture_info& info);
    explicit texture(renderer& renderer, std::uint32_t width, std::uint32_t height, std::uint32_t depth, const texture_info& info, const sampling_options& options);

    explicit texture(vulkan::image image, vulkan::memory_heap_chunk memory, vulkan::image_view image_view, vulkan::sampler sampler,
                     texture_format format, std::uint32_t width, std::uint32_t height, std::uint32_t depth) noexcept
    :m_image{std::move(image)}
    ,m_memory{std::move(memory)}
    ,m_image_view{std::move(image_view)}
    ,m_sampler{std::move(sampler)}
    ,m_format{format}
    ,m_aspect{aspect_from_format(format)}
    ,m_width{width}
    ,m_height{height}
    ,m_depth{depth}
    {

    }

    ~texture() = default;
    texture(const texture&) = delete;
    texture& operator=(const texture&) = delete;
    texture(texture&& other) noexcept = default;
    texture& operator=(texture&& other) noexcept = default;

    texture_format format() const noexcept
    {
        return m_format;
    }

    texture_aspect aspect() const noexcept
    {
        return m_aspect;
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

    void transition(command_buffer& command_buffer, resource_access source_access, resource_access destination_access, pipeline_stage source_stage, pipeline_stage destination_stage, texture_layout current_layout, texture_layout next_layout) noexcept;

private:
    vulkan::image m_image{};
    vulkan::memory_heap_chunk m_memory{};
    vulkan::image_view m_image_view{};
    vulkan::sampler m_sampler{};
    texture_format m_format{texture_format::undefined};
    texture_aspect m_aspect{};
    std::uint32_t m_width{};
    std::uint32_t m_height{};
    std::uint32_t m_depth{};
};

TEPHRA_API void set_object_name(renderer& renderer, const texture& object, const std::string& name);

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

template<>
inline VkImageView underlying_cast(const texture& texture) noexcept
{
     return texture.m_image_view;
}

template<>
inline VkSampler underlying_cast(const texture& texture) noexcept
{
     return texture.m_sampler;
}

}

template<> struct tph::enable_enum_operations<tph::texture_usage> {static constexpr bool value{true};};

#endif
