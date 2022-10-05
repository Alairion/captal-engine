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

#ifndef TEPHRA_DEVICE_HPP_INCLUDED
#define TEPHRA_DEVICE_HPP_INCLUDED

#include "config.hpp"

#include <memory>

#include "vulkan/vulkan.hpp"
#include "vulkan/memory.hpp"

#include "application.hpp"
#include "hardware.hpp"

namespace tph
{

class application;
class command_buffer;

enum class device_options : std::uint32_t
{
    none = 0x00,
    tiny_memory_heaps = 0x01,
    small_memory_heaps = 0x02,
    large_memory_heaps = 0x04,
    giant_memory_heaps = 0x08,
    standalone_transfer_queue = 0x10,
    standalone_compute_queue = 0x20
};

enum class device_layer : std::uint32_t
{
    none = 0x00,
    validation = 0x01
};

enum class device_extension : std::uint32_t
{
    none = 0x00,
    swapchain = 0x01
};

class TEPHRA_API device
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
    constexpr device() = default;
    explicit device(application& app, const physical_device& phydev, device_layer layers, device_extension extensions, const physical_device_features& enabled_features = physical_device_features{}, device_options options = device_options::none);

    explicit device(application& app, const physical_device& phydev, vulkan::device dev, device_layer layers, device_extension extensions,
                    const queue_families_t& queue_families, const queues_t& queues, const vulkan::memory_allocator::heap_sizes& sizes);

    ~device() = default;
    device(const device&) = delete;
    device& operator=(const device&) = delete;
    device(device&& other) noexcept = default;
    device& operator=(device&& other) noexcept = default;

    void wait();

    vulkan::device_context context() const noexcept
    {
        return m_device.context();
    }

    const vulkan::functions::device_level_functions* operator->() const noexcept
    {
        return m_device.operator->();
    }

    device_layer enabled_layers() const noexcept
    {
        return m_layers;
    }

    device_extension enabled_extensions() const noexcept
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
    device_layer m_layers{};
    device_extension m_extensions{};
    queue_families_t m_queue_families{};
    queues_t m_queues{};
    transfer_granularity m_transfer_queue_granularity{};
    std::unique_ptr<vulkan::memory_allocator> m_allocator{};
};

TEPHRA_API void set_object_name(device& dev, const std::string& name);
TEPHRA_API void begin_queue_label(device& dev, queue q, const std::string& name, float red = 0.0f, float green = 0.0f, float blue = 0.0f, float alpha = 0.0f) noexcept;
TEPHRA_API void end_queue_label(device& dev, queue q) noexcept;
TEPHRA_API void insert_queue_label(device& dev, queue q, const std::string& name, float red = 0.0f, float green = 0.0f, float blue = 0.0f, float alpha = 0.0f) noexcept;

template<>
inline VkPhysicalDevice underlying_cast(const device& dev) noexcept
{
    return dev.m_physical_device;
}

template<>
inline VkDevice underlying_cast(const device& dev) noexcept
{
    return dev.m_device;
}

template<>
inline VkQueue underlying_cast(const device& dev, const queue& q) noexcept
{
    return dev.m_queues[static_cast<std::size_t>(q)];
}

}

template<> struct tph::enable_enum_operations<tph::device_options> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::device_layer> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::device_extension> {static constexpr bool value{true};};

#endif
