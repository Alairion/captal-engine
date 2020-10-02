#ifndef TEPHRA_HARDWARE_HPP_INCLUDED
#define TEPHRA_HARDWARE_HPP_INCLUDED

#include "config.hpp"

#include <array>
#include <string>

#include "vulkan/vulkan.hpp"

#include "enumerations.hpp"
#include "surface.hpp"

namespace tph
{

class application;
class surface;

enum class physical_device_type : std::uint32_t
{
    unknown = VK_PHYSICAL_DEVICE_TYPE_OTHER,
    integrated = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
    discrete = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
    virtualised = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
    cpu = VK_PHYSICAL_DEVICE_TYPE_CPU,
};

struct physical_device_properties
{
    physical_device_type type{};
    tph::version api_version{};
    std::string name{};
    std::array<std::uint8_t, 16> uuid{};
};

struct physical_device_features
{
    bool wide_lines{};
    bool large_points{};
    bool sample_shading{};
    bool fragment_stores_and_atomics{};
};

struct physical_device_limits
{
    sample_count sample_count{};
    std::uint32_t max_texture_size{};
    std::uint32_t max_push_constant_size{};
    std::uint64_t min_uniform_buffer_alignment{};
    std::uint64_t min_storage_buffer_alignment{};
};

struct physical_device_memory_properties
{
    std::uint64_t device_shared{};
    std::uint64_t device_local{};
    std::uint64_t host_shared{};
};

struct physical_device_surface_capabilities
{
    std::uint32_t min_image_count{};
    std::uint32_t max_image_count{};
    std::vector<present_mode> present_modes{};
};

struct physical_device_format_properties
{
    format_feature linear{};
    format_feature optimal{};
    format_feature buffer{};
};

class physical_device;

namespace vulkan
{

physical_device make_physical_device(VkPhysicalDevice device) noexcept;

}

class TEPHRA_API physical_device
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

    friend physical_device vulkan::make_physical_device(VkPhysicalDevice device) noexcept;

public:
    constexpr physical_device() = default;
    ~physical_device() = default;
    physical_device(const physical_device&) = delete;
    physical_device& operator=(const physical_device&) = delete;
    physical_device(physical_device&&) noexcept = default;
    physical_device& operator=(physical_device&&) noexcept = default;

    bool support_presentation(const surface& surface) const;
    physical_device_surface_capabilities surface_capabilities(const surface& surface) const;
    physical_device_format_properties format_properties(texture_format format) const noexcept;
    bool support_texture_format(texture_format format, format_feature features) const;

    const physical_device_properties& properties() const noexcept
    {
        return m_properties;
    }

    const physical_device_features& features() const noexcept
    {
        return m_features;
    }

    const physical_device_limits& limits() const noexcept
    {
        return m_limits;
    }

    const physical_device_memory_properties memory_properties() const noexcept
    {
        return m_memory_properties;
    }

private:
    VkPhysicalDevice m_physical_device{};
    physical_device_properties m_properties{};
    physical_device_features m_features{};
    physical_device_limits m_limits{};
    physical_device_memory_properties m_memory_properties{};
};

template<>
inline VkPhysicalDevice underlying_cast(const physical_device& physical_device) noexcept
{
    return physical_device.m_physical_device;
}

TEPHRA_API bool default_physical_device_comparator(const physical_device& left, const physical_device& right) noexcept;

}

#endif
