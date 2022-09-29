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

#include "memory_transfer.hpp"

#include "engine.hpp"

#include <sstream>

namespace cpt
{

static std::string thread_name(std::thread::id thread)
{
    std::ostringstream ss{};
    ss << thread;

    return ss.str();
}

memory_transfer_scheduler::memory_transfer_scheduler(tph::device& device) noexcept
:m_device{&device}
,m_pool{device, tph::command_pool_options::reset | tph::command_pool_options::transient}
{
    if(debug_enabled)
    {
        tph::set_object_name(*m_device, m_pool, "cpt::engine's primary transfer command pool");
    }

    m_thread_pools.reserve(4);
    m_buffers.reserve(4);

    //Main thread pool
    const auto thread{std::this_thread::get_id()};

    thread_transfer_pool main_pool{};
    main_pool.pool = tph::command_pool{*m_device, tph::command_pool_options::reset | tph::command_pool_options::transient};
    main_pool.buffers.reserve(4);

    if(debug_enabled)
    {
        const auto name{thread_name(thread)};

        tph::set_object_name(*m_device, main_pool.pool, "cpt::engine's thread transfer pool (thread: " + name + ")");
    }

    m_thread_pools.emplace(thread, std::move(main_pool));
}

memory_transfer_scheduler::~memory_transfer_scheduler()
{

}

memory_transfer_info memory_transfer_scheduler::begin_transfer()
{
    return begin_transfer(std::this_thread::get_id());
}

memory_transfer_info memory_transfer_scheduler::begin_transfer(std::thread::id thread)
{
    std::unique_lock lock{m_mutex};

    auto& pool  {get_transfer_pool(thread)};
    auto& buffer{next_thread_buffer(pool, thread)};

    m_begin = true;

    return memory_transfer_info{buffer.buffer, buffer.signal, buffer.keeper};
}

void memory_transfer_scheduler::submit_transfers()
{
    std::unique_lock lock{m_mutex};

    if(!m_begin)
    {
        return;
    }

    m_begin = false;

    auto& buffer{next_buffer()};
    const auto index{buffer_index(buffer)};
    const auto to_execute{secondary_buffers(index)};

    tph::cmd::pipeline_barrier(buffer.buffer, tph::pipeline_stage::bottom_of_pipe, tph::pipeline_stage::transfer, tph::dependency_flags::none);
    tph::cmd::execute(buffer.buffer, to_execute);
    tph::cmd::end(buffer.buffer);

    buffer.fence.reset();

    tph::submit_info info{};
    info.command_buffers.emplace_back(buffer.buffer);

    std::unique_lock queue_lock{engine::instance().submit_mutex()};
    tph::submit(*m_device, info, buffer.fence);
    queue_lock.unlock();
}

memory_transfer_scheduler::transfer_buffer& memory_transfer_scheduler::next_buffer()
{
    for(auto& buffer : m_buffers)
    {
        if(buffer.fence.try_wait())
        {
            reset_buffer(buffer);

            return buffer;
        }
    }

    return add_buffer();
}

memory_transfer_scheduler::transfer_buffer& memory_transfer_scheduler::add_buffer()
{
    transfer_buffer data{};
    data.buffer = tph::cmd::begin(m_pool, tph::command_buffer_level::primary, tph::command_buffer_options::one_time_submit);
    data.fence  = tph::fence{*m_device, true};

    if constexpr(debug_enabled)
    {
        tph::set_object_name(*m_device, data.buffer, "cpt::engine's primary transfer buffer #" + std::to_string(std::size(m_buffers)));
        tph::set_object_name(*m_device, data.fence, "cpt::engine's transfer fence #" + std::to_string(std::size(m_buffers)));
    }

    return m_buffers.emplace_back(std::move(data));
}

std::size_t memory_transfer_scheduler::buffer_index(const transfer_buffer& buffer) const noexcept
{
    return static_cast<std::size_t>(std::distance(std::data(m_buffers), &buffer));
}

void memory_transfer_scheduler::reset_buffer(transfer_buffer& buffer)
{
    const std::size_t index{buffer_index(buffer)};

    for(auto&& [thread, pool] : m_thread_pools)
    {
        for(auto&& buffer : pool.buffers)
        {
            if(buffer.parent == index)
            {
                reset_thread_buffer(buffer);
            }
        }
    }

    tph::cmd::begin(buffer.buffer, tph::command_buffer_reset_options::none, tph::command_buffer_options::one_time_submit);
}

void memory_transfer_scheduler::reset_thread_buffer(thread_transfer_buffer& data)
{
    data.signal();
    data.signal.disconnect_all();
    data.keeper.clear();
    data.parent = no_parent;
}

std::vector<std::reference_wrapper<tph::command_buffer>> memory_transfer_scheduler::secondary_buffers(std::size_t parent)
{
    std::vector<std::reference_wrapper<tph::command_buffer>> output{};
    output.reserve(std::size(m_thread_pools));

    for(auto&& pool : m_thread_pools)
    {
        for(auto&& thread_buffer : pool.second.buffers)
        {
            if(thread_buffer.begin)
            {
                if constexpr(debug_enabled)
                {
                    tph::cmd::end_label(thread_buffer.buffer);
                }

                tph::cmd::end(thread_buffer.buffer);

                thread_buffer.begin = false;
                thread_buffer.parent = parent;

                output.emplace_back(thread_buffer.buffer);
            }
        }
    }

    return output;
}

std::size_t memory_transfer_scheduler::clean_threads()
{
    return std::erase_if(m_thread_pools, [](const auto& item)
    {
        auto&& [thread, pool] = item;

        for(auto&& buffer : pool.buffers)
        {
            if(buffer.begin || buffer.parent != no_parent) //buffer in use
            {
                return false;
            }
        }

        return pool.exit_future.wait_for(std::chrono::seconds{0}) == std::future_status::ready;
    });
}

memory_transfer_scheduler::thread_transfer_pool& memory_transfer_scheduler::get_transfer_pool(std::thread::id thread)
{
    auto it{m_thread_pools.find(thread)};
    if(it == std::end(m_thread_pools))
    {
        thread_transfer_pool pool{};
        pool.pool = tph::command_pool{*m_device, tph::command_pool_options::reset | tph::command_pool_options::transient};
        pool.exit_promise = std::promise<void>{};
        pool.exit_promise.set_value_at_thread_exit();
        pool.exit_future = pool.exit_promise.get_future();
        pool.buffers.reserve(4);

        if(debug_enabled)
        {
            const auto name{thread_name(thread)};

            tph::set_object_name(*m_device, pool.pool, "cpt::engine's thread transfer pool (thread: " + name + ")");
        }

        it = m_thread_pools.emplace(thread, std::move(pool)).first;
    }

    return it->second;
}

memory_transfer_scheduler::thread_transfer_buffer& memory_transfer_scheduler::next_thread_buffer(thread_transfer_pool& pool, std::thread::id thread)
{
    for(auto& buffer : pool.buffers)
    {
        if(buffer.begin == true)
        {
            return buffer;
        }
    }

    for(auto& buffer : pool.buffers)
    {
        if(buffer.parent == no_parent)
        {
            tph::cmd::begin(buffer.buffer, tph::command_buffer_reset_options::none, tph::command_buffer_options::one_time_submit);

            if constexpr(debug_enabled)
            {
                const auto name{thread_name(thread)};

                tph::cmd::begin_label(buffer.buffer, "cpt::engine's transfer (thread: " + name + ")", 1.0f, 0.843f, 0.0f, 1.0f);
            }

            buffer.begin = true;

            return buffer;
        }
    }

    return add_thread_buffer(pool, thread);
}

memory_transfer_scheduler::thread_transfer_buffer& memory_transfer_scheduler::add_thread_buffer(thread_transfer_pool& pool, std::thread::id thread)
{
    thread_transfer_buffer data{};
    data.buffer = tph::cmd::begin(pool.pool, tph::command_buffer_level::secondary, tph::command_buffer_options::one_time_submit);
    data.keeper.reserve(512);

    if constexpr(debug_enabled)
    {
        const auto name{thread_name(thread)};

        tph::set_object_name(*m_device, data.buffer, "cpt::engine's thread transfer buffer #" + std::to_string(std::size(pool.buffers)) + " (thread: " + name + ")");
        tph::cmd::begin_label(data.buffer, "cpt::engine's transfer (thread: " + name + ")", 1.0f, 0.843f, 0.0f, 1.0f);
    }

    data.begin = true;

    return pool.buffers.emplace_back(std::move(data));
}

}
