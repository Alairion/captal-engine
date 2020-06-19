#include "render_texture.hpp"

#include "engine.hpp"

namespace cpt
{

static tph::render_pass_info make_render_pass_info(const render_texture_info& info, bool has_sampling)
{
    const bool has_multisampling{info.sample_count != tph::sample_count::msaa_x1};
    const bool has_depth_stencil{info.depth_format != tph::texture_format::undefined};

    tph::render_pass_info output{};
    auto& subpass{output.subpasses.emplace_back()};

    auto& color_attachement{output.attachments.emplace_back()};
    color_attachement.format = info.format;
    color_attachement.sample_count = info.sample_count;
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
        depth_attachement.format = info.depth_format;
        depth_attachement.sample_count = info.sample_count;
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
        resolve_attachement.format = info.format;
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

    auto& dependency{output.dependencies.emplace_back()};
    dependency.source_subpass = tph::external_subpass;
    dependency.destination_subpass = 0;
    dependency.source_stage = tph::pipeline_stage::color_attachment_output;
    dependency.destination_stage = tph::pipeline_stage::color_attachment_output;
    dependency.source_access = tph::resource_access::none;
    dependency.destination_access = tph::resource_access::color_attachment_read | tph::resource_access::color_attachment_write;

    return output;
}

static tph::texture make_multisampling_texture(const render_texture_info& info)
{
    if(info.sample_count == tph::sample_count::msaa_x1)
    {
        return tph::texture{};
    }

    return tph::texture{engine::instance().renderer(), info.width, info.height, info.format, tph::texture_usage::color_attachment, info.sample_count};
}

static tph::texture make_depth_texture(const render_texture_info& info)
{
    if(info.depth_format == tph::texture_format::undefined)
    {
        return tph::texture{};
    }

    return tph::texture{engine::instance().renderer(), info.width, info.height, info.depth_format, tph::texture_usage::depth_stencil_attachment, info.sample_count};
}

static std::vector<std::reference_wrapper<tph::texture>> make_attachments(const render_texture_info& info, tph::texture& color, tph::texture& multisampling, tph::texture& depth)
{
    const bool has_multisampling{info.sample_count != tph::sample_count::msaa_x1};
    const bool has_depth_stencil{info.depth_format != tph::texture_format::undefined};

    std::vector<std::reference_wrapper<tph::texture>> output{};

    if(has_multisampling)
    {
        output.emplace_back(multisampling);

        if(has_depth_stencil)
        {
            output.emplace_back(depth);
        }

        output.emplace_back(color);
    }
    else
    {
        output.emplace_back(color);

        if(has_depth_stencil)
        {
            output.emplace_back(depth);
        }
    }

    return output;
}

render_texture::render_texture(const render_texture_info& info)
:texture{info.width, info.height, info.format, tph::texture_usage::color_attachment | tph::texture_usage::sampled}
,render_target{make_render_pass_info(info, false)}
,m_info{info}
,m_multisampling_texture{make_multisampling_texture(info)}
,m_depth_texture{make_depth_texture(info)}
,m_framebuffer{engine::instance().renderer(), get_render_pass(), make_attachments(info, get_texture(), m_multisampling_texture, m_depth_texture), info.width, info.height, 1}
{
    m_frames_data.reserve(8);
}

render_texture::render_texture(const render_texture_info& info, const tph::sampling_options& sampling)
:texture{info.width, info.height, sampling, info.format, tph::texture_usage::color_attachment | tph::texture_usage::sampled}
,render_target{make_render_pass_info(info, true)}
,m_info{info}
,m_multisampling_texture{make_multisampling_texture(info)}
,m_depth_texture{make_depth_texture(info)}
,m_framebuffer{engine::instance().renderer(), get_render_pass(), make_attachments(info, get_texture(), m_multisampling_texture, m_depth_texture), info.width, info.height, 1}
{
    m_frames_data.reserve(8);
}

render_texture::~render_texture()
{
    wait_all();
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

    const auto find_data = [this]() -> frame_data&
    {
        for(auto& data : m_frames_data)
        {
            if(data.fence.try_wait())
            {
                return data;
            }
        }

        return add_frame_data();
    };

    auto& data{find_data()};

    data.signal();
    data.signal.disconnect_all();
    data.begin = true;
    data.pool.reset();

    data.buffer = tph::cmd::begin(data.pool, tph::command_buffer_level::primary, tph::command_buffer_flags::one_time_submit);
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
    data.begin = false;

    engine::instance().flush_transfers();

    tph::cmd::end_render_pass(data.buffer);
    tph::cmd::end(data.buffer);

    data.fence.reset();

    tph::submit_info submit_info{};
    submit_info.command_buffers.emplace_back(data.buffer);

    std::unique_lock lock{engine::instance().submit_mutex()};
    tph::submit(engine::instance().renderer(), submit_info, data.fence);
    lock.unlock();
}

render_texture::frame_data& render_texture::add_frame_data()
{
    frame_data data{tph::command_pool{engine::instance().renderer()}, tph::command_buffer{}, tph::fence{engine::instance().renderer(), true}};

    return m_frames_data.emplace_back(std::move(data));
}

void render_texture::wait_all()
{
    for(frame_data& data : m_frames_data)
    {
        data.fence.wait();
        data.signal();
        data.signal.disconnect_all();
    }
}

}
