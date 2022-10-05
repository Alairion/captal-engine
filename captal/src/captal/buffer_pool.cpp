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

void buffer_heap_chunk::upload(std::uint64_t offset, std::uint64_t size)
{
    m_parent->register_upload(m_offset + offset, std::min(size, m_size - offset));
}

void* buffer_heap_chunk::map(std::uint64_t offset) noexcept
{
    return reinterpret_cast<std::uint8_t*>(m_parent->map()) + m_offset + offset;
}

const void* buffer_heap_chunk::map(std::uint64_t offset) const noexcept
{
    return reinterpret_cast<const std::uint8_t*>(m_parent->map()) + m_offset + offset;
}

buffer_heap::buffer_heap(std::uint64_t size, tph::buffer_usage usage)
:m_local_data{engine::instance().device(), size, usage | tph::buffer_usage::transfer_src}
,m_device_data{engine::instance().device(), size, usage | tph::buffer_usage::transfer_dest | tph::buffer_usage::device_only}
,m_size{size}
,m_local_map{m_local_data.map()}
{
    m_ranges.reserve(64);
    m_upload_ranges.reserve(64);
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

#ifdef CAPTAL_DEBUG
void buffer_heap::set_name(std::string_view name)
{
    std::unique_lock lock{m_upload_mutex};

    m_name = name;

    tph::set_object_name(engine::instance().device(), m_local_data, m_name + " host buffer");
    tph::set_object_name(engine::instance().device(), m_device_data, m_name + " device buffer");

    for(std::size_t i{}; i < std::size(m_stagings); ++i)
    {
        tph::set_object_name(engine::instance().device(), m_stagings[i].buffer, m_name + " staging buffer #" + std::to_string(i));
    }
}
#endif

template<std::forward_iterator ForwardIt>
static ForwardIt coalesce(ForwardIt begin, ForwardIt end)
{
    ForwardIt result{begin};
    ForwardIt last  {begin};

    while(++begin != end)
    {
        if(last->src_offset + last->size >= begin->src_offset) //Overlaping or contiguous
        {
            last->size = std::max(begin->src_offset + begin->size - last->src_offset, last->size);
        }
        else
        {
            *result++ = *last;
            last = begin;
        }
    }

    *result++ = *last;

    return result;
}

bool buffer_heap::begin_upload(tph::command_buffer& command_buffer)
{
    std::unique_lock lock{m_upload_mutex};

    if(std::empty(m_upload_ranges))
    {
        return false;
    }

    const auto sort_predicate = [](const tph::buffer_copy& left, const tph::buffer_copy& right)
    {
        return left.src_offset < right.src_offset;
    };

    std::sort(std::begin(m_upload_ranges), std::end(m_upload_ranges), sort_predicate);
    m_upload_ranges.erase(coalesce(std::begin(m_upload_ranges), std::end(m_upload_ranges)), std::end(m_upload_ranges));

    const auto accumulator = [](std::uint64_t left, tph::buffer_copy& right)
    {
        return left + right.size;
    };

    const std::uint64_t total_size {std::accumulate(std::begin(m_upload_ranges), std::end(m_upload_ranges), 0ull, accumulator)};
    const std::uint64_t chunk_size {m_size / 4};
    const std::uint64_t chunk_count{(total_size + chunk_size) / chunk_size};
    const std::uint64_t chunk_mask {(1u << chunk_count) - 1u};

    const auto find_staging = [this, chunk_mask]
    {
        for(std::size_t i{}; i < std::size(m_stagings); ++i)
        {
            for(std::uint64_t mask{chunk_mask}; mask < 0x10; mask <<= 1)
            {
                if((m_stagings[i].used & mask) == 0)
                {
                    return std::make_pair(i, mask);
                }
            }
        }

        return std::make_pair(std::numeric_limits<std::size_t>::max(), std::uint64_t{});
    };

    const auto [staging_index, mask] = find_staging();

    if(staging_index != std::numeric_limits<std::size_t>::max())
    {
        m_current_staging = staging_index;
        m_current_mask = mask;

        for(std::size_t i{}; i < 4; ++i) //find base index of the staging
        {
            if(((m_current_mask >> i) & 0x01) != 0)
            {
                m_current_mask_index = i;
                break;
            }
        }
    }
    else
    {
        constexpr auto usage{tph::buffer_usage::transfer_src | tph::buffer_usage::transfer_dest};

        m_stagings.emplace_back(staging_buffer{tph::buffer{engine::instance().device(), m_size, usage}});

        #ifdef CAPTAL_DEBUG
        if(!std::empty(m_name))
        {
            tph::set_object_name(engine::instance().device(), m_stagings.back().buffer, m_name + " staging buffer #" + std::to_string(std::size(m_stagings) - 1));
        }
        #endif

        m_current_staging = std::size(m_stagings) - 1;
        m_current_mask = chunk_mask;
        m_current_mask_index = 0;
    }

    std::uint64_t current_offset{chunk_size * m_current_mask_index};
    for(auto& range : m_upload_ranges)
    {
        range.dest_offset = current_offset;
        current_offset += range.size;
    }

    staging_buffer& staging{m_stagings[m_current_staging]};
    staging.used |= m_current_mask;

    tph::cmd::copy(command_buffer, m_local_data, staging.buffer, m_upload_ranges);

    lock.release();

    return true;
}

void buffer_heap::end_upload(tph::command_buffer& command_buffer, transfer_ended_signal& signal)
{
    std::unique_lock lock{m_upload_mutex, std::adopt_lock};

    staging_buffer& staging{m_stagings[m_current_staging]};

    for(auto& range : m_upload_ranges)
    {
        std::swap(range.src_offset, range.dest_offset);
    }

    tph::cmd::copy(command_buffer, staging.buffer, m_device_data, m_upload_ranges);
    m_upload_ranges.clear();

    staging.connection[m_current_mask_index] = signal.connect([this, index = m_current_staging, mask = m_current_mask]()
    {
        m_stagings[index].used &= ~mask;
    });
}

void buffer_heap::register_upload(std::uint64_t offset, std::uint64_t size) noexcept
{
    std::lock_guard lock{m_upload_mutex};

    assert(size != 0 && "cpt::buffer_heap::register_upload called with null size.");

    m_upload_ranges.emplace_back(offset, 0, size);
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

        #ifdef CAPTAL_DEBUG
        if(!std::empty(m_name))
        {
            heap->set_name(m_name + " heap #" + std::to_string(std::size(m_heaps)));
        }
        #endif

        std::lock_guard lock{m_mutex};
        m_to_end.emplace_back();
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
                    return std::move(*chunk);
                }
            }
        }
    }

    auto heap {std::make_unique<buffer_heap>(m_pool_size, m_pool_usage)};
    auto chunk{heap->allocate_first(size)};

    #ifdef CAPTAL_DEBUG
    if(!std::empty(m_name))
    {
        heap->set_name(m_name + " heap #" + std::to_string(std::size(m_heaps)));
    }
    #endif

    m_to_end.emplace_back();
    m_heaps.emplace_back(std::move(heap));

    return chunk;
}

void buffer_pool::upload(memory_transfer_info info)
{
    std::lock_guard lock{m_mutex};

    #ifdef CAPTAL_DEBUG
    tph::cmd::begin_label(info.buffer, m_name + " transfer", 0.961f, 0.961f, 0.863f, 1.0f);
    #endif

    for(std::size_t i{}; i < std::size(m_heaps); ++i)
    {
        m_to_end[i] = m_heaps[i]->begin_upload(info.buffer);
    }

    tph::cmd::pipeline_barrier(info.buffer, tph::resource_access::transfer_write, tph::resource_access::transfer_read, tph::dependency_flags::none,
                                            tph::pipeline_stage::transfer, tph::pipeline_stage::transfer);

    for(std::size_t i{}; i < std::size(m_heaps); ++i)
    {
        if(m_to_end[i])
        {
            m_heaps[i]->end_upload(info.buffer, info.signal);
        }
    }

    #ifdef CAPTAL_DEBUG
    tph::cmd::end_label(info.buffer);
    #endif
}

void buffer_pool::upload()
{
    upload(cpt::engine::instance().transfer_scheduler().begin_transfer());
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

#ifdef CAPTAL_DEBUG
void buffer_pool::set_name(std::string_view name)
{
    std::lock_guard lock{m_mutex};

    m_name = name;

    for(std::size_t i{}; i < std::size(m_heaps); ++i)
    {
        m_heaps[i]->set_name(m_name + " heap #" + std::to_string(i));
    }
}
#endif

}
