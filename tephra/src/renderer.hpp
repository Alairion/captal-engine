#ifndef TEPHRA_RENDERER_HPP_INCLUDED
#define TEPHRA_RENDERER_HPP_INCLUDED

#include "config.hpp"

#include <memory>

#include "vulkan/vulkan.hpp"
#include "vulkan/memory.hpp"

#include "hardware.hpp"

namespace tph
{

class application;
class command_buffer;

enum class renderer_options
{
    none = 0x00,
    tiny_memory_heaps = 0x01,
    small_memory_heaps = 0x02,
    large_memory_heaps = 0x04,
    giant_memory_heaps = 0x08
};

template<> struct enable_enum_operations<renderer_options> {static constexpr bool value{true};};

enum class queue : std::size_t
{
    graphics = 0,
    present = 1,
};

class renderer
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr renderer() = default;
    renderer(application& app, const physical_device& physical_device, renderer_options options = renderer_options::none, const physical_device_features& enabled_features = physical_device_features{});
    ~renderer() = default;

    renderer(const renderer&) = delete;
    renderer& operator=(const renderer&) = delete;

    renderer(renderer&& other) noexcept = default;
    renderer& operator=(renderer&& other) noexcept = default;

    void wait();
    void free_memory();

    vulkan::memory_allocator& allocator() noexcept
    {
        return *m_allocator;
    }

    const vulkan::memory_allocator& allocator() const noexcept
    {
        return *m_allocator;
    }

    std::uint32_t queue_family_index(queue queue) const noexcept
    {
        return m_queue_families[static_cast<std::size_t>(queue)];
    }

private:
    VkPhysicalDevice m_physical_device{};
    vulkan::device m_device{};
    std::array<std::uint32_t, 2> m_queue_families{};
    std::array<VkQueue, 2> m_queues{};
    std::unique_ptr<vulkan::memory_allocator> m_allocator{};
};

template<>
inline VkPhysicalDevice underlying_cast(const renderer& renderer) noexcept
{
    return renderer.m_physical_device;
}

template<>
inline VkDevice underlying_cast(const renderer& renderer) noexcept
{
    return renderer.m_device;
}

template<>
inline VkQueue underlying_cast(const renderer& renderer, const queue& index) noexcept
{
    return renderer.m_queues[static_cast<std::size_t>(index)];
}

}

#endif
