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

#include "memory.hpp"

#include <cassert>
#include <algorithm>
#include <functional>

#include <captal_foundation/stack_allocator.hpp>

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
    std::swap(other.m_parent, m_parent);
    std::swap(other.m_offset, m_offset);
    std::swap(other.m_size, m_size);
    std::swap(other.m_mapped, m_mapped);

    return *this;
}

void memory_heap_chunk::bind(VkBuffer buffer)
{
    assert(m_parent && "tph::vulkan::memory_heap_chunk::bind called with an invalid memory_heap_chunk.");

    if(auto result{vkBindBufferMemory(*m_parent, buffer, *m_parent, m_offset)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

void memory_heap_chunk::bind(VkImage image)
{
    assert(m_parent && "tph::vulkan::memory_heap_chunk::bind called with an invalid memory_heap_chunk.");

    if(auto result{vkBindImageMemory(*m_parent, image, *m_parent, m_offset)}; result != VK_SUCCESS)
        throw vulkan::error{result};
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

memory_heap::memory_heap(VkDevice device, std::uint32_t type, std::uint64_t size, std::uint64_t granularity, std::uint64_t non_coherent_atom_size, bool coherent)
:m_device{device}
,m_memory{device, type, size}
,m_type{type}
,m_size{size}
,m_free_space{size}
,m_coherent{coherent}
,m_heap{std::in_place_type<non_dedicated_heap>, granularity, non_coherent_atom_size}
{
    std::get<non_dedicated_heap>(m_heap).ranges.reserve(64);
}

memory_heap::memory_heap(VkDevice device, VkImage image, std::uint32_t type, std::uint64_t size)
:m_device{device}
,m_type{type}
,m_size{size}
,m_free_space{size}
,m_heap{std::in_place_type<dedicated_heap>}
{
    VkMemoryDedicatedAllocateInfo dedicated_info{};
    dedicated_info.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR;
    dedicated_info.image = image;

    VkMemoryAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    info.pNext = &dedicated_info;
    info.allocationSize = size;
    info.memoryTypeIndex = type;

    VkDeviceMemory memory{};
    if(auto result{vkAllocateMemory(device, &info, nullptr, &memory)}; result != VK_SUCCESS)
        throw vulkan::error{result};

    m_memory = vulkan::device_memory{device, memory};
}

memory_heap::memory_heap(VkDevice device, VkBuffer buffer, std::uint32_t type, std::uint64_t size)
:m_device{device}
,m_type{type}
,m_size{size}
,m_free_space{size}
,m_heap{std::in_place_type<dedicated_heap>}
{
    VkMemoryDedicatedAllocateInfo dedicated_info{};
    dedicated_info.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR;
    dedicated_info.buffer = buffer;

    VkMemoryAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    info.pNext = &dedicated_info;
    info.allocationSize = size;
    info.memoryTypeIndex = type;

    VkDeviceMemory memory{};
    if(auto result{vkAllocateMemory(device, &info, nullptr, &memory)}; result != VK_SUCCESS)
        throw vulkan::error{result};

    m_memory = vulkan::device_memory{device, memory};
}

memory_heap::~memory_heap()
{
    assert(allocation_count() == 0 && "tph::vulkan::memory_heap destroyed with non-freed allocations");
}

memory_heap_chunk memory_heap::allocate(memory_resource_type resource_type, std::uint64_t size, std::uint64_t alignment)
{
    auto chunk{try_allocate(resource_type, size, alignment)};

    if(!chunk.has_value())
    {
        throw vulkan::error{VK_ERROR_OUT_OF_DEVICE_MEMORY};
    }

    return std::move(*chunk);
}

memory_heap_chunk memory_heap::allocate_dedicated(std::uint64_t size)
{
    assert(dedicated() && "tph::vulkan::memory_heap::allocate_dedicated called on a non-dedicated memory heap");
    assert(!std::get<dedicated_heap>(m_heap).range.has_value() && "tph::vulkan::memory_heap::allocate_dedicated called more than once");

    std::get<dedicated_heap>(m_heap).range = range{0, size, memory_resource_type{}};

    m_free_space = 0;
    m_allocation_count = 1;

    return memory_heap_chunk{this, 0, size};
}

memory_heap_chunk memory_heap::allocate_pseudo_dedicated(memory_resource_type resource_type, std::uint64_t size)
{
    assert(!dedicated() && "tph::vulkan::memory_heap::allocate_pseudo_dedicated called on a dedicated memory heap");

    auto& heap{std::get<non_dedicated_heap>(m_heap)};

    heap.ranges.emplace_back(range{0, size, resource_type});

    m_free_space -= size;
    m_allocation_count = 1;

    return memory_heap_chunk{this, 0, size};
}

std::optional<memory_heap_chunk> memory_heap::try_allocate(memory_resource_type resource_type, std::uint64_t size, std::uint64_t alignment)
{
    assert(!dedicated() && "tph::vulkan::memory_heap::try_allocate called on a dedicated memory heap");

    auto& heap{std::get<non_dedicated_heap>(m_heap)};

    std::lock_guard lock{heap.mutex};

    //The whole algorithm is basically first-fit + coalescing

    //Push it at the begginning if the heap is empty
    if(std::empty(heap.ranges) && size <= m_size)
    {
        heap.ranges.emplace_back(range{0, size, resource_type});

        m_free_space -= size;
        m_allocation_count = 1;

        return std::make_optional(memory_heap_chunk{this, 0, size});
    }

    //Try to push it at the end
    const auto try_push = [this, &heap, resource_type, size, alignment]
    {
        const range& last{heap.ranges.back()};

        if(last.type == resource_type)
        {
            const std::uint64_t end{align_up(last.offset + last.size, alignment)};

            if(m_size - end >= size)
            {
                heap.ranges.emplace_back(range{end, size, resource_type});

                return std::cend(heap.ranges) - 1;
            }
        }
        else
        {
            //Take care of the linear/non-linear granularity
            const std::uint64_t end{align_up(last.offset + last.size, std::max(alignment, heap.granularity))};

            if(m_size - end >= size)
            {
                heap.ranges.emplace_back(range{end, size, resource_type});

                return std::cend(heap.ranges) - 1;
            }
        }

        return std::cend(heap.ranges);
    };

    if(const auto it{try_push()}; it != std::cend(heap.ranges))
    {
        m_free_space -= it->size;
        m_allocation_count += 1;

        return std::make_optional(memory_heap_chunk{this, it->offset, it->size});
    }

    //Try to insert it at a suitable place
    const auto try_insert = [&heap, resource_type, size, alignment]() -> std::vector<range>::const_iterator
    {
        for(auto it{std::cbegin(heap.ranges)}; it != std::cend(heap.ranges) - 1; ++it)
        {
            const auto next{it + 1};

            if(it->type == resource_type && next->type == resource_type)
            {
                const std::uint64_t end{align_up(it->offset + it->size, alignment)};

                if(static_cast<std::int64_t>(next->offset) - static_cast<std::int64_t>(end) >= static_cast<std::int64_t>(size))
                {
                    return heap.ranges.insert(next, range{end, size, resource_type});
                }
            }
            else
            {
                std::uint64_t begin{};
                if(it->type != resource_type)
                {
                    begin = align_up(it->offset + it->size, std::max(alignment, heap.granularity));
                }
                else
                {
                    begin = align_up(it->offset + it->size, alignment);
                }

                std::uint64_t end{};
                if(next->type != resource_type)
                {
                    end = align_down(next->offset, std::max(alignment, heap.granularity));
                }
                else
                {
                    end = align_down(next->offset, alignment);
                }

                if(static_cast<std::int64_t>(end) - static_cast<std::int64_t>(begin) >= static_cast<std::int64_t>(size))
                {
                    return heap.ranges.insert(next, range{begin, size, resource_type});
                }
            }
        }

        return std::cend(heap.ranges);
    };

    if(const auto it{try_insert()}; it != std::cend(heap.ranges))
    {
        m_free_space -= it->size;
        m_allocation_count += 1;

        return std::make_optional(memory_heap_chunk{this, it->offset, it->size});
    }

    return std::nullopt;
}

void* memory_heap::map()
{
    if(dedicated())
    {
        if(!m_map)
        {
            if(vkMapMemory(m_device, m_memory, 0, VK_WHOLE_SIZE, 0, &m_map) != VK_SUCCESS)
                throw std::runtime_error{"Can not map memory."};
        }
    }
    else
    {
        auto& heap{std::get<non_dedicated_heap>(m_heap)};

        std::lock_guard lock{heap.mutex};

        if(!m_map)
        {
            if(vkMapMemory(m_device, m_memory, 0, VK_WHOLE_SIZE, 0, &m_map) != VK_SUCCESS)
                throw std::runtime_error{"Can not map memory."};
        }

        ++heap.map_count;
    }

    return m_map;
}

void memory_heap::flush(std::uint64_t offset, std::uint64_t size)
{
    if(dedicated())
    {
        VkMappedMemoryRange range{};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory = m_memory;
        range.offset = 0;
        range.size = VK_WHOLE_SIZE;

        if(auto result{vkFlushMappedMemoryRanges(m_device, 1, &range)}; result != VK_SUCCESS)
            throw vulkan::error{result};
    }
    else
    {
        auto& heap{std::get<non_dedicated_heap>(m_heap)};

        std::lock_guard lock{heap.mutex};

        const std::uint64_t aligned_offset{align_down(offset, heap.non_coherent_atom_size)};
        const std::uint64_t aligned_size{align_up((offset - aligned_offset) + size, heap.non_coherent_atom_size)};

        VkMappedMemoryRange range{};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory = m_memory;
        range.offset = aligned_offset;
        range.size = aligned_size;

        if(auto result{vkFlushMappedMemoryRanges(m_device, 1, &range)}; result != VK_SUCCESS)
            throw vulkan::error{result};
    }

}

void memory_heap::invalidate(std::uint64_t offset, std::uint64_t size)
{
    if(dedicated())
    {
        VkMappedMemoryRange range{};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory = m_memory;
        range.offset = 0;
        range.size = VK_WHOLE_SIZE;

        if(auto result{vkInvalidateMappedMemoryRanges(m_device, 1, &range)}; result != VK_SUCCESS)
            throw vulkan::error{result};
    }
    else
    {
        auto& heap{std::get<non_dedicated_heap>(m_heap)};

        std::lock_guard lock{heap.mutex};

        const std::uint64_t aligned_offset{align_down(offset, heap.non_coherent_atom_size)};
        const std::uint64_t aligned_size{align_up((offset - aligned_offset) + size, heap.non_coherent_atom_size)};

        VkMappedMemoryRange range{};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory = m_memory;
        range.offset = aligned_offset;
        range.size = aligned_size;

        if(auto result{vkInvalidateMappedMemoryRanges(m_device, 1, &range)}; result != VK_SUCCESS)
            throw vulkan::error{result};
    }
}

void memory_heap::unmap() noexcept
{
    if(dedicated())
    {
        vkUnmapMemory(m_device, m_memory);
        m_map = nullptr;
    }
    else
    {
        auto& heap{std::get<non_dedicated_heap>(m_heap)};

        std::lock_guard lock{heap.mutex};

        --heap.map_count;

        if(heap.map_count == 0)
        {
            vkUnmapMemory(m_device, m_memory);
            m_map = nullptr;
        }
    }
}

void memory_heap::unregister_chunk(const memory_heap_chunk& chunk) noexcept
{
    if(dedicated())
    {
        auto& heap{std::get<dedicated_heap>(m_heap)};

        m_free_space = m_size;
        m_allocation_count = 0;

        heap.range.reset();
    }
    else
    {
        auto& heap{std::get<non_dedicated_heap>(m_heap)};

        std::lock_guard lock{heap.mutex};

        const auto predicate = [](const range& range, const memory_heap_chunk& chunk)
        {
            return range.offset < chunk.m_offset;
        };

        const auto it{std::lower_bound(std::cbegin(heap.ranges), std::cend(heap.ranges), chunk, predicate)};
        assert(it != std::cend(heap.ranges) && "Bad memory heap chunk.");

        m_free_space += it->size;
        m_allocation_count -= 1;

        heap.ranges.erase(it);
    }
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

    m_version.major = static_cast<std::uint16_t>(VK_VERSION_MAJOR(properties.apiVersion));
    m_version.minor = static_cast<std::uint16_t>(VK_VERSION_MINOR(properties.apiVersion));
    m_granularity = properties.limits.bufferImageGranularity;
    m_non_coherent_atom_size = properties.limits.nonCoherentAtomSize;
}

memory_heap_chunk memory_allocator::allocate(const VkMemoryRequirements& requirements, memory_resource_type resource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal)
{
    const std::uint32_t memory_type{find_memory_type(m_memory_properties, requirements.memoryTypeBits, minimal, optimal)};
    const std::uint64_t default_size{default_heap_size(memory_type)};
    const bool coherent{(m_memory_properties.memoryTypes[memory_type].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0};

    if(requirements.size > default_size) //Pseudo-dedicated allocation (because it is reusable unlike real dedicated allocations)
    {
        auto heap{std::make_unique<memory_heap>(m_device, memory_type, requirements.size, m_granularity, m_non_coherent_atom_size, coherent)};
        auto chunk{heap->allocate_pseudo_dedicated(resource_type, requirements.size)};

        std::lock_guard lock{m_mutex};
        m_heaps.emplace_back(std::move(heap));

        return chunk;
    }

    std::lock_guard lock{m_mutex};

    if(!std::empty(m_heaps))
    {
        stack_memory_pool<512> pool{};
        auto candidates{make_stack_vector<std::reference_wrapper<memory_heap>>(pool)};
        candidates.reserve(m_heaps.size());

        for(auto& heap : m_heaps)
        {
            if(!heap->dedicated() && heap->type() == memory_type && heap->free_space() > align_up(requirements.size, m_granularity))
            {
                candidates.emplace_back(std::ref(*heap));
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
                auto chunk{heap.try_allocate(resource_type, requirements.size, requirements.alignment)};

                if(chunk)
                {
                    return std::move(*chunk);
                }
            }
        }
    }

    auto& heap{m_heaps.emplace_back(std::make_unique<memory_heap>(m_device, memory_type, default_size, m_granularity, m_non_coherent_atom_size, coherent))};

    return heap->allocate(resource_type, requirements.size, requirements.alignment);
}

memory_heap_chunk memory_allocator::allocate(VkBuffer buffer, memory_resource_type resource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal)
{
    if(m_version >= tph::version{1, 1})
    {
        VkBufferMemoryRequirementsInfo2 info{};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
        info.buffer = buffer;

        VkMemoryDedicatedRequirements dedicated_requirements{};
        dedicated_requirements.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;

        VkMemoryRequirements2 requirements{};
        requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
        requirements.pNext = &dedicated_requirements;

        vkGetBufferMemoryRequirements2(m_device, &info, &requirements);

        if(dedicated_requirements.prefersDedicatedAllocation == VK_TRUE)
        {
            const std::uint32_t memory_type{find_memory_type(m_memory_properties, requirements.memoryRequirements.memoryTypeBits, minimal, optimal)};
            const std::uint64_t size{requirements.memoryRequirements.size};

            auto heap{std::make_unique<memory_heap>(m_device, buffer, memory_type, size)};
            auto chunk{heap->allocate_dedicated(size)};

            std::lock_guard lock{m_mutex};
            m_heaps.emplace_back(std::move(heap));

            return chunk;
        }
        else
        {
            return allocate(requirements.memoryRequirements, resource_type, minimal, optimal);
        }
    }
    else
    {
        VkMemoryRequirements requirements{};
        vkGetBufferMemoryRequirements(m_device, buffer, &requirements);

        return allocate(requirements, resource_type, minimal, optimal);
    }
}

memory_heap_chunk memory_allocator::allocate(VkImage image, memory_resource_type resource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal)
{
    if(m_version >= tph::version{1, 1})
    {
        VkImageMemoryRequirementsInfo2 info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
        info.image = image;

        VkMemoryDedicatedRequirements dedicated_requirements{};
        dedicated_requirements.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;

        VkMemoryRequirements2 requirements{};
        requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
        requirements.pNext = &dedicated_requirements;

        vkGetImageMemoryRequirements2(m_device, &info, &requirements);

        if(dedicated_requirements.prefersDedicatedAllocation == VK_TRUE)
        {
            const std::uint32_t memory_type{find_memory_type(m_memory_properties, requirements.memoryRequirements.memoryTypeBits, minimal, optimal)};
            const std::uint64_t size{requirements.memoryRequirements.size};

            auto heap{std::make_unique<memory_heap>(m_device, image, memory_type, size)};
            auto chunk{heap->allocate_dedicated(size)};

            std::lock_guard lock{m_mutex};
            m_heaps.emplace_back(std::move(heap));

            return chunk;
        }
        else
        {
            return allocate(requirements.memoryRequirements, resource_type, minimal, optimal);
        }
    }
    else
    {
        VkMemoryRequirements requirements{};
        vkGetImageMemoryRequirements(m_device, image, &requirements);

        return allocate(requirements, resource_type, minimal, optimal);
    }
}

memory_heap_chunk memory_allocator::allocate_bound(VkBuffer buffer, memory_resource_type resource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal)
{
    memory_heap_chunk chunk{allocate(buffer, resource_type, minimal, optimal)};
    chunk.bind(buffer);

    return chunk;
}

memory_heap_chunk memory_allocator::allocate_bound(VkImage image, memory_resource_type resource_type, VkMemoryPropertyFlags minimal, VkMemoryPropertyFlags optimal)
{
    memory_heap_chunk chunk{allocate(image, resource_type, minimal, optimal)};
    chunk.bind(image);

    return chunk;
}

void memory_allocator::clean()
{
    std::lock_guard lock{m_mutex};

    const auto predicate = [](const std::unique_ptr<memory_heap>& heap)
    {
         return heap->allocation_count() == 0;
    };

    m_heaps.erase(std::remove_if(std::begin(m_heaps), std::end(m_heaps), predicate), std::end(m_heaps));
}

void memory_allocator::clean_dedicated()
{
    std::lock_guard lock{m_mutex};

    const auto predicate = [](const std::unique_ptr<memory_heap>& heap)
    {
         return heap->dedicated() && heap->allocation_count() == 0;
    };

    m_heaps.erase(std::remove_if(std::begin(m_heaps), std::end(m_heaps), predicate), std::end(m_heaps));
}

memory_allocator::heap_sizes memory_allocator::heap_count() const
{
    std::lock_guard lock{m_mutex};

    heap_sizes output{};

    for(const auto& heap : m_heaps)
    {
        const auto flags{m_heaps_flags[m_memory_properties.memoryTypes[heap->type()].heapIndex]};

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

memory_allocator::heap_sizes memory_allocator::allocation_count() const
{
    std::lock_guard lock{m_mutex};

    heap_sizes output{};

    for(const auto& heap : m_heaps)
    {
        const auto flags{m_heaps_flags[m_memory_properties.memoryTypes[heap->type()].heapIndex]};

        if((flags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        {
            output.device_shared += heap->allocation_count();
        }
        else if(flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            output.device_local += heap->allocation_count();
        }
        else if(flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            output.host_shared += heap->allocation_count();
        }
    }

    return output;
}

memory_allocator::heap_sizes memory_allocator::used_memory() const
{
    std::lock_guard lock{m_mutex};

    heap_sizes output{};

    for(auto& heap : m_heaps)
    {
        const auto flags{m_heaps_flags[m_memory_properties.memoryTypes[heap->type()].heapIndex]};

        if((flags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        {
            output.device_shared += (heap->size() - heap->free_space());
        }
        else if(flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            output.device_local += (heap->size() - heap->free_space());
        }
        else if(flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            output.host_shared += (heap->size() - heap->free_space());
        }
    }

    return output;
}

memory_allocator::heap_sizes memory_allocator::allocated_memory() const
{
    std::lock_guard lock{m_mutex};

    heap_sizes output{};

    for(auto& heap : m_heaps)
    {
        const auto flags{m_heaps_flags[m_memory_properties.memoryTypes[heap->type()].heapIndex]};

        if((flags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        {
            output.device_shared += heap->size();
        }
        else if(flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            output.device_local += heap->size();
        }
        else if(flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            output.host_shared += heap->size();
        }
    }

    return output;
}

memory_allocator::heap_sizes memory_allocator::dedicated_heap_count() const
{
    std::lock_guard lock{m_mutex};

    heap_sizes output{};

    for(const auto& heap : m_heaps)
    {
        if(heap->dedicated())
        {
            const auto flags{m_heaps_flags[m_memory_properties.memoryTypes[heap->type()].heapIndex]};

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
    }

    return output;
}

memory_allocator::heap_sizes memory_allocator::dedicated_allocation_count() const
{
    std::lock_guard lock{m_mutex};

    heap_sizes output{};

    for(const auto& heap : m_heaps)
    {
        if(heap->dedicated())
        {
            const auto flags{m_heaps_flags[m_memory_properties.memoryTypes[heap->type()].heapIndex]};

            if((flags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
            {
                output.device_shared += heap->allocation_count();
            }
            else if(flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                output.device_local += heap->allocation_count();
            }
            else if(flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                output.host_shared += heap->allocation_count();
            }
        }
    }

    return output;
}

memory_allocator::heap_sizes memory_allocator::dedicated_used_memory() const
{
    std::lock_guard lock{m_mutex};

    heap_sizes output{};

    for(auto& heap : m_heaps)
    {
        if(heap->dedicated())
        {
            const auto flags{m_heaps_flags[m_memory_properties.memoryTypes[heap->type()].heapIndex]};

            if((flags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
            {
                output.device_shared += (heap->size() - heap->free_space());
            }
            else if(flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                output.device_local += (heap->size() - heap->free_space());
            }
            else if(flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                output.host_shared += (heap->size() - heap->free_space());
            }
        }
    }

    return output;
}

memory_allocator::heap_sizes memory_allocator::dedicated_allocated_memory() const
{
    std::lock_guard lock{m_mutex};

    heap_sizes output{};

    for(auto& heap : m_heaps)
    {
        if(heap->dedicated())
        {
            const auto flags{m_heaps_flags[m_memory_properties.memoryTypes[heap->type()].heapIndex]};

            if((flags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
            {
                output.device_shared += heap->size();
            }
            else if(flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                output.device_local += heap->size();
            }
            else if(flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                output.host_shared += heap->size();
            }
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
