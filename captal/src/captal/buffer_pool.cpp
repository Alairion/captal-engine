#include "buffer_pool.hpp"

#include <captal_foundation/stack_allocator.hpp>

#include "engine.hpp"

namespace cpt
{

buffer_heap_chunk::buffer_heap_chunk(buffer_heap* parent, std::uint64_t offset, std::uint64_t size) noexcept
:m_parent{parent}
,m_offset{offset}
,m_size{size}
{

}

buffer_heap_chunk::~buffer_heap_chunk()
{
    if(m_parent)
    {
        m_parent->unregister_chunk(*this);
    }
}

buffer_heap_chunk::buffer_heap_chunk(buffer_heap_chunk&& other) noexcept
:m_parent{std::exchange(other.m_parent, nullptr)}
,m_offset{other.m_offset}
,m_size{other.m_size}
{

}

buffer_heap_chunk& buffer_heap_chunk::operator=(buffer_heap_chunk&& other) noexcept
{
    std::swap(other.m_parent, m_parent);
    std::swap(other.m_offset, m_offset);
    std::swap(other.m_size, m_size);

    return *this;
}

void* buffer_heap_chunk::map() noexcept
{
    return reinterpret_cast<std::uint8_t*>(m_parent->map()) + m_offset;
}

const void* buffer_heap_chunk::map() const noexcept
{
    return reinterpret_cast<const std::uint8_t*>(m_parent->map()) + m_offset;
}

buffer_heap::buffer_heap(std::uint64_t size, tph::buffer_usage usage)
:m_local_data{engine::instance().renderer(), size, usage | tph::buffer_usage::transfer_source}
,m_device_data{engine::instance().renderer(), size, usage | tph::buffer_usage::transfer_destination | tph::buffer_usage::device_only}
,m_size{size}
,m_local_map{m_local_data.map()}
{

}

buffer_heap::~buffer_heap()
{
    assert(allocation_count() == 0 && "cpt::buffer_heap destroyed with non-freed buffers");
}

std::optional<buffer_heap_chunk> buffer_heap::try_allocate(std::uint64_t size, std::uint64_t alignment)
{
    std::lock_guard lock{m_mutex};

    //The whole algorithm is basically first-fit + coalescing

    //Push it at the begginning if the heap is empty
    if(std::empty(m_ranges) && size <= m_size)
    {
        m_ranges.emplace_back(0, size);

        m_free_space -= size;
        m_allocation_count = 1;

        return std::make_optional(buffer_heap_chunk{this, 0, size});
    }

    //Try to push it at the end
    const auto try_push = [this, size, alignment]
    {
        const auto& last{m_ranges.back()};
        const auto  end {align_up(last.offset + last.size, alignment)};

        if(m_size - end >= size)
        {
            m_ranges.emplace_back(end, size);

            return std::cend(m_ranges) - 1;
        }

        return std::cend(m_ranges);
    };

    if(const auto it{try_push()}; it != std::cend(m_ranges))
    {
        m_free_space -= it->size;
        m_allocation_count += 1;

        return std::make_optional(buffer_heap_chunk{this, it->offset, it->size});
    }

    //Try to insert it at a suitable place
    const auto try_insert = [this, size, alignment]() -> std::vector<range>::const_iterator
    {
        for(auto it{std::cbegin(m_ranges)}; it != std::cend(m_ranges) - 1; ++it)
        {
            const auto next{it + 1};
            const auto end {align_up(it->offset + it->size, alignment)};

            if(static_cast<std::int64_t>(next->offset) - static_cast<std::int64_t>(end) >= static_cast<std::int64_t>(size))
            {
                return m_ranges.insert(next, range{end, size});
            }
        }

        return std::cend(m_ranges);
    };

    if(const auto it{try_insert()}; it != std::cend(m_ranges))
    {
        m_free_space -= it->size;
        m_allocation_count += 1;

        return std::make_optional(buffer_heap_chunk{this, it->offset, it->size});
    }

    return std::nullopt;
}

buffer_heap_chunk buffer_heap::allocate_first(std::uint64_t size)
{
    m_ranges.emplace_back(0, size);

    m_free_space -= size;
    m_allocation_count = 1;

    return buffer_heap_chunk{this, 0, size};
}

void buffer_heap::upload_local_changes(tph::command_buffer& command_buffer, transfer_ended_signal& signal)
{
    std::size_t staging_index{std::numeric_limits<std::size_t>::max()};

    for(std::size_t i{}; i < std::size(m_stagings); ++i)
    {
        if(m_stagings[i].available)
        {
            staging_index = i;
            break;
        }
    }

    if(staging_index == std::numeric_limits<std::size_t>::max())
    {
        m_stagings.emplace_back(staging_buffer{tph::buffer{engine::instance().renderer(), m_size, tph::buffer_usage::transfer_source}});
        staging_index = std::size(m_stagings) - 1;
    }

    staging_buffer& buffer{m_stagings[staging_index]};
    buffer.available = false;

    std::memcpy(buffer.buffer.map(), std::data(m_data), m_size);
    buffer.buffer.unmap();

    tph::cmd::copy(command_buffer, buffer.buffer, m_device_buffer);

    buffer.connection = signal.connect([this, staging_index]()
    {
        m_stagings[staging_index].available = true;
    });
}

void buffer_heap::unregister_chunk(const buffer_heap_chunk& chunk) noexcept
{
    std::lock_guard lock{m_mutex};

    const auto predicate = [](const range& range, const buffer_heap_chunk& chunk)
    {
        return range.offset < chunk.m_offset;
    };

    const auto it{std::lower_bound(std::cbegin(m_ranges), std::cend(m_ranges), chunk, predicate)};
    assert(it != std::cend(m_ranges) && "Bad memory heap chunk.");

    m_free_space += it->size;
    m_allocation_count -= 1;

    m_ranges.erase(it);
}

buffer_pool::buffer_pool(tph::buffer_usage pool_usage, std::uint64_t pool_size)
:m_pool_usage{pool_usage}
,m_pool_size{pool_size}
{

}

buffer_heap_chunk buffer_pool::allocate(std::uint64_t size, std::uint64_t alignment)
{
    if(size > m_pool_size)
    {
        auto heap {std::make_unique<buffer_heap>(size, m_pool_usage)};
        auto chunk{heap->allocate_first(size)};

        std::lock_guard lock{m_mutex};
        m_heaps.emplace_back(std::move(heap));

        return chunk;
    }

    std::lock_guard lock{m_mutex};

    if(!std::empty(m_heaps))
    {
        stack_memory_pool<512> pool{};
        auto candidates{make_stack_vector<std::reference_wrapper<buffer_heap>>(pool)};
        candidates.reserve(m_heaps.size());

        for(auto& heap : m_heaps)
        {
            if(heap->free_space() > align_up(size, alignment))
            {
                candidates.emplace_back(std::ref(*heap));
            }
        }

        if(!std::empty(candidates))
        {
            const auto predicate = [](const buffer_heap& left, const buffer_heap& right)
            {
                return left.free_space() < right.free_space();
            };

            std::sort(std::begin(candidates), std::end(candidates), predicate);

            for(buffer_heap& heap : candidates)
            {
                auto chunk{heap.try_allocate(size, alignment)};

                if(chunk)
                {
                    return std::move(chunk.value());
                }
            }
        }
    }

    return m_heaps.emplace_back(std::make_unique<buffer_heap>(size, m_pool_usage))->allocate_first(size);
}

void buffer_pool::clean()
{
    std::lock_guard lock{m_mutex};

    const auto predicate = [](const std::unique_ptr<buffer_heap>& heap)
    {
         return heap->allocation_count() == 0;
    };

    m_heaps.erase(std::remove_if(std::begin(m_heaps), std::end(m_heaps), predicate), std::end(m_heaps));
}

}
