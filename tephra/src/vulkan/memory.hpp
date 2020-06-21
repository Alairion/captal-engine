#ifndef TEPHRA_VULKAN_MEMORY_HPP_INCLUDED
#define TEPHRA_VULKAN_MEMORY_HPP_INCLUDED

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <forward_list>
#include <optional>
#include <mutex>

#include "vulkan.hpp"

namespace tph::vulkan
{

class memory_heap;

enum class memory_resource_type : std::uint32_t
{
    linear = 0,
    non_linear = 1,
};

class TEPHRA_API memory_heap_chunk
{
    friend class memory_heap;

private:
    memory_heap_chunk(memory_heap* parent, std::uint64_t offset, std::uint64_t size) noexcept;

public:
    constexpr memory_heap_chunk() = default;
    ~memory_heap_chunk();

    memory_heap_chunk(const memory_heap_chunk&) = delete;
    memory_heap_chunk& operator=(const memory_heap_chunk&) = delete;

    memory_heap_chunk(memory_heap_chunk&& other) noexcept;
    memory_heap_chunk& operator=(memory_heap_chunk&& other) noexcept;

    void bind(VkBuffer buffer);
    void bind(VkImage image);

    void* map();
    const void* map() const;
    void flush();
    void invalidate();
    void unmap() const noexcept;

    template<typename T>
    T* map()
    {
        return reinterpret_cast<T*>(map());
    }

    memory_heap& heap() noexcept
    {
        return *m_parent;
    }

    const memory_heap& heap() const noexcept
    {
        return *m_parent;
    }

    std::uint64_t offset() const noexcept
    {
        return m_offset;
    }

    std::uint64_t size() const noexcept
    {
        return m_size;
    }

private:
    memory_heap* m_parent{};
    std::uint64_t m_offset{};
    std::uint64_t m_size{};
    mutable bool m_mapped{};
};

class TEPHRA_API memory_heap
{
    friend class memory_heap_chunk;

private:
    struct range
    {
        std::uint64_t offset{};
        std::uint64_t size{};
        memory_resource_type type{};
    };

public:
    memory_heap(VkDevice device, std::uint32_t type, std::uint64_t size, std::uint64_t granularity, std::uint64_t non_coherent_atom_size, bool coherent);
    ~memory_heap();

    memory_heap(const memory_heap&) = delete;
    memory_heap& operator=(const memory_heap&) = delete;

    memory_heap(memory_heap&&) noexcept = delete;
    memory_heap& operator=(memory_heap&&) noexcept = delete;

    memory_heap_chunk allocate(memory_resource_type ressource_type, std::uint64_t size, std::uint64_t alignment);
    std::optional<memory_heap_chunk> try_allocate(memory_resource_type ressource_type, std::uint64_t size, std::uint64_t alignment);

    void* map();
    void flush(std::uint64_t offset, std::uint64_t size);
    void invalidate(std::uint64_t offset, std::uint64_t size);
    void unmap() noexcept;

    std::uint64_t free_space() const noexcept
    {
        return m_free_space;
    }

    std::uint32_t type() const noexcept
    {
        return m_type;
    }

    std::uint64_t size() const noexcept
    {
        return m_size;
    }

    std::size_t allocation_count() const noexcept
    {
        return std::size(m_ranges);
    }

    bool coherent() const noexcept
    {
        return m_coherent;
    }

    operator VkDevice() const noexcept
    {
        return m_device;
    }

    operator VkDeviceMemory() const noexcept
    {
        return m_memory;
    }

private:
    void unregister_chunk(const memory_heap_chunk& chunk) noexcept;

private:
    VkDevice m_device{};
    device_memory m_memory{};
    std::uint32_t m_type{};
    std::uint64_t m_size{};
    std::uint64_t m_free_space{};
    std::uint64_t m_granularity{};
    std::uint64_t m_non_coherent_atom_size{};
    bool m_coherent{};
    std::uint64_t m_map_count{};
    void* m_map{};
    std::vector<range> m_ranges{};
    std::mutex m_mutex{};
};

class TEPHRA_API memory_allocator
{
public:
    struct heap_sizes
    {
        std::uint64_t host_shared{};
        std::uint64_t device_local{};
        std::uint64_t device_shared{};
    };

public:
    constexpr memory_allocator() = default;
    memory_allocator(VkPhysicalDevice physical_device, VkDevice device, const heap_sizes& sizes);
    ~memory_allocator() = default;

    memory_allocator(const memory_allocator&) = delete;
    memory_allocator& operator=(const memory_allocator&) = delete;
    memory_allocator(memory_allocator&&) noexcept = delete;
    memory_allocator& operator=(memory_allocator&&) noexcept = delete;

    memory_heap_chunk allocate(const VkMemoryRequirements& requirements, memory_resource_type ressource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal = 0);
    memory_heap_chunk allocate(VkBuffer buffer, memory_resource_type ressource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal = 0);
    memory_heap_chunk allocate(VkImage image, memory_resource_type ressource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal = 0);

    memory_heap_chunk allocate_bound(VkBuffer buffer, memory_resource_type ressource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal = 0);
    memory_heap_chunk allocate_bound(VkImage image, memory_resource_type ressource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal = 0);

    void clean();

    heap_sizes heap_count();
    heap_sizes used_memory();
    heap_sizes allocated_memory();
    heap_sizes available_memory();

    heap_sizes default_heap_sizes() const noexcept
    {
        return m_sizes;
    }

    operator VkPhysicalDevice() const noexcept
    {
        return m_physical_device;
    }

    operator VkDevice() const noexcept
    {
        return m_device;
    }

private:
    std::uint64_t default_heap_size(std::uint32_t type) const;

private:
    VkPhysicalDevice m_physical_device{};
    VkDevice m_device{};
    VkPhysicalDeviceMemoryProperties m_memory_properties{};
    std::vector<VkMemoryPropertyFlags> m_heaps_flags{};
    heap_sizes m_sizes{};
    std::uint64_t m_granularity{};
    std::uint64_t m_non_coherent_atom_size{};
    std::forward_list<memory_heap> m_heaps{};
    std::mutex m_mutex{};
};

}

#endif
