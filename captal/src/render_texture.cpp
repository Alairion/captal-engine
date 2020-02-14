#include "render_texture.hpp"

#include "engine.hpp"

namespace cpt
{

render_texture::render_texture(std::uint32_t width, std::uint32_t height, tph::render_target_options target_options, tph::sample_count sample_count)
:texture{width, height, tph::texture_usage::color_attachment | tph::texture_usage::sampled}
,render_target{engine::instance().renderer(), get_texture(), target_options, sample_count}
,m_options{target_options}
,m_sample_count{sample_count}
{

}

render_texture::render_texture(std::uint32_t width, std::uint32_t height, const tph::sampling_options& sampling, tph::render_target_options target_options, tph::sample_count sample_count)
:texture{width, height, sampling, tph::texture_usage::color_attachment | tph::texture_usage::sampled}
,render_target{engine::instance().renderer(), get_texture(), target_options, sample_count}
,m_options{target_options}
,m_sample_count{sample_count}
{

}

std::pair<tph::command_buffer&, frame_presented_signal&> render_texture::begin_render()
{
    for(auto&& data : m_frames_data)
        if(data.begin)
            return {data.buffer, data.signal};

    const auto find_data = [this]() -> frame_data&
    {
        for(auto&& data : m_frames_data)
            if(data.fence.try_wait())
                return data;

        return add_frame_data();
    };

    auto& data{find_data()};

    data.signal();
    data.signal.disconnect_all();
    data.begin = true;
    data.pool.reset();
    data.buffer = tph::cmd::begin(data.pool, tph::command_buffer_level::primary, tph::command_buffer_flags::one_time_submit);

    tph::cmd::begin_render_pass(data.buffer, get_target(), 0);

    return {data.buffer, data.signal};
}

void render_texture::present()
{
    const auto find_data = [this]() -> frame_data&
    {
        for(auto&& data : m_frames_data)
            if(data.begin)
                return data;

        throw std::runtime_error{"cpt::render_texture::present called before cpt::render_texture::begin_render"};
    };

    auto& data{find_data()};
    data.begin = false;

    engine::instance().flush_transfers();

    tph::cmd::end_render_pass(data.buffer);
    tph::cmd::end(data.buffer);

    data.fence.reset();

    tph::submit_info submit_info{};
    submit_info.command_buffers.push_back(data.buffer);

    std::unique_lock lock{engine::instance().submit_mutex()};
    tph::submit(engine::instance().renderer(), submit_info, data.fence);
    lock.unlock();
}

render_texture::frame_data& render_texture::add_frame_data()
{
    frame_data data{tph::command_pool{engine::instance().renderer()}, tph::command_buffer{}, tph::fence{engine::instance().renderer(), true}};

    return m_frames_data.emplace_back(std::move(data));
}

}
