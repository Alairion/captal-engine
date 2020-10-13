#include "render_window.hpp"

#include "engine.hpp"

namespace cpt
{

static void check_presentation_support(tph::surface& surface)
{
    if(!engine::instance().graphics_device().support_presentation(surface))
        throw std::runtime_error{"Device does not support presentation"};
}

static tph::surface make_window_surface(apr::window& window)
{
    tph::application& application{engine::instance().application().graphics_application()};

    tph::surface output{tph::vulkan::surface{tph::underlying_cast<VkInstance>(application), window.make_surface(tph::underlying_cast<VkInstance>(application))}};
    check_presentation_support(output);

    return output;
}

static tph::texture_format choose_surface_format(tph::surface& surface)
{
    const auto formats{surface.formats(engine::instance().graphics_device())};

    if(std::size(formats) == 1 && formats[0] == tph::texture_format::undefined)
    {
        return tph::texture_format::r8g8b8a8_srgb;
    }

    for(auto format : formats)
    {
        if(format == tph::texture_format::b8g8r8a8_srgb)
        {
            return format;
        }
        else if(format == tph::texture_format::r8g8b8a8_srgb)
        {
            return format;
        }
    }

    for(auto format : formats)
    {
        if(format == tph::texture_format::b8g8r8a8_unorm)
        {
            return format;
        }
        else if(format == tph::texture_format::b8g8r8a8_unorm)
        {
            return format;
        }
    }

    return formats[0];
}

static tph::render_pass_info make_render_pass_info(const video_mode& info, tph::texture_format color_format)
{
    const bool has_multisampling{info.sample_count != tph::sample_count::msaa_x1};
    const bool has_depth_stencil{info.depth_format != tph::texture_format::undefined};

    tph::render_pass_info output{};
    auto& subpass{output.subpasses.emplace_back()};

    auto& color_attachement{output.attachments.emplace_back()};
    color_attachement.format = color_format;
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
    else
    {
        color_attachement.final_layout = tph::texture_layout::present_source;
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
        resolve_attachement.format = color_format;
        resolve_attachement.sample_count = tph::sample_count::msaa_x1;
        resolve_attachement.load_op = tph::attachment_load_op::clear;
        resolve_attachement.store_op = tph::attachment_store_op::store;
        resolve_attachement.stencil_load_op = tph::attachment_load_op::clear;
        resolve_attachement.stencil_store_op = tph::attachment_store_op::dont_care;
        resolve_attachement.initial_layout = tph::texture_layout::undefined;
        resolve_attachement.final_layout = tph::texture_layout::present_source;

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

static tph::swapchain_info make_swapchain_info(const video_mode& info, const tph::surface_capabilities& capabilities, tph::texture_format surface_format)
{
    tph::swapchain_info output{};
    output.image_count = info.image_count;
    output.width = capabilities.current_width;
    output.height = capabilities.current_height;
    output.format = surface_format;
    output.transform = capabilities.current_transform;
    output.present_mode = info.present_mode;
    output.clipping = info.clipping;

    return output;
}

static tph::texture make_multisampling_texture(const video_mode& info, tph::texture_format surface_format)
{
    if(info.sample_count == tph::sample_count::msaa_x1)
    {
        return tph::texture{};
    }

    return tph::texture{engine::instance().renderer(), info.width, info.height, tph::texture_info{surface_format, tph::texture_usage::color_attachment, {}, info.sample_count}};
}

static tph::texture make_depth_texture(const video_mode& info)
{
    if(info.depth_format == tph::texture_format::undefined)
    {
        return tph::texture{};
    }

    return tph::texture{engine::instance().renderer(), info.width, info.height, tph::texture_info{info.depth_format, tph::texture_usage::depth_stencil_attachment, {}, info.sample_count}};
}

static std::vector<std::reference_wrapper<tph::texture>> make_attachments(const video_mode& info, tph::texture& color, tph::texture& multisampling, tph::texture& depth)
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

render_window::render_window(const std::string& title, const cpt::video_mode& mode, apr::window_options options)
:render_window{engine::instance().application().system_application().main_monitor(), title, mode, options}
{

}

render_window::render_window(const apr::monitor& monitor, const std::string& title, const cpt::video_mode& mode, apr::window_options options)
:apr::window{engine::instance().application().system_application(), monitor, title, mode.width, mode.height, options}
,tph::surface{make_window_surface(get_window())}
,render_target{make_render_pass_info(mode, choose_surface_format(get_surface()))}
,m_surface_format{choose_surface_format(get_surface())}
,m_swapchain{engine::instance().renderer(), get_surface(), make_swapchain_info(mode, tph::surface::capabilities(engine::instance().renderer()), m_surface_format)}
,m_multisampling_texture{make_multisampling_texture(mode, m_surface_format)}
,m_depth_texture{make_depth_texture(mode)}
,m_video_mode{mode}
{
    setup_frame_data();
    setup_signals();
}

render_window::~render_window()
{
    for(frame_data& data : m_frames_data)
    {
        if(data.submitted)
        {
            data.fence.wait();
        }
    }
}

void render_window::update()
{
    for(auto&& event : apr::event_iterator{engine::instance().application().system_application(), *this})
    {
        if(std::holds_alternative<apr::window_event>(event))
        {
            const auto& window_event{std::get<apr::window_event>(event)};

            if(window_event.type == apr::window_event::gained_focus)
            {
                m_gained_focus(window_event);
            }
            else if(window_event.type == apr::window_event::lost_focus)
            {
                m_lost_focus(window_event);
            }
            else if(window_event.type == apr::window_event::mouse_entered)
            {
                m_mouse_entered(window_event);
            }
            else if(window_event.type == apr::window_event::mouse_left)
            {
                m_mouse_left(window_event);
            }
            else if(window_event.type == apr::window_event::moved)
            {
                m_moved(window_event);
            }
            else if(window_event.type == apr::window_event::resized)
            {
                m_resized(window_event);
            }
            else if(window_event.type == apr::window_event::minimized)
            {
                m_minimized(window_event);
            }
            else if(window_event.type == apr::window_event::maximized)
            {
                m_maximized(window_event);
            }
            else if(window_event.type == apr::window_event::restored)
            {
                m_restored(window_event);
            }
            else if(window_event.type == apr::window_event::closed)
            {
                m_closed = true;
                m_close(window_event);
                break;
            }
        }
        else if(std::holds_alternative<apr::mouse_event>(event))
        {
            const auto& mouse_event{std::get<apr::mouse_event>(event)};

            if(mouse_event.type == apr::mouse_event::button_pressed)
            {
                m_mouse_button_pressed(mouse_event);
            }
            else if(mouse_event.type == apr::mouse_event::button_released)
            {
                m_mouse_button_released(mouse_event);
            }
            else if(mouse_event.type == apr::mouse_event::moved)
            {
                m_mouse_moved(mouse_event);
            }
            else if(mouse_event.type == apr::mouse_event::wheel_scroll)
            {
                m_mouse_wheel_scroll(mouse_event);
            }
        }
        else if(std::holds_alternative<apr::keyboard_event>(event))
        {
            const auto& keyboard_event{std::get<apr::keyboard_event>(event)};

            if(keyboard_event.type == apr::keyboard_event::key_pressed)
            {
                m_key_pressed(keyboard_event);
            }
            else if(keyboard_event.type == apr::keyboard_event::key_released)
            {
                m_key_released(keyboard_event);
            }
        }
        else if(std::holds_alternative<apr::text_event>(event))
        {
            const auto& text_event{std::get<apr::text_event>(event)};

            if(text_event.type == apr::text_event::text_entered)
            {
                m_text_entered(text_event);
            }
        }
    }
}

void render_window::close()
{
     m_closed = true;
     m_close(apr::window_event{apr::window_event::closed, id()});
     disable_rendering();
}

frame_time_signal& render_window::register_frame_time()
{
    frame_data& data{m_frames_data[m_frame_index]};

    if(data.begin)
    {
        if(data.timed)
        {
            return data.time_signal;
        }
        else
        {
            throw std::runtime_error{"cpt::render_window::register_frame_time first call is done after a call of cpt::render_window::begin_render"};
        }
    }

    reset(data);

    data.timed = true;

    tph::cmd::reset_query_pool(data.buffer, data.query_pool, 0, 2);
    tph::cmd::write_timestamp(data.buffer, data.query_pool, 0, tph::pipeline_stage::top_of_pipe);

    begin_render_impl(data);

    return data.time_signal;
}

frame_render_info render_window::begin_render()
{
    frame_data& data{m_frames_data[m_frame_index]};

    if(data.begin)
    {
        return frame_render_info{data.buffer, data.signal, data.keeper};
    }

    reset(data);
    begin_render_impl(data);

    return frame_render_info{data.buffer, data.signal, data.keeper};
}

void render_window::present()
{
    frame_data& data{m_frames_data[m_frame_index]};

    m_frame_index = (m_frame_index + 1) % m_swapchain.info().image_count;

    tph::cmd::end_render_pass(data.buffer);

    if(data.timed)
    {
        tph::cmd::write_timestamp(data.buffer, data.query_pool, 1, tph::pipeline_stage::bottom_of_pipe);
    }

    tph::cmd::end(data.buffer);

    engine::instance().flush_transfers();

    tph::submit_info submit_info{};
    submit_info.wait_semaphores.emplace_back(data.image_available);
    submit_info.wait_stages.emplace_back(tph::pipeline_stage::color_attachment_output);
    submit_info.command_buffers.emplace_back(data.buffer);
    submit_info.signal_semaphores.emplace_back(data.image_presentable);

    data.fence.reset();

    std::unique_lock lock{engine::instance().submit_mutex()};
    tph::submit(engine::instance().renderer(), submit_info, data.fence);
    lock.unlock();

    data.begin = false;
    data.submitted = true;

    if(m_swapchain.present(data.image_presentable) != tph::swapchain_status::valid)
    {
        const auto capabilities{tph::surface::capabilities(engine::instance().renderer())};
        if(capabilities.current_width == 0 && capabilities.current_height == 0)
        {
            disable_rendering();
            return;
        }

        recreate(capabilities);
    }
}

void render_window::setup_frame_data()
{
    for(std::uint32_t i{}; i < m_swapchain.info().image_count; ++i)
    {
        const auto attachments{make_attachments(m_video_mode, m_swapchain.textures()[i], m_multisampling_texture, m_depth_texture)};

        frame_data data{};
        data.framebuffer = tph::framebuffer{engine::instance().renderer(), get_render_pass(), attachments, m_swapchain.info().width, m_swapchain.info().height, 1};
        data.pool = tph::command_pool{engine::instance().renderer()};
        data.image_available = tph::semaphore{engine::instance().renderer()};
        data.image_presentable = tph::semaphore{engine::instance().renderer()};
        data.fence = tph::fence{engine::instance().renderer(), true};
        data.query_pool = tph::query_pool{engine::instance().renderer(), 2, tph::query_type::timestamp};

        m_frames_data.emplace_back(std::move(data));
    }
}

void render_window::setup_signals()
{
    m_minimized.connect([this](const apr::window_event&)
    {
        disable_rendering();
    });

    m_restored.connect([this](const apr::window_event&)
    {
        enable_rendering();
    });
}

void render_window::update_clear_values(tph::framebuffer& framebuffer)
{
    const bool has_multisampling{m_video_mode.sample_count != tph::sample_count::msaa_x1};
    const bool has_depth_stencil{m_video_mode.depth_format != tph::texture_format::undefined};

    framebuffer.set_clear_value(0, m_clear_color);

    if(has_depth_stencil)
    {
        framebuffer.set_clear_value(1, m_clear_depth_stencil);

        if(has_multisampling)
        {
            framebuffer.set_clear_value(2, m_clear_color);
        }
    }
    else if(has_multisampling)
    {
        framebuffer.set_clear_value(1, m_clear_color);
    }
}

void render_window::begin_render_impl(frame_data& data)
{
    update_clear_values(data.framebuffer);
    tph::cmd::begin_render_pass(data.buffer, get_render_pass(), data.framebuffer);

    auto status{m_swapchain.acquire(data.image_available, tph::nullref)};

    //We continue the normal path even if the swapchain is suboptimal
    //The present function will recreate it after presentation (if possible)
    while(status == tph::swapchain_status::out_of_date)
    {
        const auto capabilities{tph::surface::capabilities(engine::instance().renderer())};
        if(capabilities.current_width == 0 && capabilities.current_height == 0)
        {
            disable_rendering();
            return;
        }

        recreate(capabilities);

        data.pool.reset();
        data.buffer = tph::cmd::begin(data.pool, tph::command_buffer_level::primary, tph::command_buffer_flags::one_time_submit);

        tph::cmd::begin_render_pass(data.buffer, get_render_pass(), data.framebuffer);

        status = m_swapchain.acquire(data.image_available, tph::nullref);
    }

    if(status == tph::swapchain_status::surface_lost) //may happens on window closure
    {
        disable_rendering();
    }
}

void render_window::time_results(frame_data& data)
{
    std::array<std::uint64_t, 2> results;
    data.query_pool.results(0, 2, std::size(results) * sizeof(std::uint64_t), std::data(results), sizeof(std::uint64_t), tph::query_results::uint64 | tph::query_results::wait);

    const auto period{static_cast<double>(cpt::engine::instance().graphics_device().limits().timestamp_period)};
    const frame_time_t time{static_cast<std::uint64_t>((results[1] - results[0]) * period)};

    data.timed = false;

    data.time_signal(time);
    data.time_signal.disconnect_all();
}

void render_window::reset(frame_data& data)
{
    data.fence.wait();
    data.submitted = false;

    if(data.timed)
    {
        time_results(data);
    }

    data.signal();
    data.signal.disconnect_all();
    data.keeper.clear();
    data.pool.reset();

    data.buffer = tph::cmd::begin(data.pool, tph::command_buffer_level::primary, tph::command_buffer_flags::one_time_submit);
    data.begin = true;
}

void render_window::wait_all()
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
        }

        data.begin = false;
        data.submitted = false;
    }
}

void render_window::recreate(const tph::surface_capabilities& capabilities)
{
    wait_all();

    m_frame_index = 0;
    m_video_mode.width = capabilities.current_width;
    m_video_mode.height = capabilities.current_height;
    m_swapchain = tph::swapchain{engine::instance().renderer(), get_surface(), make_swapchain_info(m_video_mode, capabilities, m_surface_format), m_swapchain};
    m_multisampling_texture = make_multisampling_texture(m_video_mode, m_surface_format);
    m_depth_texture = make_depth_texture(m_video_mode);

    if(std::size(m_frames_data) != m_swapchain.info().image_count)
    {
        m_frames_data.clear();
        setup_frame_data();
    }
    else
    {
        for(std::uint32_t i{}; i < m_swapchain.info().image_count; ++i)
        {
            const auto attachments{make_attachments(m_video_mode, m_swapchain.textures()[i], m_multisampling_texture, m_depth_texture)};

            frame_data& data{m_frames_data[i]};
            data.framebuffer = tph::framebuffer{engine::instance().renderer(), get_render_pass(), attachments, m_swapchain.info().width, m_swapchain.info().height, 1};
        }
    }
}

}
