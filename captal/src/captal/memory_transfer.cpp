#include "memory_transfer.hpp"

namespace cpt
{

/*
std::pair<tph::command_buffer&, transfer_ended_signal&> engine::begin_transfer()
{
    if(!m_transfer_began)
    {
        auto buffer{tph::cmd::begin(m_transfer_pool, tph::command_buffer_level::primary, tph::command_buffer_flags::one_time_submit)};

        if constexpr(debug_enabled)
        {
            tph::set_object_name(m_renderer, buffer, "cpt::engine's transfer buffer (frame: " + std::to_string(m_frame_id) + ")");
        }

        tph::cmd::pipeline_barrier(buffer, tph::pipeline_stage::color_attachment_output, tph::pipeline_stage::transfer);

        if constexpr(debug_enabled)
        {
            tph::cmd::begin_label(buffer, "cpt::engine's transfer (frame: " + std::to_string(m_frame_id) + ")", 1.0f, 0.843f, 0.0f, 1.0f);
        }

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



}
