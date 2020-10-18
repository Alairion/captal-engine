#include "memory_transfer.hpp"

#include "engine.hpp"

#include <sstream>

namespace cpt
{

/*
std::pair<tph::command_buffer&, transfer_ended_signal&> engine::begin_transfer()
{
    if(!m_transfer_began)
    {
        auto buffer{tph::cmd::begin(m_thread_transfer_pool, tph::command_buffer_level::primary, tph::command_buffer_options::one_time_submit)};


        m_thread_transfer_buffers.emplace_back(thread_transfer_buffer{m_frame_id, std::move(buffer), tph::fence{m_renderer}});
        m_transfer_began = true;
    }

    return {m_thread_transfer_buffers.back().buffer, m_thread_transfer_buffers.back().signal};
}

void engine::flush_transfers()
{
    if(std::exchange(m_transfer_began, false))
    {
        if constexpr(debug_enabled)
        {
            tph::cmd::end_label(m_thread_transfer_buffers.back().buffer);
        }

        tph::cmd::end(m_thread_transfer_buffers.back().buffer);

        tph::submit_info info{};
        info.command_buffers.emplace_back(m_thread_transfer_buffers.back().buffer);

        std::lock_guard lock{m_queue_mutex};
        tph::submit(m_renderer, info, m_thread_transfer_buffers.back().fence);
    }

    for(auto it{std::begin(m_thread_transfer_buffers)}; it != std::end(m_thread_transfer_buffers);)
    {
        if(it->fence.try_wait())
        {
            it->signal();
            it = m_thread_transfer_buffers.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

tph::set_object_name(m_renderer, m_thread_transfer_pool, "cpt::engine's transfer command pool");

*/

static std::string thread_name(std::thread::id thread)
{
    std::ostringstream ss{};
    ss << thread;

    return ss.str();
};

static bool is_future_ready(const std::future<void>& future)
{
    return future.wait_for(std::chrono::seconds{0}) == std::future_status::ready;
}

memory_transfer_scheduler::memory_transfer_scheduler(tph::renderer& renderer) noexcept
:m_renderer{&renderer}
,m_pool{renderer, tph::command_pool_options::reset | tph::command_pool_options::transient}
{
    m_thread_pools.reserve(4);
    m_buffers.reserve(4);
}

memory_transfer_info memory_transfer_scheduler::begin_transfer()
{
    return begin_transfer(std::this_thread::get_id());
}

memory_transfer_info memory_transfer_scheduler::begin_transfer(std::thread::id thread)
{
    std::unique_lock lock{m_mutex};

    auto& pool{get_transfer_pool(thread)};
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

    tph::cmd::pipeline_barrier(buffer.buffer, tph::pipeline_stage::bottom_of_pipe, tph::pipeline_stage::transfer);
    tph::cmd::execute(buffer.buffer, to_execute);
    tph::cmd::end(buffer.buffer);

    buffer.fence.reset();

    tph::submit_info info{};
    info.command_buffers.emplace_back(buffer.buffer);

    std::lock_guard queue_lock{engine::instance().submit_mutex()};
    tph::submit(*m_renderer, info, buffer.fence);
    /*
    std::unique_lock lock{m_mutex};

    const auto find_data = [](thread_transfer_pool& pool) -> optional_ref<thread_transfer_buffer>
    {
        for(auto& buffer : pool.buffers)
        {
            if(buffer.begin)
            {
                return buffer;
            }
        }

        return nullref;
    };

    for(auto&& pool : m_thread_pools)
    {
        const auto data{find_data(pool.second)};

        if(data)
        {
            if constexpr(debug_enabled)
            {
                tph::cmd::end_label(data->buffer);
            }


            tph::cmd::pipeline_barrier(buffer.buffer, tph::pipeline_stage::color_attachment_output, tph::pipeline_stage::transfer);

            tph::cmd::end(data->buffer);

            data->begin = false;
        }
    }

    for(auto&& pool : m_thread_pools)
    {
        if(pool.second.exit_future.wait_for(std::chrono::seconds{0}) == std::future_status::ready)
        {

        }
    }*/
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
    data.buffer = tph::command_buffer{tph::cmd::begin(m_pool, tph::command_buffer_level::primary, tph::command_buffer_options::one_time_submit)};
    data.fence = tph::fence{*m_renderer, true};

    return m_buffers.emplace_back(std::move(data));
}

std::size_t memory_transfer_scheduler::buffer_index(const transfer_buffer& buffer) const noexcept
{
    return static_cast<std::size_t>(std::distance(std::data(m_buffers), &buffer));
}

void memory_transfer_scheduler::reset_buffer(transfer_buffer& buffer)
{
    const std::size_t index{buffer_index(buffer)};

    for(auto&& pool : m_thread_pools)
    {
        for(auto&& buffer : pool.second.buffers)
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
                thread_buffer.begin = false;
                thread_buffer.parent = parent;

                output.emplace_back(thread_buffer.buffer);
            }
        }
    }

    return output;
}

memory_transfer_scheduler::thread_transfer_pool& memory_transfer_scheduler::get_transfer_pool(std::thread::id thread)
{
    auto it{m_thread_pools.find(thread)};
    if(it == std::end(m_thread_pools))
    {
        thread_transfer_pool pool{};
        pool.pool = tph::command_pool{*m_renderer, tph::command_pool_options::reset | tph::command_pool_options::transient};
        pool.exit_promise = std::promise<void>{};
        pool.exit_promise.set_value_at_thread_exit();
        pool.exit_future = pool.exit_promise.get_future();
        pool.buffers.reserve(4);

        it = m_thread_pools.emplace(thread, std::move(pool)).first;
    }

    //A thread ID may be recycled, if it happens, simply do as nothing happened :)
    if(is_future_ready(it->second.exit_future))
    {
        it->second.exit_promise = std::promise<void>{};
        it->second.exit_promise.set_value_at_thread_exit();
        it->second.exit_future = it->second.exit_promise.get_future();
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
    data.buffer = tph::command_buffer{tph::cmd::begin(pool.pool, tph::command_buffer_level::secondary, tph::command_buffer_options::one_time_submit)};

    if constexpr(debug_enabled)
    {
        const auto name{thread_name(thread)};

        tph::set_object_name(*m_renderer, data.buffer, "cpt::engine's transfer buffer (thread: " + name + ")");
        tph::cmd::begin_label(data.buffer, "cpt::engine's transfer (thread: " + name + ")", 1.0f, 0.843f, 0.0f, 1.0f);
    }

    data.begin = true;

    return pool.buffers.emplace_back(std::move(data));
}
}
