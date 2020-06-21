#include "memory.hpp"

#include <cassert>
#include <algorithm>
#include <functional>

#include "vulkan_functions.hpp"
#include "helper.hpp"

using namespace tph::vulkan::functions;

namespace tph::vulkan
{

memory_heap_chunk::memory_heap_chunk(memory_heap* parent, std::uint64_t offset, std::uint64_t size) noexcept
:m_parent{parent}
,m_offset{offset}
,m_size{size}
{

}

memory_heap_chunk::~memory_heap_chunk()
{
    if(m_parent)
    {
        if(m_mapped)
        {
            unmap();
        }

        m_parent->unregister_chunk(*this);
    }
}

memory_heap_chunk::memory_heap_chunk(memory_heap_chunk&& other) noexcept
:m_parent{std::exchange(other.m_parent, nullptr)}
,m_offset{other.m_offset}
,m_size{other.m_size}
,m_mapped{other.m_mapped}
{

}

memory_heap_chunk& memory_heap_chunk::operator=(memory_heap_chunk&& other) noexcept
{
    m_parent = std::exchange(other.m_parent, m_parent);
    m_offset = std::exchange(other.m_offset, m_offset);
    m_size = std::exchange(other.m_size, m_size);
    m_mapped = std::exchange(other.m_mapped, m_mapped);

    return *this;
}

void memory_heap_chunk::bind(VkBuffer buffer)
{
    assert(m_parent && "tph::vulkan::memory_heap_chunk::bind called with an invalid memory_heap_chunk.");

    if(vkBindBufferMemory(*m_parent, buffer, *m_parent, m_offset) != VK_SUCCESS)
        throw std::runtime_error{"Can not bind buffer memory."};
}

void memory_heap_chunk::bind(VkImage image)
{
    assert(m_parent && "tph::vulkan::memory_heap_chunk::bind called with an invalid memory_heap_chunk.");

    if(vkBindImageMemory(*m_parent, image, *m_parent, m_offset) != VK_SUCCESS)
        throw std::runtime_error{"Can not bind image memory."};
}

void* memory_heap_chunk::map()
{
    assert(m_parent && "tph::vulkan::memory_heap_chunk::map called with an invalid memory_heap_chunk.");
    assert(!m_mapped && "tph::vulkan::memory_heap_chunk::map called on an mapped memory_heap_chunk.");

    void* const out{reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(m_parent->map()) + m_offset)};

    m_mapped = true;

    return out;
}

const void* memory_heap_chunk::map() const
{
    assert(m_parent && "tph::vulkan::memory_heap_chunk::map called with an invalid memory_heap_chunk.");
    assert(!m_mapped && "tph::vulkan::memory_heap_chunk::map called on an mapped memory_heap_chunk.");

    const void* const out{reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(m_parent->map()) + m_offset)};

    m_mapped = true;

    return out;
}

void memory_heap_chunk::flush()
{
    assert(m_parent && "tph::vulkan::memory_heap_chunk::flush called with an invalid memory_heap_chunk.");
    assert(m_mapped && "tph::vulkan::memory_heap_chunk::flush called on an unmapped memory_heap_chunk.");

    m_parent->flush(m_offset, m_size);
}

void memory_heap_chunk::invalidate()
{
    assert(m_parent && "tph::vulkan::memory_heap_chunk::invalidate called with an invalid memory_heap_chunk.");
    assert(m_mapped && "tph::vulkan::memory_heap_chunk::invalidate called on an unmapped memory_heap_chunk.");

    m_parent->invalidate(m_offset, m_size);
}

void memory_heap_chunk::unmap() const noexcept
{
    assert(m_parent && "tph::vulkan::memory_heap_chunk::unmap called with an invalid memory_heap_chunk.");
    assert(m_mapped && "tph::vulkan::memory_heap_chunk::unmap called on an unmapped memory_heap_chunk.");

    m_parent->unmap();
    m_mapped = false;
}

memory_heap::memory_heap(VkDevice device, std::uint32_t type, std::uint64_t size, uint64_t granularity, uint64_t non_coherent_atom_size, bool coherent)
:m_device{device}
,m_memory{device, type, size}
,m_type{type}
,m_size{size}
,m_free_space{size}
,m_granularity{granularity}
,m_non_coherent_atom_size{non_coherent_atom_size}
,m_coherent{coherent}
{
    m_ranges.reserve(128);
}

memory_heap::~memory_heap()
{
    assert(allocation_count() == 0 && "tph::vulkan::memory_heap destroyed with non-freed allocations");
}

memory_heap_chunk memory_heap::allocate(memory_resource_type ressource_type, std::uint64_t size, std::uint64_t alignment)
{
    auto chunk{try_allocate(ressource_type, size, alignment)};

    assert(chunk.has_value() && "tph::vulkan::memory_heap::allocate failed.");

    return std::move(chunk.value());
}

std::optional<memory_heap_chunk> memory_heap::try_allocate(memory_resource_type ressource_type, std::uint64_t size, std::uint64_t alignment)
{
    std::lock_guard lock{m_mutex};

    //Push it at the begginning if the heap is empty
    if(std::empty(m_ranges) && size <= m_size)
    {
        m_ranges.emplace_back(range{0, size, ressource_type});
        m_free_space -= size;
        return std::make_optional(memory_heap_chunk{this, 0, size});
    }

    //Try to push it at the end
    const auto try_push = [this, ressource_type, size, alignment]
    {
        const range& last{m_ranges.back()};

        if(last.type == ressource_type)
        {
            const std::uint64_t end{align_up(last.offset + last.size, alignment)};

            if(m_size - end >= size)
            {
                m_ranges.emplace_back(range{end, size, ressource_type});
                return std::cend(m_ranges) - 1;
            }
        }
        else
        {
            //Take care of the linear/non-linear granularity
            const std::uint64_t end{align_up(last.offset + last.size, std::max(alignment, m_granularity))};

            if(m_size - end >= size)
            {
                m_ranges.emplace_back(range{end, size, ressource_type});
                return std::cend(m_ranges) - 1;
            }
        }

        return std::cend(m_ranges);
    };

    if(const auto it{try_push()}; it != std::cend(m_ranges))
    {
        m_free_space -= it->size;
        return std::make_optional(memory_heap_chunk{this, it->offset, it->size});
    }

    //Try to insert it at a suitable place
    const auto try_insert = [this, ressource_type, size, alignment]() -> std::vector<range>::const_iterator
    {
        for(auto it{std::cbegin(m_ranges)}; it != std::cend(m_ranges) - 1; ++it)
        {
            const auto next{it + 1};

            if(it->type == ressource_type && next->type == ressource_type)
            {
                const std::uint64_t end{align_up(it->offset + it->size, alignment)};

                if(static_cast<std::int64_t>(next->offset) - static_cast<std::int64_t>(end) >= static_cast<std::int64_t>(size))
                {
                    return m_ranges.insert(next, range{end, size, ressource_type});
                }
            }
            else
            {
                std::uint64_t begin{};
                if(it->type != ressource_type)
                {
                    begin = align_up(it->offset + it->size, std::max(alignment, m_granularity));
                }
                else
                {
                    begin = align_up(it->offset + it->size, alignment);
                }

                std::uint64_t end{};
                if(next->type != ressource_type)
                {
                    end = align_down(next->offset, std::max(alignment, m_granularity));
                }
                else
                {
                    end = align_down(next->offset, alignment);
                }

                if(static_cast<std::int64_t>(end) - static_cast<std::int64_t>(begin) >= static_cast<std::int64_t>(size))
                {
                    return m_ranges.insert(next, range{begin, size, ressource_type});
                }
            }
        }

        return std::cend(m_ranges);
    };

    if(const auto it{try_insert()}; it != std::cend(m_ranges))
    {
        m_free_space -= it->size;
        return std::make_optional(memory_heap_chunk{this, it->offset, it->size});
    }

    return std::nullopt;
}

void* memory_heap::map()
{
    std::lock_guard lock{m_mutex};

    if(!m_map)
    {
        if(vkMapMemory(m_device, m_memory, 0, VK_WHOLE_SIZE, 0, &m_map) != VK_SUCCESS)
            throw std::runtime_error{"Can not map memory."};
    }

    ++m_map_count;

    return m_map;
}

void memory_heap::flush(std::uint64_t offset, std::uint64_t size)
{
    std::lock_guard lock{m_mutex};

    assert(m_map && "tph::vulkan::memory_heap::flush called on an unmapped memory_heap.");

    const std::uint64_t aligned_offset{align_down(offset, m_non_coherent_atom_size)};
    const std::uint64_t aligned_size{align_up((offset - aligned_offset) + size, m_non_coherent_atom_size)};

    VkMappedMemoryRange range{};
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.memory = m_memory;
    range.offset = aligned_offset;
    range.size = aligned_size;

    if(vkFlushMappedMemoryRanges(m_device, 1, &range) != VK_SUCCESS)
        throw std::runtime_error{"Can not flush memory."};
}

void memory_heap::invalidate(std::uint64_t offset, std::uint64_t size)
{
    std::lock_guard lock{m_mutex};

    assert(m_map && "tph::vulkan::memory_heap::invalidate called on an unmapped memory_heap.");

    const std::uint64_t aligned_offset{align_down(offset, m_non_coherent_atom_size)};
    const std::uint64_t aligned_size{align_up((offset - aligned_offset) + size, m_non_coherent_atom_size)};

    VkMappedMemoryRange range{};
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.memory = m_memory;
    range.offset = aligned_offset;
    range.size = aligned_size;

    if(vkInvalidateMappedMemoryRanges(m_device, 1, &range) != VK_SUCCESS)
        throw std::runtime_error{"Can not flush memory."};
}

void memory_heap::unmap() noexcept
{
    std::lock_guard lock{m_mutex};

    assert(m_map && "tph::vulkan::memory_heap::unmap called on an unmapped memory_heap.");

    --m_map_count;

    if(m_map_count == 0)
    {
        vkUnmapMemory(m_device, m_memory);
        m_map = nullptr;
    }
}

void memory_heap::unregister_chunk(const memory_heap_chunk& chunk) noexcept
{
    std::lock_guard lock{m_mutex};

    const auto predicate = [](const range& range, const memory_heap_chunk& chunk)
    {
        return range.offset < chunk.m_offset;
    };

    const auto it{std::lower_bound(std::cbegin(m_ranges), std::cend(m_ranges), chunk, predicate)};
    assert(it != std::cend(m_ranges) && "Bad memory heap chunk.");

    m_free_space += it->size;
    m_ranges.erase(it);
}

memory_allocator::memory_allocator(VkPhysicalDevice physical_device, VkDevice device, const heap_sizes& sizes)
:m_physical_device{physical_device}
,m_device{device}
,m_sizes{sizes}
{
    vkGetPhysicalDeviceMemoryProperties(m_physical_device, &m_memory_properties);

    m_heaps_flags.resize(m_memory_properties.memoryHeapCount);
    for(std::uint32_t i{}; i < m_memory_properties.memoryTypeCount; ++i)
    {
        m_heaps_flags[m_memory_properties.memoryTypes[i].heapIndex] |= m_memory_properties.memoryTypes[i].propertyFlags;
    }

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_physical_device, &properties);

    m_granularity = properties.limits.bufferImageGranularity;
    m_non_coherent_atom_size = properties.limits.nonCoherentAtomSize;
}

memory_heap_chunk memory_allocator::allocate(const VkMemoryRequirements& requirements, memory_resource_type ressource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal)
{
    std::lock_guard lock{m_mutex};

    const std::uint32_t memory_type{find_memory_type(m_memory_properties, requirements.memoryTypeBits, minimal, optimal)};
    const std::uint64_t default_size{default_heap_size(memory_type)};
    const bool coherent{(m_memory_properties.memoryTypes[memory_type].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0};

    if(requirements.size > default_size)
    {
        auto& heap{m_heaps.emplace_front(m_device, memory_type, requirements.size, m_granularity, m_non_coherent_atom_size, coherent)};

        return heap.allocate(ressource_type, requirements.size, requirements.alignment);
    }

    if(!std::empty(m_heaps))
    {
        std::vector<std::reference_wrapper<memory_heap>> candidates{};
        for(memory_heap& heap : m_heaps)
        {
            if(heap.type() == memory_type && heap.free_space() > align_up(requirements.size, m_granularity))
            {
                candidates.emplace_back(std::ref(heap));
            }
        }

        if(!std::empty(candidates))
        {
            const auto predicate = [](const memory_heap& left, const memory_heap& right)
            {
                return left.free_space() < right.free_space();
            };

            std::sort(std::begin(candidates), std::end(candidates), predicate);

            for(memory_heap& heap : candidates)
            {
                auto chunk{heap.try_allocate(ressource_type, requirements.size, requirements.alignment)};

                if(chunk)
                {
                    return std::move(chunk.value());
                }
            }
        }
    }

    auto& heap{m_heaps.emplace_front(m_device, memory_type, default_size, m_granularity, m_non_coherent_atom_size, coherent)};

    return heap.allocate(ressource_type, requirements.size, requirements.alignment);
}

memory_heap_chunk memory_allocator::allocate(VkBuffer buffer, memory_resource_type ressource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal)
{
    VkMemoryRequirements requirements{};
    vkGetBufferMemoryRequirements(m_device, buffer, &requirements);

    return allocate(requirements, ressource_type, minimal, optimal);
}

memory_heap_chunk memory_allocator::allocate(VkImage image, memory_resource_type ressource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal)
{
    VkMemoryRequirements requirements{};
    vkGetImageMemoryRequirements(m_device, image, &requirements);

    return allocate(requirements, ressource_type, minimal, optimal);
}

memory_heap_chunk memory_allocator::allocate_bound(VkBuffer buffer, memory_resource_type ressource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal)
{
    memory_heap_chunk chunk{allocate(buffer, ressource_type, minimal, optimal)};
    chunk.bind(buffer);

    return chunk;
}

memory_heap_chunk memory_allocator::allocate_bound(VkImage image, memory_resource_type ressource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal)
{
    memory_heap_chunk chunk{allocate(image, ressource_type, minimal, optimal)};
    chunk.bind(image);

    return chunk;
}

void memory_allocator::clean()
{
    std::lock_guard lock{m_mutex};

    m_heaps.remove_if([](const memory_heap& heap)
    {
        return heap.allocation_count() == 0;
    });
}

memory_allocator::heap_sizes memory_allocator::heap_count()
{
    std::lock_guard lock{m_mutex};

    heap_sizes output{};

    for(const auto& heap : m_heaps)
    {
        const auto flags{m_heaps_flags[m_memory_properties.memoryTypes[heap.type()].heapIndex]};

        if((flags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        {
            ++output.device_shared;
        }
        else if(flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            ++output.device_local;
        }
        else if(flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            ++output.host_shared;
        }
    }

    return output;
}

memory_allocator::heap_sizes memory_allocator::used_memory()
{
    std::lock_guard lock{m_mutex};

    heap_sizes output{};

    for(auto& heap : m_heaps)
    {
        const auto flags{m_heaps_flags[m_memory_properties.memoryTypes[heap.type()].heapIndex]};

        if((flags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        {
            output.device_shared += (heap.size() - heap.free_space());
        }
        else if(flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            output.device_local += (heap.size() - heap.free_space());
        }
        else if(flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            output.host_shared += (heap.size() - heap.free_space());
        }
    }

    return output;
}

memory_allocator::heap_sizes memory_allocator::allocated_memory()
{
    std::lock_guard lock{m_mutex};

    heap_sizes output{};

    for(auto& heap : m_heaps)
    {
        const auto flags{m_heaps_flags[m_memory_properties.memoryTypes[heap.type()].heapIndex]};

        if((flags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        {
            output.device_shared += heap.size();
        }
        else if(flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            output.device_local += heap.size();
        }
        else if(flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            output.host_shared += heap.size();
        }
    }

    return output;
}

memory_allocator::heap_sizes memory_allocator::available_memory()
{
    std::lock_guard lock{m_mutex};

    heap_sizes output{};

    for(auto& heap : m_heaps)
    {
        const auto flags{m_heaps_flags[m_memory_properties.memoryTypes[heap.type()].heapIndex]};

        if((flags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        {
            output.device_shared += heap.free_space();
        }
        else if(flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            output.device_local += heap.free_space();
        }
        else if(flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            output.host_shared += heap.free_space();
        }
    }

    return output;
}

std::uint64_t memory_allocator::default_heap_size(std::uint32_t type) const
{
    const auto flags{m_heaps_flags[m_memory_properties.memoryTypes[type].heapIndex]};

    if((flags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
    {
        return m_sizes.device_shared;
    }
    else if(flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    {
        return m_sizes.device_local;
    }
    else if(flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        return m_sizes.host_shared;
    }

    throw std::runtime_error{"Wrong memory type."};
}

}
