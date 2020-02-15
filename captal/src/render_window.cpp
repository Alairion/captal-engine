#include "render_window.hpp"

#include "engine.hpp"

namespace cpt
{

void check_presentation_support(tph::surface& surface)
{
    if(!engine::instance().graphics_device().support_presentation(surface))
        throw std::runtime_error{"Device does not support presentation"};
}

tph::surface make_window_surface(apr::window& window)
{
    tph::application& application{engine::instance().application().graphics_application()};

    tph::surface output{tph::vulkan::surface{tph::underlying_cast<VkInstance>(application), window.make_surface(tph::underlying_cast<VkInstance>(application))}};
    check_presentation_support(output);

    return output;
}

render_window::render_window(const std::string& title, const cpt::video_mode& mode, apr::window_options options)
:apr::window{engine::instance().application().system_application(), title, mode.width, mode.height, options}
,tph::surface{make_window_surface(*this)}
,render_target{engine::instance().renderer(), static_cast<tph::surface&>(*this), mode.present_mode, mode.image_count, mode.target_options, mode.sample_count}
,m_video_mode{mode}
{
    prepare_frame_data();
    setup_signals();
}

render_window::render_window(const apr::monitor& monitor, const std::string& title, const cpt::video_mode& mode, apr::window_options options)
:apr::window{engine::instance().application().system_application(), monitor, title, mode.width, mode.height, options}
,tph::surface{make_window_surface(*this)}
,render_target{engine::instance().renderer(), static_cast<tph::surface&>(*this), mode.present_mode, mode.image_count, mode.target_options, mode.sample_count}
,m_video_mode{mode}
{
    prepare_frame_data();
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

std::pair<tph::command_buffer&, frame_presented_signal&> render_window::begin_render()
{
    frame_data& data{m_frames_data[m_frame_index]};

    if(data.begin)
        return {data.buffer, data.signal};

    data.fence.wait();
    data.signal();
    data.signal.disconnect_all();
    data.begin = true;
    data.pool.reset();
    data.buffer = tph::cmd::begin(data.pool, tph::command_buffer_level::primary, tph::command_buffer_flags::one_time_submit);

    tph::cmd::begin_render_pass(data.buffer, get_target(), m_frame_index);

    tph::render_target_status status{get_target().acquire(data.image_available, tph::nullref)};
    while(status == tph::render_target_status::out_of_date)
    {
        if(tph::surface::size(engine::instance().renderer()) == std::make_pair(0u, 0u))
        {
            disable_rendering();
            return {data.buffer, data.signal};
        }

        wait_all();
        get_target().recreate();

        data.pool.reset();
        data.buffer = tph::cmd::begin(data.pool, tph::command_buffer_level::primary, tph::command_buffer_flags::one_time_submit);
        tph::cmd::begin_render_pass(data.buffer, get_target(), m_frame_index);

        status = get_target().acquire(data.image_available, tph::nullref);
    }

    if(status == tph::render_target_status::surface_lost) //may happens on window closure
    {
        disable_rendering();
    }

    return {data.buffer, data.signal};
}

void render_window::present()
{
    frame_data& data{m_frames_data[m_frame_index]};

    data.begin = false;
    m_frame_index = (m_frame_index + 1) % m_video_mode.image_count;
    engine::instance().flush_transfers();

    tph::cmd::end_render_pass(data.buffer);
    tph::cmd::end(data.buffer);

    data.fence.reset();

    tph::submit_info submit_info{};
    submit_info.wait_semaphores.push_back(data.image_available);
    submit_info.wait_stages.push_back(tph::pipeline_stage::color_attachment_output);
    submit_info.command_buffers.push_back(data.buffer);
    submit_info.signal_semaphores.push_back(data.image_presentable);

    std::unique_lock lock{engine::instance().submit_mutex()};
    tph::submit(engine::instance().renderer(), submit_info, data.fence);
    lock.unlock();

    tph::render_target_status status{get_target().present(data.image_presentable)};
    if(status != tph::render_target_status::valid)
    {
        if(tph::surface::size(engine::instance().renderer()) == std::make_pair(0u, 0u))
        {
            disable_rendering();
            return;
        }

        wait_all();
        get_target().recreate();
    }
}

void render_window::prepare_frame_data()
{
    for(std::uint32_t i{}; i < m_video_mode.image_count; ++i)
    {
        frame_data data{tph::command_pool{engine::instance().renderer()}, tph::command_buffer{},
                        tph::semaphore{engine::instance().renderer()}, tph::semaphore{engine::instance().renderer()},
                        tph::fence{engine::instance().renderer(), true}};

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

    m_frame_index = 0;
}

}
