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

enum class texture_usage : std::uint32_t
{
    transfer_source = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    transfer_destination = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    sampled = VK_IMAGE_USAGE_SAMPLED_BIT,
    storage = VK_IMAGE_USAGE_STORAGE_BIT,
    color_attachment = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
};

template<> struct enable_enum_operations<texture_usage> {static constexpr bool value{true};};

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

class texture
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

    friend class render_target;

public:
    using size_type = std::uint32_t;

public:
    constexpr texture() = default;
    texture(renderer& renderer, size_type width, size_type height, texture_usage usage);
    texture(renderer& renderer, size_type width, size_type height, const sampling_options& options, texture_usage usage);
    texture(renderer& renderer, size_type width, size_type height, size_type depth, texture_usage usage);
    texture(renderer& renderer, size_type width, size_type height, size_type depth, const sampling_options& options, texture_usage usage);

    ~texture() = default;
    texture(const texture&) = delete;
    texture& operator=(const texture&) = delete;
    texture(texture&& other) noexcept = default;
    texture& operator=(texture&& other) noexcept = default;

    image_layout layout() const noexcept
    {
        return m_layout;
    }

    size_type width() const noexcept
    {
        return m_width;
    }

    size_type height() const noexcept
    {
        return m_height;
    }

    size_type depth() const noexcept
    {
        return m_depth;
    }

    void transition(command_buffer& command_buffer, resource_access source_access, resource_access destination_access, pipeline_stage source_stage, pipeline_stage destination_stage, image_layout layout);
    void transition(command_buffer& command_buffer, resource_access source_access, resource_access destination_access, pipeline_stage source_stage, pipeline_stage destination_stage, image_layout current_layout, image_layout next_layout);

private:
    vulkan::image m_image{};
    vulkan::memory_heap_chunk m_memory{};
    vulkan::image_view m_image_view{};
    vulkan::sampler m_sampler{};
    image_layout m_layout{image_layout::undefined};
    size_type m_width{};
    size_type m_height{};
    size_type m_depth{};
};

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

#endif
