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

enum class renderer_options : std::uint32_t
{
    none = 0x00,
    tiny_memory_heaps = 0x01,
    small_memory_heaps = 0x02,
    large_memory_heaps = 0x04,
    giant_memory_heaps = 0x08,
    standalone_transfer_queue = 0x10,
    standalone_compute_queue = 0x20
};

enum class renderer_layer : std::uint32_t
{
    none = 0x00,
    validation = 0x01
};

enum class renderer_extension : std::uint32_t
{
    none = 0x00,
    swapchain = 0x01
};

class TEPHRA_API renderer
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    using queue_families_t = std::array<std::uint32_t, static_cast<std::size_t>(queue::count)>;
    using queues_t = std::array<VkQueue, static_cast<std::size_t>(queue::count)>;

public:
    struct transfer_granularity
    {
        std::uint32_t width{1};
        std::uint32_t height{1};
        std::uint32_t depth{1};
    };

public:
    constexpr renderer() = default;
    explicit renderer(const physical_device& physical_device, renderer_layer layers, renderer_extension extensions, const physical_device_features& enabled_features = physical_device_features{}, renderer_options options = renderer_options::none);

    explicit renderer(const physical_device& physical_device, vulkan::device device, renderer_layer layers, renderer_extension extensions,
                      const queue_families_t& queue_families, const queues_t& queues, const vulkan::memory_allocator::heap_sizes& sizes);

    ~renderer() = default;
    renderer(const renderer&) = delete;
    renderer& operator=(const renderer&) = delete;
    renderer(renderer&& other) noexcept = default;
    renderer& operator=(renderer&& other) noexcept = default;

    void wait();

    renderer_layer enabled_layers() const noexcept
    {
        return m_layers;
    }

    renderer_extension enabled_extensions() const noexcept
    {
        return m_extensions;
    }

    std::uint32_t queue_family(queue queue) const noexcept
    {
        return m_queue_families[static_cast<std::size_t>(queue)];
    }

    const queue_families_t& queue_families() const noexcept
    {
        return m_queue_families;
    }

    bool is_same_queue(queue first, queue second) const noexcept
    {
        return queue_family(first) == queue_family(second);
    }

    const transfer_granularity& transfer_queue_granularity() const noexcept
    {
        return m_transfer_queue_granularity;
    }

    vulkan::memory_allocator& allocator() noexcept
    {
        return *m_allocator;
    }

    const vulkan::memory_allocator& allocator() const noexcept
    {
        return *m_allocator;
    }

private:
    VkPhysicalDevice m_physical_device{};
    vulkan::device m_device{};
    renderer_layer m_layers{};
    renderer_extension m_extensions{};
    queue_families_t m_queue_families{};
    queues_t m_queues{};
    transfer_granularity m_transfer_queue_granularity{};
    std::unique_ptr<vulkan::memory_allocator> m_allocator{};
};

TEPHRA_API void set_object_name(renderer& renderer, const std::string& name);
TEPHRA_API void begin_queue_label(renderer& renderer, queue queue, const std::string& name, float red = 0.0f, float green = 0.0f, float blue = 0.0f, float alpha = 0.0f) noexcept;
TEPHRA_API void end_queue_label(renderer& renderer, queue queue) noexcept;
TEPHRA_API void insert_queue_label(renderer& renderer, queue queue, const std::string& name, float red = 0.0f, float green = 0.0f, float blue = 0.0f, float alpha = 0.0f) noexcept;

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
template<> struct tph::enable_enum_operations<tph::renderer_layer> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::renderer_extension> {static constexpr bool value{true};};

#endif
