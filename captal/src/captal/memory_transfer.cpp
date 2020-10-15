#include "memory_transfer.hpp"

#include <sstream>

namespace cpt
{

/*
std::pair<tph::command_buffer&, transfer_ended_signal&> engine::begin_transfer()
{
    if(!m_transfer_began)
    {
        auto buffer{tph::cmd::begin(m_transfer_pool, tph::command_buffer_level::primary, tph::command_buffer_flags::one_time_submit)};


        m_transfer_buffers.emplace_back(transfer_buffer{m_frame_id, std::move(buffer), tph::fence{m_renderer}});
        m_transfer_began = true;
    }

    return {m_transfer_buffers.back().buffer, m_transfer_buffers.back().signal};
}

void engine::flush_transfers()
{
    if(std::exchange(m_transfer_began, false))
    {
        if constexpr(debug_enabled)
        {
            tph::cmd::end_label(m_transfer_buffers.back().buffer);
        }

        tph::cmd::end(m_transfer_buffers.back().buffer);

        tph::submit_info info{};
        info.command_buffers.emplace_back(m_transfer_buffers.back().buffer);

        std::lock_guard lock{m_queue_mutex};
        tph::submit(m_renderer, info, m_transfer_buffers.back().fence);
    }

    for(auto it{std::begin(m_transfer_buffers)}; it != std::end(m_transfer_buffers);)
    {
        if(it->fence.try_wait())
        {
            it->signal();
            it = m_transfer_buffers.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

tph::set_object_name(m_renderer, m_transfer_pool, "cpt::engine's transfer command pool");

*/

static std::string thread_name(std::thread::id thread)
{
    std::ostringstream ss{};
    ss << thread;

    return ss.str();
};


memory_transfer_scheduler::memory_transfer_scheduler(tph::renderer& renderer) noexcept
:m_renderer{&renderer}
{

}

memory_transfer_info memory_transfer_scheduler::begin_transfer()
{
    std::unique_lock lock{m_mutex};

    const auto thread{std::this_thread::get_id()};

    auto it{m_pools.find(thread)};
    if(it == std::end(m_pools))
    {
        transfer_pool pool{};
        pool.pool = tph::command_pool{*m_renderer, tph::command_pool_options::reset};
        pool.exit_promise = std::promise<void>{};
        pool.exit_future = pool.exit_promise.get_future();

        it = m_pools.emplace(thread, std::move(pool)).first;
    }

    lock.unlock();

    auto& buffer{next_buffer(it->second, thread)};

}

void memory_transfer_scheduler::flush_transfers()
{

}

memory_transfer_scheduler::transfer_buffer& memory_transfer_scheduler::next_buffer(transfer_pool& pool, std::thread::id thread)
{
    const auto find_buffer = [this, &pool]() -> transfer_buffer&
    {
        for(auto& buffer : pool.buffers)
        {
            if(buffer.fence.try_wait())
            {
                reset(buffer);

                return buffer;
            }
        }

        return add_buffer(pool);
    };

    auto& buffer{find_buffer()};

    if constexpr(debug_enabled)
    {
        tph::set_object_name(*m_renderer, buffer.buffer, "cpt::engine's transfer buffer (thread: " + thread_name(thread) + ")");
    }

    tph::cmd::pipeline_barrier(buffer.buffer, tph::pipeline_stage::color_attachment_output, tph::pipeline_stage::transfer);

    if constexpr(debug_enabled)
    {
        tph::cmd::begin_label(buffer.buffer, "cpt::engine's transfer (thread: " + thread_name(thread) + ")", 1.0f, 0.843f, 0.0f, 1.0f);
    }

    return buffer;
}

memory_transfer_scheduler::transfer_buffer& memory_transfer_scheduler::add_buffer(transfer_pool& pool)
{
    transfer_buffer data{};
    data.buffer = tph::command_buffer{tph::cmd::begin(pool.pool, tph::command_buffer_level::primary, tph::command_buffer_flags::one_time_submit)};
    data.fence = tph::fence{*m_renderer, true};
    data.begin = true;

    return pool.buffers.emplace_back(std::move(data));
}

void memory_transfer_scheduler::reset(transfer_buffer& data)
{
    data.keeper.clear();
    data.signal();
    data.signal.disconnect_all();

    tph::cmd::begin(data.buffer, tph::command_buffer_reset_flags::none, tph::command_buffer_flags::one_time_submit);
    data.begin = true;
}

}
