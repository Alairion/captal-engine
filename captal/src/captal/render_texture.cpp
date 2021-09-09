#include "render_texture.hpp"

#include "engine.hpp"

namespace cpt
{

static tph::render_pass_info make_render_pass_info(tph::texture_format color_format, tph::texture_layout final_layout, tph::sample_count sample_count, tph::texture_format depth_format)
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
    else
    {
        color_attachement.final_layout = final_layout;
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
        resolve_attachement.final_layout = final_layout;

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

static std::vector<texture_ptr> make_attachments(texture_ptr user, tph::sample_count sample_count, tph::texture_format depth_format)
{
    const bool has_multisampling{sample_count != tph::sample_count::msaa_x1};
    const bool has_depth_stencil{depth_format != tph::texture_format::undefined};

    const auto width {user->width()};
    const auto height{user->height()};

    std::vector<texture_ptr> output{};

    if(has_multisampling)
    {
        output.emplace_back(make_texture(width, height, tph::texture_info{.format = user->format(), .usage = tph::texture_usage::color_attachment, .sample_count = sample_count}));

        if(has_depth_stencil)
        {
            output.emplace_back(make_texture(width, height, tph::texture_info{.format = depth_format, .usage = tph::texture_usage::depth_stencil_attachment, .sample_count = sample_count}));
        }

        output.emplace_back(std::move(user));
    }
    else
    {
        output.emplace_back(std::move(user));

        if(has_depth_stencil)
        {
            output.emplace_back(make_texture(width, height, tph::texture_info{.format = depth_format, .usage = tph::texture_usage::depth_stencil_attachment, .sample_count = sample_count}));
        }
    }

    return output;
}

static std::vector<std::reference_wrapper<tph::texture_view>> convert_framebuffer_attachments(std::span<const texture_ptr> attachments)
{
    std::vector<std::reference_wrapper<tph::texture_view>> output{};
    output.reserve(std::size(attachments));

    for(auto&& attachment : attachments)
    {
        output.emplace_back(attachment->get_texture_view());
    }

    return output;
}

render_texture::render_texture(std::uint32_t width, std::uint32_t height, const tph::render_pass_info& render_pass, std::vector<texture_ptr> attachments)
:render_target{render_pass}
,m_attachments{std::move(attachments)}
,m_framebuffer{engine::instance().renderer(), get_render_pass(), convert_framebuffer_attachments(m_attachments), width, height, 1}
,m_pool{engine::instance().renderer(), tph::command_pool_options::reset}
{
    m_frames_data.reserve(4);
}

render_texture::render_texture(texture_ptr texture, tph::sample_count sample_count, tph::texture_format depth_format, tph::texture_layout final_layout)
:render_target{make_render_pass_info(texture->format(), final_layout, sample_count, depth_format)}
,m_attachments{make_attachments(texture, sample_count, depth_format)}
,m_framebuffer{engine::instance().renderer(), get_render_pass(), convert_framebuffer_attachments(m_attachments), texture->width(), texture->height(), 1}
,m_pool{engine::instance().renderer(), tph::command_pool_options::reset}
#ifdef CAPTAL_DEBUG
,m_own_attachments{true}
,m_has_multisampling{sample_count != tph::sample_count::msaa_x1}
,m_has_depth_stencil{depth_format != tph::texture_format::undefined}
#endif
{
    m_frames_data.reserve(4);
}

render_texture::~render_texture()
{
    wait();
}

std::optional<frame_render_info> render_texture::begin_render(begin_render_options options)
{
    if(m_data)
    {
        if(m_data->epoch == m_epoch)
        {
            return std::nullopt;
        }

        if(static_cast<bool>(options & begin_render_options::timed))
        {
            assert(m_data->timed && "cpt::render_texture::begin_render must not be called with begin_render_options::timed flag if initial call was made without.");

            return frame_render_info{m_data->buffer, m_data->signal, m_data->keeper, m_data->time_signal};
        }
        else
        {
            return frame_render_info{m_data->buffer, m_data->signal, m_data->keeper};
        }
    }

    if(static_cast<bool>(options & begin_render_options::reset))
    {
        ++m_epoch;
    }

    if(!next_frame())
    {
        return std::nullopt;
    }

    tph::cmd::begin(m_data->buffer, tph::command_buffer_reset_options::none);

    if(static_cast<bool>(options & begin_render_options::timed))
    {
        m_data->timed = true;

        tph::cmd::reset_query_pool(m_data->buffer, m_data->query_pool, 0, 2);
        tph::cmd::write_timestamp(m_data->buffer, m_data->query_pool, 0, tph::pipeline_stage::top_of_pipe);

        tph::cmd::begin_render_pass(m_data->buffer, get_render_pass(), m_framebuffer);

        return frame_render_info{m_data->buffer, m_data->signal, m_data->keeper, m_data->time_signal};
    }
    else
    {
        tph::cmd::begin_render_pass(m_data->buffer, get_render_pass(), m_framebuffer);

        return frame_render_info{m_data->buffer, m_data->signal, m_data->keeper};
    }
}

void render_texture::present()
{
    assert(m_data && "cpt::render_texture::present called without prior call to cpt::render_texture::begin_render.");

    tph::cmd::end_render_pass(m_data->buffer);

    if(m_data->timed)
    {
        tph::cmd::write_timestamp(m_data->buffer, m_data->query_pool, 1, tph::pipeline_stage::bottom_of_pipe);
    }

    tph::cmd::end(m_data->buffer);

    m_data->fence.reset();

    tph::submit_info submit_info{};
    submit_info.command_buffers.emplace_back(m_data->buffer);

    std::unique_lock lock{engine::instance().submit_mutex()};
    tph::submit(engine::instance().renderer(), submit_info, m_data->fence);
    lock.unlock();

    m_data->epoch = m_epoch;
    m_data->submitted = true;

    m_data = nullptr;
}

void render_texture::wait()
{
    for(frame_data& data : m_frames_data)
    {
        if(data.submitted)
        {
            data.fence.wait();

            if(data.timed)
            {
                time_results(data);
            }

            data.signal();
            data.signal.disconnect_all();
            data.keeper.clear();

            data.submitted = false;
        }
    }
}

#ifdef CAPTAL_DEBUG
void render_texture::set_name(std::string_view name)
{
    m_name = name;

    tph::set_object_name(engine::instance().renderer(), get_render_pass(), m_name + " render pass");
    tph::set_object_name(engine::instance().renderer(), m_pool, m_name + " command pool");
    tph::set_object_name(engine::instance().renderer(), m_framebuffer, m_name + " framebuffer");

    if(m_own_attachments)
    {
        if(m_has_multisampling)
        {
            tph::set_object_name(engine::instance().renderer(), m_attachments[0]->get_texture(), m_name + " multisampling attachment.");

            if(m_has_depth_stencil)
            {
                tph::set_object_name(engine::instance().renderer(), m_attachments[1]->get_texture(), m_name + " depth stencil attachment.");
            }
        }
        else if(m_has_depth_stencil)
        {
            tph::set_object_name(engine::instance().renderer(), m_attachments[1]->get_texture(), m_name + " depth stencil attachment.");
        }
    }

    for(std::size_t i{}; i < std::size(m_frames_data); ++i)
    {
        tph::set_object_name(engine::instance().renderer(), m_frames_data[i].buffer,     m_name + " frame #" + std::to_string(i) + " command buffer");
        tph::set_object_name(engine::instance().renderer(), m_frames_data[i].fence,      m_name + " frame #" + std::to_string(i) + " fence");
        tph::set_object_name(engine::instance().renderer(), m_frames_data[i].query_pool, m_name + " frame #" + std::to_string(i) + " query pool");
    }
}
#endif

void render_texture::time_results(frame_data& data)
{
    std::array<std::uint64_t, 2> results;
    data.query_pool.results(0, 2, std::size(results) * sizeof(std::uint64_t), std::data(results), sizeof(std::uint64_t), tph::query_results::uint64 | tph::query_results::wait);

    const auto         period{static_cast<double>(cpt::engine::instance().graphics_device().limits().timestamp_period)};
    const frame_time_t time  {static_cast<std::uint64_t>(static_cast<double>(results[1] - results[0]) * period)};

    data.time_signal(time);
}

void render_texture::flush_frame_data(frame_data& data)
{
    data.submitted = false;

    if(data.timed)
    {
        time_results(data);
    }

    data.signal();
}

void render_texture::reset_frame_data(frame_data& data)
{
    data.submitted = false;

    if(data.timed)
    {
        time_results(data);

        data.timed = false;
        data.time_signal.disconnect_all();
    }

    data.signal();
    data.signal.disconnect_all();

    data.keeper.clear();
}

bool render_texture::next_frame()
{
    for(auto& data : m_frames_data)
    {
        if(data.fence.try_wait())
        {
            m_data = &data;

            if(m_data->epoch == m_epoch)
            {
                flush_frame_data(*m_data);

                return false;
            }
            else
            {
                reset_frame_data(*m_data);

                return true;
            }
        }
    }

    m_data = &add_frame_data();

    return true;
}

render_texture::frame_data& render_texture::add_frame_data()
{
    frame_data data{};
    data.buffer = tph::cmd::begin(m_pool, tph::command_buffer_level::primary);
    data.fence = tph::fence{engine::instance().renderer(), true};
    data.query_pool = tph::query_pool{engine::instance().renderer(), 2, tph::query_type::timestamp};

#ifdef CAPTAL_DEBUG
    const std::size_t i{std::size(m_frames_data)};
    tph::set_object_name(engine::instance().renderer(), data.buffer,     m_name + " frame #" + std::to_string(i) + " command buffer");
    tph::set_object_name(engine::instance().renderer(), data.fence,      m_name + " frame #" + std::to_string(i) + " fence");
    tph::set_object_name(engine::instance().renderer(), data.query_pool, m_name + " frame #" + std::to_string(i) + " query pool");
#endif

    return m_frames_data.emplace_back(std::move(data));
}


}
