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
    giant_memory_heaps = 0x08,
    standalone_transfer_queue = 0x10,
    standalone_compute_queue = 0x20
};

enum class queue : std::size_t
{
    graphics = 0,
    present = 1,
    transfer = 2,
    compute = 3,
    count = 4
};

class TEPHRA_API renderer
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    struct transfer_granularity
    {
        std::uint32_t width{1};
        std::uint32_t height{1};
        std::uint32_t depth{1};
    };

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

    bool is_same_queue(queue first, queue second) const noexcept
    {
        return queue_family_index(first) == queue_family_index(second);
    }

    const transfer_granularity& transfer_queue_granularity() const noexcept
    {
        return m_transfer_queue_granularity;
    }

private:
    VkPhysicalDevice m_physical_device{};
    vulkan::device m_device{};
    std::array<std::uint32_t, static_cast<std::size_t>(queue::count)> m_queue_families{};
    std::array<VkQueue, static_cast<std::size_t>(queue::count)> m_queues{};
    transfer_granularity m_transfer_queue_granularity{};
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

template<> struct tph::enable_enum_operations<tph::renderer_options> {static constexpr bool value{true};};

#endif
