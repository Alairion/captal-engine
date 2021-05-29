#include "texture.hpp"

#include <cassert>

#include "vulkan/vulkan_functions.hpp"
#include "vulkan/helper.hpp"

#include "renderer.hpp"
#include "commands.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

texture::texture(renderer& renderer, std::uint32_t width, const texture_info& info)
:m_dimensions{1}
,m_width{width}
,m_height{1}
,m_depth{1}
,m_format{info.format}
,m_aspect{aspect_from_format(m_format)}
,m_mip_levels{info.mip_levels}
,m_array_layers{info.array_layers}
,m_sample_count{info.sample_count}
{
    VkImageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.imageType = VK_IMAGE_TYPE_1D;
    create_info.extent = VkExtent3D{width, 1, 1};
    create_info.format = static_cast<VkFormat>(m_format);
    create_info.mipLevels = info.mip_levels;
    create_info.arrayLayers = info.array_layers;
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.usage = static_cast<VkImageUsageFlags>(info.usage);
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    m_image  = vulkan::image{underlying_cast<VkDevice>(renderer), create_info};
    m_memory = renderer.allocator().allocate_bound(m_image, vulkan::memory_resource_type::non_linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

texture::texture(renderer& renderer, std::uint32_t width, std::uint32_t height, const texture_info& info)
:m_dimensions{2}
,m_width{width}
,m_height{height}
,m_depth{1}
,m_format{info.format}
,m_aspect{aspect_from_format(m_format)}
,m_mip_levels{info.mip_levels}
,m_array_layers{info.array_layers}
,m_sample_count{info.sample_count}
{
    VkImageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.extent = VkExtent3D{width, height, 1};
    create_info.format = static_cast<VkFormat>(m_format);
    create_info.mipLevels = info.mip_levels;
    create_info.arrayLayers = info.array_layers;
    create_info.samples = static_cast<VkSampleCountFlagBits>(info.sample_count);
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.usage = static_cast<VkImageUsageFlags>(info.usage);
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    m_image  = vulkan::image{underlying_cast<VkDevice>(renderer), create_info};
    m_memory = renderer.allocator().allocate_bound(m_image, vulkan::memory_resource_type::non_linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

texture::texture(renderer& renderer, std::uint32_t width, std::uint32_t height, std::uint32_t depth, const texture_info& info)
:m_dimensions{3}
,m_width{width}
,m_height{height}
,m_depth{depth}
,m_format{info.format}
,m_aspect{aspect_from_format(m_format)}
,m_mip_levels{info.mip_levels}
,m_array_layers{info.array_layers}
,m_sample_count{info.sample_count}
{
    VkImageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.imageType = VK_IMAGE_TYPE_3D;
    create_info.extent = VkExtent3D{width, height, depth};
    create_info.format = static_cast<VkFormat>(m_format);
    create_info.mipLevels = info.mip_levels;
    create_info.arrayLayers = info.array_layers;
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.usage = static_cast<VkImageUsageFlags>(info.usage);
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    m_image  = vulkan::image{underlying_cast<VkDevice>(renderer), create_info};
    m_memory = renderer.allocator().allocate_bound(m_image, vulkan::memory_resource_type::non_linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

texture::texture(renderer& renderer, cubemap_t, std::uint32_t size, const texture_info& info)
:m_dimensions{2}
,m_width{size}
,m_height{size}
,m_depth{1}
,m_cubemap{true}
,m_format{info.format}
,m_aspect{aspect_from_format(m_format)}
,m_mip_levels{info.mip_levels}
,m_array_layers{info.array_layers * 6}
,m_sample_count{info.sample_count}
{
    VkImageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.extent = VkExtent3D{size, size, 1};
    create_info.format = static_cast<VkFormat>(m_format);
    create_info.mipLevels = info.mip_levels;
    create_info.arrayLayers = info.array_layers * 6;
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.usage = static_cast<VkImageUsageFlags>(info.usage);
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    m_image  = vulkan::image{underlying_cast<VkDevice>(renderer), create_info};
    m_memory = renderer.allocator().allocate_bound(m_image, vulkan::memory_resource_type::non_linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void set_object_name(renderer& renderer, const texture& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_IMAGE;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkImage>(object));
    info.pObjectName = std::data(name);

    if(auto result{vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(renderer), &info)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

static VkComponentMapping make_mapping(const component_mapping& mapping)
{
    return VkComponentMapping
    {
        static_cast<VkComponentSwizzle>(mapping.r),
        static_cast<VkComponentSwizzle>(mapping.g),
        static_cast<VkComponentSwizzle>(mapping.b),
        static_cast<VkComponentSwizzle>(mapping.a)
    };
}

texture_view::texture_view(renderer& renderer, const texture& texture, const component_mapping& mapping)
:texture_view{renderer, texture, texture_subresource_range{0, texture.mip_levels(), 0, texture.array_layers()}, mapping}
{

}

texture_view::texture_view(renderer& renderer, const texture& texture, const texture_subresource_range& subresource_range, const component_mapping& mapping)
{
    static constexpr std::array view_types{VK_IMAGE_VIEW_TYPE_1D, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_VIEW_TYPE_3D};
    static constexpr std::array array_view_types{VK_IMAGE_VIEW_TYPE_1D_ARRAY, VK_IMAGE_VIEW_TYPE_2D_ARRAY};

    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = underlying_cast<VkImage>(texture);

    if(!texture.is_cubemap())
    {
        if(subresource_range.array_layer_count == 1)
        {
            create_info.viewType = view_types[texture.dimensions() - 1];
        }
        else
        {
            assert(texture.dimensions() < 3 && "3D textures can not have multiple array layers.");

            create_info.viewType = array_view_types[texture.dimensions() - 1];
        }
    }
    else
    {
        if(subresource_range.array_layer_count == 6)
        {
            create_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        }
        else
        {
            create_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        }
    }

    create_info.format = static_cast<VkFormat>(texture.format());
    create_info.components = make_mapping(mapping);
    create_info.subresourceRange.aspectMask = static_cast<VkImageAspectFlags>(texture.aspect());
    create_info.subresourceRange.baseMipLevel = subresource_range.base_mip_level;
    create_info.subresourceRange.levelCount = subresource_range.mip_level_count;
    create_info.subresourceRange.baseArrayLayer = subresource_range.base_array_layer;
    create_info.subresourceRange.layerCount = subresource_range.array_layer_count;

    m_image_view = vulkan::image_view{underlying_cast<VkDevice>(renderer), create_info};
}

void set_object_name(renderer& renderer, const texture_view& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkImageView>(object));
    info.pObjectName = std::data(name);

    if(auto result{vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(renderer), &info)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

sampler::sampler(renderer& renderer, const sampler_info& info)
{
    VkSamplerCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    create_info.magFilter = static_cast<VkFilter>(info.mag_filter);
    create_info.minFilter = static_cast<VkFilter>(info.min_filter);
    create_info.mipmapMode = static_cast<VkSamplerMipmapMode>(info.mipmap_mode);
    create_info.addressModeU = static_cast<VkSamplerAddressMode>(info.address_mode);
    create_info.addressModeV = static_cast<VkSamplerAddressMode>(info.address_mode);
    create_info.addressModeW = static_cast<VkSamplerAddressMode>(info.address_mode);
    create_info.borderColor = static_cast<VkBorderColor>(info.border_color);
    create_info.mipLodBias = info.mip_lod_bias;
    create_info.anisotropyEnable = info.anisotropy_level > 1.0f ? VK_TRUE : VK_FALSE;
    create_info.maxAnisotropy = info.anisotropy_level;
    create_info.unnormalizedCoordinates = static_cast<VkBool32>(info.unnormalized_coordinates);
    create_info.compareEnable = static_cast<VkBool32>(info.compare);
    create_info.compareOp = static_cast<VkCompareOp>(info.compare_op);
    create_info.minLod = info.min_lod;
    create_info.maxLod = info.max_lod;

    m_sampler = vulkan::sampler{underlying_cast<VkDevice>(renderer), create_info};
}

void set_object_name(renderer& renderer, const sampler& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_SAMPLER;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkSampler>(object));
    info.pObjectName = std::data(name);

    if(auto result{vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(renderer), &info)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

}
