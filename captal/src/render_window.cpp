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

    auto& dependency{output.dependencies.emplace_back()};
    dependency.source_subpass = tph::external_subpass;
    dependency.destination_subpass = 0;
    dependency.source_stage = tph::pipeline_stage::color_attachment_output;
    dependency.destination_stage = tph::pipeline_stage::color_attachment_output;
    dependency.source_access = tph::resource_access::none;
    dependency.destination_access = tph::resource_access::color_attachment_read | tph::resource_access::color_attachment_write;

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

    return tph::texture{engine::instance().renderer(), info.width, info.height, surface_format, tph::texture_usage::color_attachment, info.sample_count};
}

static tph::texture make_depth_texture(const video_mode& info)
{
    if(info.depth_format == tph::texture_format::undefined)
    {
        return tph::texture{};
    }

    return tph::texture{engine::instance().renderer(), info.width, info.height, info.depth_format, tph::texture_usage::depth_stencil_attachment, info.sample_count};
}

static std::vector<std::reference_wrapper<tph::texture>> make_attachments(const video_mode& info, tph::texture& color, tph::texture& multisampling, tph::texture& depth)
{
    const bool has_multisampling{info.sample_count != tph::sample_count::msaa_x1};
    const bool has_depth_stencil{info.depth_format != tph::texture_format::undefined};

    std::vector<std::reference_wrapper<tph::texture>> output{};

    if(has_multisampling)
    {
        output.push_back(multisampling);

        if(has_depth_stencil)
        {
            output.push_back(depth);
        }

        output.push_back(color);
    }
    else
    {
        output.push_back(color);

        if(has_depth_stencil)
        {
            output.push_back(depth);
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
    wait_all();
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

std::pair<tph::command_buffer&, frame_presented_signal&> render_window::begin_render()
{
    frame_data& data{m_frames_data[m_frame_index]};

    if(data.begin)
    {
        return {data.buffer, data.signal};
    }

    data.fence.wait();
    data.signal();
    data.signal.disconnect_all();
    data.begin = true;
    data.pool.reset();
    data.buffer = tph::cmd::begin(data.pool, tph::command_buffer_level::primary, tph::command_buffer_flags::one_time_submit);

    tph::cmd::begin_render_pass(data.buffer, get_render_pass(), data.framebuffer);

    auto status{m_swapchain.acquire(data.image_available, tph::nullref)};
    while(status == tph::swapchain_status::out_of_date)
    {
        const auto capabilities{tph::surface::capabilities(engine::instance().renderer())};
        if(capabilities.current_width == 0 && capabilities.current_height == 0)
        {
            disable_rendering();
            return {data.buffer, data.signal};
        }

        wait_all();
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

    return {data.buffer, data.signal};
}

void render_window::present()
{
    engine::instance().flush_transfers();

    frame_data& data{m_frames_data[m_frame_index]};
    data.begin = false;

    m_frame_index = (m_frame_index + 1) % m_swapchain.info().image_count;

    tph::cmd::end_render_pass(data.buffer);
    tph::cmd::end(data.buffer);

    tph::submit_info submit_info{};
    submit_info.wait_semaphores.push_back(data.image_available);
    submit_info.wait_stages.push_back(tph::pipeline_stage::color_attachment_output);
    submit_info.command_buffers.push_back(data.buffer);
    submit_info.signal_semaphores.push_back(data.image_presentable);

    data.fence.reset();

    std::unique_lock lock{engine::instance().submit_mutex()};
    tph::submit(engine::instance().renderer(), submit_info, data.fence);
    lock.unlock();

    if(m_swapchain.present(data.image_presentable) != tph::swapchain_status::valid)
    {
        const auto capabilities{tph::surface::capabilities(engine::instance().renderer())};
        if(capabilities.current_width == 0 && capabilities.current_height == 0)
        {
            disable_rendering();
            return;
        }

        wait_all();
        recreate(capabilities);
    }
}

void render_window::setup_frame_data()
{
    for(std::uint32_t i{}; i < m_swapchain.info().image_count; ++i)
    {
        const auto attachments{make_attachments(m_video_mode, m_swapchain.texture(i), m_multisampling_texture, m_depth_texture)};

        frame_data data{};
        data.framebuffer = tph::framebuffer{engine::instance().renderer(), get_render_pass(), attachments, m_swapchain.info().width, m_swapchain.info().height, 1};
        data.pool = tph::command_pool{engine::instance().renderer()};
        data.image_available = tph::semaphore{engine::instance().renderer()};
        data.image_presentable = tph::semaphore{engine::instance().renderer()};
        data.fence = tph::fence{engine::instance().renderer()};
        data.begin = true;

        m_frames_data.push_back(std::move(data));
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

void render_window::wait_all()
{
    for(frame_data& data : m_frames_data)
    {
        data.fence.wait();
        data.signal();
        data.signal.disconnect_all();
    }
}

void render_window::recreate(const tph::surface_capabilities& capabilities)
{
    m_frame_index = 0;
    m_swapchain = tph::swapchain{engine::instance().renderer(), get_surface(), make_swapchain_info(m_video_mode, capabilities, m_surface_format), m_swapchain};

    for(std::uint32_t i{}; i < m_swapchain.info().image_count; ++i)
    {
        const auto attachments{make_attachments(m_video_mode, m_swapchain.texture(i), m_multisampling_texture, m_depth_texture)};

        frame_data& data{m_frames_data[i]};
        data.framebuffer = tph::framebuffer{engine::instance().renderer(), get_render_pass(), attachments, m_swapchain.info().width, m_swapchain.info().height, 1};
    }
}

}
