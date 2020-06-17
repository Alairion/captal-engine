#include "hardware.hpp"

#include <algorithm>
#include <unordered_map>

#include "vulkan/vulkan_functions.hpp"

#include "application.hpp"
#include "surface.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

namespace vulkan
{

static physical_device_properties make_properties(VkPhysicalDevice physical_device) noexcept
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physical_device, &properties);

    physical_device_properties output{};
    output.name = properties.deviceName;
    output.type = static_cast<physical_device_type>(properties.deviceType);
    std::copy(std::cbegin(properties.pipelineCacheUUID), std::cend(properties.pipelineCacheUUID), std::begin(output.uuid));

    return output;
}

static physical_device_features make_features(VkPhysicalDevice physical_device) noexcept
{
    VkPhysicalDeviceFeatures features{};
    vkGetPhysicalDeviceFeatures(physical_device, &features);

    physical_device_features output{};

    output.wide_lines = features.wideLines;
    output.large_points = features.largePoints;
    output.sample_shading = features.sampleRateShading;

    return output;
}

static physical_device_limits make_limits(VkPhysicalDevice physical_device) noexcept
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physical_device, &properties);

    physical_device_limits output{};

    output.sample_count = static_cast<sample_count>(properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts);
    output.max_texture_size = properties.limits.maxImageDimension2D;
    output.max_push_constant_size = properties.limits.maxPushConstantsSize;
    output.min_uniform_buffer_alignment = properties.limits.minUniformBufferOffsetAlignment;

    return output;
}

static physical_device_memory_properties make_memory_properties(VkPhysicalDevice physical_device) noexcept
{
    VkPhysicalDeviceMemoryProperties properties{};
    vkGetPhysicalDeviceMemoryProperties(physical_device, &properties);

    std::vector<VkMemoryPropertyFlags> heaps_flags{};
    heaps_flags.resize(properties.memoryHeapCount);

    for(std::uint32_t i{}; i < properties.memoryTypeCount; ++i)
        heaps_flags[properties.memoryTypes[i].heapIndex] |= properties.memoryTypes[i].propertyFlags;

    physical_device_memory_properties output{};

    for(std::size_t i{}; i < std::size(heaps_flags); ++i)
    {
        if((heaps_flags[i] & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        {
            output.device_shared += properties.memoryHeaps[i].size;
        }
        else if(heaps_flags[i] & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            output.device_local += properties.memoryHeaps[i].size;
        }
        else if(heaps_flags[i] & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            output.host_shared += properties.memoryHeaps[i].size;
        }
    }

    return output;
}

physical_device make_physical_device(VkPhysicalDevice vk_physical_device) noexcept
{
    physical_device output{};

    output.m_physical_device = vk_physical_device;
    output.m_properties = make_properties(vk_physical_device);
    output.m_features = make_features(vk_physical_device);
    output.m_limits = make_limits(vk_physical_device);
    output.m_memory_properties = make_memory_properties(vk_physical_device);

    return output;
}

}

bool physical_device::support_presentation(const surface& surface) const
{
    std::uint32_t count{};
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &count, nullptr);

    for(std::uint32_t i{}; i < count; ++i)
    {
        VkBool32 support{};
        if(auto result{vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_device, i, underlying_cast<VkSurfaceKHR>(surface), &support)}; result != VK_SUCCESS)
            throw vulkan::error{result};

        if(support)
        {
            return true;
        }
    }

    return false;
}

physical_device_surface_capabilities physical_device::surface_capabilities(const surface& surface) const
{
    std::uint32_t count{};
    if(auto result{vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, underlying_cast<VkSurfaceKHR>(surface), &count, nullptr)}; result != VK_SUCCESS)
        throw vulkan::error{result};

    std::vector<VkPresentModeKHR> present_modes{};
    present_modes.resize(count);
    if(auto result{vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, underlying_cast<VkSurfaceKHR>(surface), &count, std::data(present_modes))}; result != VK_SUCCESS)
        throw vulkan::error{result};

    VkSurfaceCapabilitiesKHR capabilities{};
    if(auto result{vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, underlying_cast<VkSurfaceKHR>(surface), &capabilities)}; result != VK_SUCCESS)
        throw vulkan::error{result};

    physical_device_surface_capabilities output{};

    output.min_image_count = capabilities.minImageCount;
    output.max_image_count = capabilities.maxImageCount == 0 ? std::numeric_limits<std::uint32_t>::max() : capabilities.maxImageCount;

    output.present_modes.reserve(std::size(present_modes));
    for(auto mode : present_modes)
    {
        output.present_modes.emplace_back(static_cast<present_mode>(mode));
    }

    return output;
}

physical_device_format_properties physical_device::format_properties(texture_format format) const noexcept
{
    VkFormatProperties properties{};
    vkGetPhysicalDeviceFormatProperties(m_physical_device, static_cast<VkFormat>(format), &properties);

    physical_device_format_properties output{};
    output.linear = static_cast<format_feature>(properties.linearTilingFeatures);
    output.optimal = static_cast<format_feature>(properties.optimalTilingFeatures);
    output.buffer = static_cast<format_feature>(properties.bufferFeatures);

    return output;
}

bool physical_device::support_texture_format(texture_format format, format_feature features) const
{
    VkFormatProperties properties{};
    vkGetPhysicalDeviceFormatProperties(m_physical_device, static_cast<VkFormat>(format), &properties);

    return (static_cast<format_feature>(properties.optimalTilingFeatures) & features) == features;
}

bool default_physical_device_comparator(const physical_device& left, const physical_device& right) noexcept
{
    const auto physical_device_score = [](const physical_device& device) noexcept -> std::int64_t
    {
        std::int64_t score{};

        score += device.memory_properties().device_shared / 1048576;
        score += device.memory_properties().device_local / 1048576;
        score += device.memory_properties().host_shared / 1048576;

        if(device.properties().type == physical_device_type::discrete)
        {
            score *= 2;
        }

        return score;
    };

    return physical_device_score(left) > physical_device_score(right);
}

}
