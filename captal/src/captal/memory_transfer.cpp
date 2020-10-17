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
        auto buffer{tph::cmd::begin(m_thread_transfer_pool, tph::command_buffer_level::primary, tph::command_buffer_flags::one_time_submit)};


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
,m_pool{renderer, tph::command_pool_options::reset}
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

    auto& pool{get_pool(thread)};
    auto& buffer{next_thread_buffer(pool, thread)};

/*
    tph::cmd::pipeline_barrier(buffer.buffer, tph::pipeline_stage::color_attachment_output, tph::pipeline_stage::transfer);
*/
    return memory_transfer_info{buffer.buffer, buffer.signal, buffer.keeper};
}

void memory_transfer_scheduler::submit_transfers()
{/*
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

            tph::cmd::end(data->buffer);

            data->begin = false;
            data->fence.reset();

            tph::submit_info info{};
            info.command_buffers.emplace_back(data->buffer);

            std::lock_guard lock{engine::instance().submit_mutex()};
            tph::submit(*m_renderer, info, data->fence);
        }
    }

    for(auto&& pool : m_thread_pools)
    {
        if(pool.second.exit_future.wait_for(std::chrono::seconds{0}) == std::future_status::ready)
        {

        }
    }*/
}

memory_transfer_scheduler::thread_transfer_pool& memory_transfer_scheduler::get_pool(std::thread::id thread)
{
    auto it{m_thread_pools.find(thread)};
    if(it == std::end(m_thread_pools))
    {
        thread_transfer_pool pool{};
        pool.pool = tph::command_pool{*m_renderer, tph::command_pool_options::reset};
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
        if(buffer.fence.try_wait())
        {
            reset_thread_buffer(buffer, thread);

            return buffer;
        }
    }

    return add_thread_buffer(pool, thread);
}

memory_transfer_scheduler::thread_transfer_buffer& memory_transfer_scheduler::add_thread_buffer(thread_transfer_pool& pool, std::thread::id thread)
{
    thread_transfer_buffer data{};
    data.buffer = tph::command_buffer{tph::cmd::begin(pool.pool, tph::command_buffer_level::secondary, tph::command_buffer_flags::one_time_submit)};

    if constexpr(debug_enabled)
    {
        const auto name{thread_name(thread)};

        tph::set_object_name(*m_renderer, data.buffer, "cpt::engine's transfer buffer (thread: " + name + ")");
        tph::cmd::begin_label(data.buffer, "cpt::engine's transfer (thread: " + name + ")", 1.0f, 0.843f, 0.0f, 1.0f);
    }

    data.begin = true;

    return pool.buffers.emplace_back(std::move(data));
}

bool memory_transfer_scheduler::check_thread_buffer(thread_transfer_buffer& data)
{

}

void memory_transfer_scheduler::reset_thread_buffer(thread_transfer_buffer& data, std::thread::id thread)
{
    data.signal();
    data.signal.disconnect_all();
    data.keeper.clear();
    data.submitted = false;
    data.parent = no_parent;

    tph::cmd::begin(data.buffer, tph::command_buffer_reset_flags::none, tph::command_buffer_flags::one_time_submit);

    if constexpr(debug_enabled)
    {
        const auto name{thread_name(thread)};

        tph::cmd::begin_label(data.buffer, "cpt::engine's transfer (thread: " + name + ")", 1.0f, 0.843f, 0.0f, 1.0f);
    }

    data.begin = true;
}


}
