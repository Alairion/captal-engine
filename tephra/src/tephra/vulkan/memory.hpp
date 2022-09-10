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

#ifndef TEPHRA_VULKAN_MEMORY_HPP_INCLUDED
#define TEPHRA_VULKAN_MEMORY_HPP_INCLUDED

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <memory>
#include <optional>
#include <mutex>
#include <atomic>
#include <variant>

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
    explicit memory_heap_chunk(memory_heap* parent, std::uint64_t offset, std::uint64_t size) noexcept;

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
    struct memory_range
    {
        std::uint64_t offset{};
        std::uint64_t size{};
        memory_resource_type type{};
    };

    struct non_dedicated_heap
    {
        non_dedicated_heap(std::uint64_t _granularity, std::uint64_t _non_coherent_atom_size)
        :granularity{_granularity}
        ,non_coherent_atom_size{_non_coherent_atom_size}
        {

        }

        std::uint64_t granularity{};
        std::uint64_t non_coherent_atom_size{};
        std::uint64_t map_count{};
        std::vector<memory_range> ranges{};
        mutable std::mutex mutex{};
    };

    struct dedicated_heap
    {
        std::optional<memory_range> range{};
    };

public:
    explicit memory_heap(VkDevice device, std::uint32_t type, std::uint64_t size, std::uint64_t granularity, std::uint64_t non_coherent_atom_size, bool coherent);
    explicit memory_heap(VkDevice device, VkImage image, std::uint32_t type, std::uint64_t size);
    explicit memory_heap(VkDevice device, VkBuffer buffer, std::uint32_t type, std::uint64_t size);

    ~memory_heap();
    memory_heap(const memory_heap&) = delete;
    memory_heap& operator=(const memory_heap&) = delete;
    memory_heap(memory_heap&&) noexcept = delete;
    memory_heap& operator=(memory_heap&&) noexcept = delete;

    memory_heap_chunk allocate(memory_resource_type resource_type, std::uint64_t size, std::uint64_t alignment);
    memory_heap_chunk allocate_dedicated(std::uint64_t size);
    memory_heap_chunk allocate_pseudo_dedicated(memory_resource_type resource_type, std::uint64_t size);
    std::optional<memory_heap_chunk> try_allocate(memory_resource_type resource_type, std::uint64_t size, std::uint64_t alignment);

    std::uint32_t type() const noexcept
    {
        return m_type;
    }

    std::uint64_t size() const noexcept
    {
        return m_size;
    }

    std::uint64_t free_space() const noexcept
    {
        return m_free_space;
    }

    std::size_t allocation_count() const noexcept
    {
        return m_allocation_count;
    }

    bool coherent() const noexcept
    {
        return m_coherent;
    }

    bool dedicated() const noexcept
    {
        return m_heap.index() == 1;
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
    void* map();
    void flush(std::uint64_t offset, std::uint64_t size);
    void invalidate(std::uint64_t offset, std::uint64_t size);
    void unmap() noexcept;
    void unregister_chunk(const memory_heap_chunk& chunk) noexcept;

private:
    VkDevice m_device{};
    device_memory m_memory{};
    std::uint32_t m_type{};
    std::uint64_t m_size{};
    std::atomic<std::uint64_t> m_free_space{};
    std::atomic<std::size_t> m_allocation_count{};
    bool m_coherent{};
    void* m_map{};
    std::variant<non_dedicated_heap, dedicated_heap> m_heap;
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
    explicit memory_allocator(VkPhysicalDevice physical_device, VkDevice device, const heap_sizes& sizes);
    ~memory_allocator() = default;

    memory_allocator(const memory_allocator&) = delete;
    memory_allocator& operator=(const memory_allocator&) = delete;
    memory_allocator(memory_allocator&&) noexcept = delete;
    memory_allocator& operator=(memory_allocator&&) noexcept = delete;

    memory_heap_chunk allocate(const VkMemoryRequirements& requirements, memory_resource_type resource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal = 0);
    memory_heap_chunk allocate(VkBuffer buffer, memory_resource_type resource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal = 0);
    memory_heap_chunk allocate(VkImage image, memory_resource_type resource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal = 0);

    memory_heap_chunk allocate_bound(VkBuffer buffer, memory_resource_type resource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal = 0);
    memory_heap_chunk allocate_bound(VkImage image, memory_resource_type resource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal = 0);

    void clean();
    void clean_dedicated();

    heap_sizes heap_count() const;
    heap_sizes allocation_count() const;
    heap_sizes allocated_memory() const;
    heap_sizes used_memory() const;

    heap_sizes dedicated_heap_count() const;
    heap_sizes dedicated_allocation_count() const;
    heap_sizes dedicated_allocated_memory() const;
    heap_sizes dedicated_used_memory() const;

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
    tph::version m_version{};
    VkPhysicalDeviceMemoryProperties m_memory_properties{};
    std::vector<VkMemoryPropertyFlags> m_heaps_flags{};
    heap_sizes m_sizes{};
    std::uint64_t m_granularity{};
    std::uint64_t m_non_coherent_atom_size{};
    std::vector<std::unique_ptr<memory_heap>> m_heaps{};
    mutable std::mutex m_mutex{};
};

constexpr memory_allocator::heap_sizes operator+(const memory_allocator::heap_sizes& left, const memory_allocator::heap_sizes& right) noexcept
{
    memory_allocator::heap_sizes output{left};

    output.host_shared += right.host_shared;
    output.device_local += right.device_local;
    output.device_shared += right.device_shared;

    return output;
}

constexpr memory_allocator::heap_sizes operator-(const memory_allocator::heap_sizes& left, const memory_allocator::heap_sizes& right) noexcept
{
    memory_allocator::heap_sizes output{left};

    output.host_shared -= right.host_shared;
    output.device_local -= right.device_local;
    output.device_shared -= right.device_shared;

    return output;
}

}

#endif
