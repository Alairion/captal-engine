#include "render_texture.hpp"

#include "engine.hpp"

namespace cpt
{

static tph::render_pass_info make_render_pass_info(tph::texture_format color_format, bool has_sampling, tph::sample_count sample_count, tph::texture_format depth_format)
{
    const bool has_multisampling{sample_count != tph::sample_count::msaa_x1};
    const bool has_depth_stencil{depth_format != tph::texture_format::undefined};

    tph::render_pass_info output{};
    auto& subpass{output.subpasses.emplace_back()};

    auto& color_attachement{output.attachments.emplace_back()};
    color_attachement.format = color_format;
    color_attachement.sample_count = sample_count;
    color_attachement.load_op = tph::attachment_load_op::clear;

    if(has_multisampling)
    {
        color_attachement.store_op = tph::attachment_store_op::dont_care;
    }
    else
    {
        color_attachement.store_op = tph::attachment_store_op::store;
    }

    color_attachement.stencil_load_op = tph::attachment_load_op::clear;
    color_attachement.stencil_store_op = tph::attachment_store_op::dont_care;
    color_attachement.initial_layout = tph::texture_layout::undefined;

    if(has_multisampling)
    {
        color_attachement.final_layout = tph::texture_layout::color_attachment_optimal;
    }
    else if(has_sampling)
    {
        color_attachement.final_layout = tph::texture_layout::shader_read_only_optimal;
    }
    else
    {
        color_attachement.final_layout = tph::texture_layout::transfer_source_optimal;
    }

    subpass.color_attachments.emplace_back(tph::attachment_reference{0, tph::texture_layout::color_attachment_optimal});

    if(has_depth_stencil)
    {
        auto& depth_attachement{output.attachments.emplace_back()};
        depth_attachement.format = depth_format;
        depth_attachement.sample_count = sample_count;
        depth_attachement.load_op = tph::attachment_load_op::clear;
        depth_attachement.store_op = tph::attachment_store_op::dont_care;
        depth_attachement.stencil_load_op = tph::attachment_load_op::clear;
        depth_attachement.stencil_store_op = tph::attachment_store_op::dont_care;
        depth_attachement.initial_layout = tph::texture_layout::undefined;
        depth_attachement.final_layout = tph::texture_layout::depth_stencil_attachment_optimal;

        subpass.depth_attachment.emplace(tph::attachment_reference{1, tph::texture_layout::depth_stencil_attachment_optimal});
    }

    if(has_multisampling)
    {
        auto& resolve_attachement{output.attachments.emplace_back()};
        resolve_attachement.format = color_format;
        resolve_attachement.sample_count = tph::sample_count::msaa_x1;
        resolve_attachement.load_op = tph::attachment_load_op::clear;
        resolve_attachement.store_op = tph::attachment_store_op::store;
        resolve_attachement.stencil_load_op = tph::attachment_load_op::clear;
        resolve_attachement.stencil_store_op = tph::attachment_store_op::dont_care;
        resolve_attachement.initial_layout = tph::texture_layout::undefined;

        if(has_sampling)
        {
            resolve_attachement.final_layout = tph::texture_layout::shader_read_only_optimal;
        }
        else
        {
            resolve_attachement.final_layout = tph::texture_layout::transfer_source_optimal;
        }

        if(has_depth_stencil)
        {
            subpass.resolve_attachments.emplace_back(tph::attachment_reference{2, tph::texture_layout::color_attachment_optimal});
        }
        else
        {
            subpass.resolve_attachments.emplace_back(tph::attachment_reference{1, tph::texture_layout::color_attachment_optimal});
        }
    }

    return output;
}

static std::vector<render_texture_attachment> make_attachments(const render_texture_info& info, tph::sample_count sample_count, tph::texture_format depth_format)
{
    const bool has_multisampling{sample_count != tph::sample_count::msaa_x1};
    const bool has_depth_stencil{depth_format != tph::texture_format::undefined};

    std::vector<render_texture_attachment> output{};

    if(has_multisampling)
    {
        output.emplace_back(make_texture(info.width, info.height, tph::texture_info{info.format, tph::texture_usage::color_attachment, {}, sample_count}));

        if(has_depth_stencil)
        {
            output.emplace_back(make_texture(info.width, info.height, tph::texture_info{depth_format, tph::texture_usage::depth_stencil_attachment, {}, sample_count}));
        }

        output.emplace_back(current_target);
    }
    else
    {
        output.emplace_back(current_target);

        if(has_depth_stencil)
        {
            output.emplace_back(make_texture(info.width, info.height, tph::texture_info{depth_format, tph::texture_usage::depth_stencil_attachment, {}, sample_count}));
        }
    }

    return output;
}

static std::vector<std::reference_wrapper<tph::texture>> convert_framebuffer_attachments(std::span<const render_texture_attachment> attachments, tph::texture& current)
{
    std::vector<std::reference_wrapper<tph::texture>> output{};
    output.reserve(std::size(attachments));

    for(auto&& attachment : attachments)
    {
        if(std::holds_alternative<current_target_t>(attachment))
        {
            output.emplace_back(current);
        }
        else
        {
            output.emplace_back(std::get<texture_ptr>(attachment)->get_texture());
        }
    }

    return output;
}

render_texture::render_texture(const render_texture_info& info, const tph::render_pass_info& render_pass, std::vector<render_texture_attachment> attachments)
:texture{info.width, info.height, tph::texture_info{info.format, info.usage | tph::texture_usage::color_attachment | tph::texture_usage::sampled}}
,render_target{render_pass}
,m_attachments{std::move(attachments)}
,m_framebuffer{engine::instance().renderer(), get_render_pass(), convert_framebuffer_attachments(m_attachments, get_texture()), info.width, info.height, 1}
{
    m_frames_data.reserve(4);
}

render_texture::render_texture(const render_texture_info& info, const tph::sampling_options& sampling, const tph::render_pass_info& render_pass, std::vector<render_texture_attachment> attachments)
:texture{info.width, info.height, tph::texture_info{info.format, info.usage | tph::texture_usage::color_attachment | tph::texture_usage::sampled}, sampling}
,render_target{render_pass}
,m_attachments{std::move(attachments)}
,m_framebuffer{engine::instance().renderer(), get_render_pass(), convert_framebuffer_attachments(m_attachments, get_texture()), info.width, info.height, 1}
{
    m_frames_data.reserve(4);
}

render_texture::render_texture(const render_texture_info& info, tph::sample_count sample_count, tph::texture_format depth_format)
:texture{info.width, info.height, tph::texture_info{info.format, info.usage | tph::texture_usage::color_attachment | tph::texture_usage::sampled}}
,render_target{make_render_pass_info(info.format, false, sample_count, depth_format)}
,m_attachments{make_attachments(info, sample_count, depth_format)}
,m_framebuffer{engine::instance().renderer(), get_render_pass(), convert_framebuffer_attachments(m_attachments, get_texture()), info.width, info.height, 1}
{
    m_frames_data.reserve(4);
}

render_texture::render_texture(const render_texture_info& info, const tph::sampling_options& sampling, tph::sample_count sample_count, tph::texture_format depth_format)
:texture{info.width, info.height, tph::texture_info{info.format, info.usage | tph::texture_usage::color_attachment | tph::texture_usage::sampled}, sampling}
,render_target{make_render_pass_info(info.format, true, sample_count, depth_format)}
,m_attachments{make_attachments(info, sample_count, depth_format)}
,m_framebuffer{engine::instance().renderer(), get_render_pass(), convert_framebuffer_attachments(m_attachments, get_texture()), info.width, info.height, 1}
{
    m_frames_data.reserve(4);
}

render_texture::~render_texture()
{
    wait_all();
}

frame_time_signal& render_texture::register_frame_time()
{
    for(auto&& data : m_frames_data)
    {
        if(data.begin)
        {
            if(data.timed)
            {
                return data.time_signal;
            }
            else
            {
                throw std::runtime_error{"cpt::render_texture::register_frame_time first call is done after a call of cpt::render_texture::begin_render"};
            }
        }
    }

    auto& data{next_frame()};

    data.timed = true;

    tph::cmd::reset_query_pool(data.buffer, data.query_pool, 0, 2);
    tph::cmd::write_timestamp(data.buffer, data.query_pool, 0, tph::pipeline_stage::top_of_pipe);

    tph::cmd::begin_render_pass(data.buffer, get_render_pass(), m_framebuffer);

    return data.time_signal;
}

std::pair<tph::command_buffer&, frame_presented_signal&> render_texture::begin_render()
{
    for(auto&& data : m_frames_data)
    {
        if(data.begin)
        {
            return {data.buffer, data.signal};
        }
    }

    auto& data{next_frame()};

    tph::cmd::begin_render_pass(data.buffer, get_render_pass(), m_framebuffer);

    return {data.buffer, data.signal};
}

void render_texture::present()
{
    const auto find_data = [this]() -> frame_data&
    {
        for(auto& data : m_frames_data)
        {
            if(data.begin)
            {
                return data;
            }
        }

        throw std::runtime_error{"cpt::render_texture::present called before cpt::render_texture::begin_render"};
    };

    auto& data{find_data()};

    tph::cmd::end_render_pass(data.buffer);

    if(data.timed)
    {
        tph::cmd::write_timestamp(data.buffer, data.query_pool, 1, tph::pipeline_stage::bottom_of_pipe);
    }

    tph::cmd::end(data.buffer);

    engine::instance().flush_transfers();

    data.fence.reset();

    tph::submit_info submit_info{};
    submit_info.command_buffers.emplace_back(data.buffer);

    std::unique_lock lock{engine::instance().submit_mutex()};
    tph::submit(engine::instance().renderer(), submit_info, data.fence);
    lock.unlock();

    data.submited = true;
    data.begin = false;
}

render_texture::frame_data& render_texture::next_frame()
{
    const auto find_data = [this]() -> frame_data&
    {
        for(auto& data : m_frames_data)
        {
            if(data.fence.try_wait())
            {
                reset(data);

                return data;
            }
        }

        return add_frame_data();
    };

    auto& data{find_data()};

    data.buffer = tph::cmd::begin(data.pool, tph::command_buffer_level::primary, tph::command_buffer_flags::one_time_submit);
    data.begin = true;

    return data;
}

render_texture::frame_data& render_texture::add_frame_data()
{
    frame_data data
    {
        tph::command_pool{engine::instance().renderer()},
        tph::command_buffer{},
        tph::fence{engine::instance().renderer(), true},
        tph::query_pool{engine::instance().renderer(), 2, tph::query_type::timestamp}
    };

    return m_frames_data.emplace_back(std::move(data));
}

void render_texture::reset(frame_data& data)
{
    data.begin = false;
    data.submited = false;

    if(data.timed)
    {
        time_results(data);
    }

    data.signal();
    data.signal.disconnect_all();
    data.pool.reset();
}

void render_texture::time_results(frame_data& data)
{
    std::array<std::uint64_t, 2> results;
    data.query_pool.results(0, 2, std::size(results) * sizeof(std::uint64_t), std::data(results), sizeof(std::uint64_t), tph::query_results::uint64 | tph::query_results::wait);

    const auto period{static_cast<double>(cpt::engine::instance().graphics_device().limits().timestamp_period)};
    const frame_time_t time{static_cast<std::uint64_t>((results[1] - results[0]) * period)};

    data.timed = false;

    data.time_signal(time);
    data.time_signal.disconnect_all();
}

void render_texture::wait_all()
{
    for(frame_data& data : m_frames_data)
    {
        if(data.submited)
        {
            data.fence.wait();
        }
    }
}

}
