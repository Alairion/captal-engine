#include "render_window.hpp"

#include "engine.hpp"

namespace cpt
{

event_iterator::event_iterator(window& window, apr::event_mode mode)
:m_window{&window}
,m_iterator{engine::instance().application().system_application(), window.get_window(), mode}
{

}

event_iterator& event_iterator::operator++()
{
    ++m_iterator;

    if(m_iterator != apr::event_iterator{})
    {
        const apr::event event{*m_iterator};

        if(std::holds_alternative<apr::window_event>(event))
        {
            const auto& window_event{std::get<apr::window_event>(event)};

            if(window_event.type == apr::window_event::minimized)
            {
                m_window->m_renderable = false;
            }
            else if(window_event.type == apr::window_event::restored)
            {
                m_window->m_renderable = true;
            }
            else if(window_event.type == apr::window_event::closed)
            {
                m_window->close();
            }
        }
    }

    return *this;
}

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

static tph::swapchain_info make_swapchain_info(const cpt::video_mode& info, const tph::surface_capabilities& capabilities, tph::texture_format surface_format)
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

window::window(const std::string& title, std::uint32_t width, std::uint32_t height, const cpt::video_mode& mode, apr::window_options options)
:window{engine::instance().application().system_application().main_monitor(), title, width, height, mode, options}
{

}

window::window(const apr::monitor& monitor, const std::string& title, std::uint32_t width, std::uint32_t height, const cpt::video_mode& mode, apr::window_options options)
:window{apr::window{engine::instance().application().system_application(), monitor, title, width, height, options}, mode}
{

}

window::window(apr::window window, const cpt::video_mode& mode)
:apr::window{std::move(window)}
,m_video_mode{mode}
,m_surface_format{choose_surface_format(m_surface)}
,m_surface{make_window_surface(get_window())}
,m_swapchain{engine::instance().renderer(), m_surface, make_swapchain_info(mode, m_surface.capabilities(engine::instance().renderer()), m_surface_format)}
{

}

void window::dispatch_events()
{
    for(auto&& event : event_iterator{*this})
    {
        dispatch_event(event);
    }
}

void window::discard_events()
{
    for(auto&& event [[maybe_unused]] : event_iterator{*this})
    {

    }
}

void window::dispatch_event(const apr::event& event)
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
            m_close(window_event);
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

void window::close()
{
     m_closed = true;
     m_renderable = false;
}

}
