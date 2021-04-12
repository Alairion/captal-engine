#include "window.hpp"

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

window::window(const std::string& title, std::uint32_t width, std::uint32_t height, apr::window_options options)
:window{engine::instance().application().system_application().main_monitor(), title, width, height, options}
{

}

window::window(const apr::monitor& monitor, const std::string& title, std::uint32_t width, std::uint32_t height, apr::window_options options)
:apr::window{engine::instance().application().system_application(), monitor, title, width, height, options}
,m_surface{make_window_surface(get_window())}
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
            m_gained_focus(*this, window_event);
        }
        else if(window_event.type == apr::window_event::lost_focus)
        {
            m_lost_focus(*this, window_event);
        }
        else if(window_event.type == apr::window_event::mouse_entered)
        {
            m_mouse_entered(*this, window_event);
        }
        else if(window_event.type == apr::window_event::mouse_left)
        {
            m_mouse_left(*this, window_event);
        }
        else if(window_event.type == apr::window_event::moved)
        {
            m_moved(*this, window_event);
        }
        else if(window_event.type == apr::window_event::resized)
        {
            m_resized(*this, window_event);
        }
        else if(window_event.type == apr::window_event::minimized)
        {
            m_minimized(*this, window_event);
        }
        else if(window_event.type == apr::window_event::maximized)
        {
            m_maximized(*this, window_event);
        }
        else if(window_event.type == apr::window_event::restored)
        {
            m_restored(*this, window_event);
        }
        else if(window_event.type == apr::window_event::closed)
        {
            m_close(*this, window_event);
        }
    }
    else if(std::holds_alternative<apr::mouse_event>(event))
    {
        const auto& mouse_event{std::get<apr::mouse_event>(event)};

        if(mouse_event.type == apr::mouse_event::button_pressed)
        {
            m_mouse_button_pressed(*this, mouse_event);
        }
        else if(mouse_event.type == apr::mouse_event::button_released)
        {
            m_mouse_button_released(*this, mouse_event);
        }
        else if(mouse_event.type == apr::mouse_event::moved)
        {
            m_mouse_moved(*this, mouse_event);
        }
        else if(mouse_event.type == apr::mouse_event::wheel_scroll)
        {
            m_mouse_wheel_scroll(*this, mouse_event);
        }
    }
    else if(std::holds_alternative<apr::keyboard_event>(event))
    {
        const auto& keyboard_event{std::get<apr::keyboard_event>(event)};

        if(keyboard_event.type == apr::keyboard_event::key_pressed)
        {
            m_key_pressed(*this, keyboard_event);
        }
        else if(keyboard_event.type == apr::keyboard_event::key_released)
        {
            m_key_released(*this, keyboard_event);
        }
    }
    else if(std::holds_alternative<apr::text_event>(event))
    {
        const auto& text_event{std::get<apr::text_event>(event)};

        if(text_event.type == apr::text_event::text_entered)
        {
            m_text_entered(*this, text_event);
        }
    }
}

#ifdef CAPTAL_DEBUG
void window::set_name(std::string_view name)
{
    tph::set_object_name(engine::instance().renderer(), m_surface, std::string{name} + " surface");
}
#endif

event_iterator::event_iterator(window_ptr window, apr::event_mode mode)
:apr::event_iterator{engine::instance().application().system_application(), window->get_window(), mode}
{

}

event_iterator::event_iterator(window& window, apr::event_mode mode)
:apr::event_iterator{engine::instance().application().system_application(), window.get_window(), mode}
{

}

}
