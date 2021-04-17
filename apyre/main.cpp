#include <iostream>

#include <apyre/application.hpp>
#include <apyre/window.hpp>
#include <apyre/messagebox.hpp>
#include <apyre/inputs.hpp>

using namespace apr::enum_operations;

int main()
{
    //Initialize Apyre.
    apr::application application{apr::application_extension::extended_client_area};

    //Create a window.
    //The parameters are pretty simple, the app, the window's title, it's size (width then height), and some additionnal options.
    apr::window window{application, "Test window", 640, 480, apr::window_options::extended_client_area};

    apr::cursor cursor{application, apr::system_cursor::crosshair};
    cursor.activate();

    //Let's play with monitors, we can grab on which monitor the windows actually is.
    const apr::monitor& monitor{window.current_monitor()};

    //Display some informations.
    std::cout << "Window is on monitor: " << monitor.name() << '.' << std::endl;
    std::cout << "It is located at (" << monitor.x() << "; " << monitor.y() << ") on the virtual screen." << std::endl;
    std::cout << "It has a size of " << monitor.width() << "x" << monitor.height() << "px." << std::endl;
    std::cout << "It has dpi of " << monitor.horizontal_dpi() << "x" << monitor.vertical_dpi() << "." << std::endl;
    std::cout << "It has refresh rate of " << monitor.frequency() << "Hz." << std::endl;

    if(monitor.is_main_monitor())
    {
        std::cout << "It is the main monitor." << std::endl;
    }
    else
    {
        std::cout << "It is not the main monitor." << std::endl;
    }

    std::string text{};
    bool relative_mouse{};

    apr::begin_text_input(application);

    window.change_hit_test_function([](std::int32_t x, std::int32_t y) -> apr::hit_test_result
    {
        if(x < 5 && y < 5)
        {
            return apr::hit_test_result::resize_topleft;
        }

        return apr::hit_test_result::normal;
    });

    //Here is the event loop.
    //The event iterator will wait indefinitly for window's event, due to apr::event_mode::wait.
    //There is also a polling mode (apr::event_mode::poll) that only runs the loop for available events, then leave it immediately.
    for(auto&& event : apr::event_iterator{application, window, apr::event_mode::wait})
    {
        //apr::event is a std::variant of various apr::xxx_event, so you may use it as you want (visitor pattern, holds_alternative, ...)
        if(std::holds_alternative<apr::window_event>(event))
        {
            auto&& windowevent{std::get<apr::window_event>(event)};

            //Since the loop will not stop by itself, we must break it at some point.
            if(windowevent.type == apr::window_event::closed)
            {
                break;
            }
            else if(windowevent.type == apr::window_event::resized)
            {
                std::cout << "Window resized: " << windowevent.width << "; " << windowevent.height << std::endl;
            }
        }
        else if(std::holds_alternative<apr::keyboard_event>(event))
        {
            auto&& keyevent{std::get<apr::keyboard_event>(event)};

            //Some user interactions
            if(keyevent.type == apr::keyboard_event::key_pressed && keyevent.scan == apr::scancode::escape)
            {
                //Message box buttons, they all have an ID, a text, and optionally a default binding on escape or enter.
                const std::array buttons{apr::message_box_button{0, "It was good!"}, apr::message_box_button{1, "It wasn't great."}};

                //This function is blocking.
                //It will return the button clicked by the user (based on buttons id), or apr::no_selection if they closed the message box.
                const auto id{apr::message_box(window, apr::message_box_type::warning, "Hello!", "Did you have a good day?", buttons)};

                if(id == 0)
                {
                    apr::message_box(window, apr::message_box_type::information, "Cool!", "I hope you like Apyre!");
                }
                else if(id == 1)
                {
                    apr::message_box(window, apr::message_box_type::information, "Oh.", "Don't worry, tomorrow will be a better day! <3");
                }
                else //id == apr::no_selection
                {
                    apr::message_box(window, apr::message_box_type::information, "Hum...", "Kinda shy aren't you?");
                }
            }
            else if(keyevent.type == apr::keyboard_event::key_pressed && keyevent.scan == apr::scancode::backspace)
            {
                if(!std::empty(text))
                {
                    text.pop_back();

                    std::cout << ">" << text << std::endl;
                }
            }
            else if(keyevent.type == apr::keyboard_event::key_pressed && keyevent.scan == apr::scancode::f1)
            {
                window.switch_to_fullscreen();

                std::cout << "Window switched to fullscreen" << std::endl;
            }
            else if(keyevent.type == apr::keyboard_event::key_pressed && keyevent.scan == apr::scancode::f2)
            {
                window.switch_to_windowed_fullscreen();

                std::cout << "Window switched to windowed fullscreen" << std::endl;
            }
            else if(keyevent.type == apr::keyboard_event::key_pressed && keyevent.scan == apr::scancode::f3)
            {
                window.switch_to_windowed();

                std::cout << "Window switched to windowed" << std::endl;
            }
            else if(keyevent.type == apr::keyboard_event::key_pressed && keyevent.scan == apr::scancode::enter)
            {
                std::cout << "Window size: " << window.width() << "; " << window.height() << std::endl;
            }
        }
        else if(std::holds_alternative<apr::mouse_event>(event))
        {
            auto&& mouseevent{std::get<apr::mouse_event>(event)};

            if(mouseevent.type == apr::mouse_event::button_pressed)
            {
                if(static_cast<bool>(mouseevent.button & apr::mouse_button::right))
                {
                    if(apr::is_cursor_visible(application))
                    {
                        apr::hide_cursor(application);
                    }
                    else
                    {
                        apr::show_cursor(application);
                    }
                }
                else if(static_cast<bool>(mouseevent.button & apr::mouse_button::left))
                {
                    apr::move_mouse(application, 5, -3);
                }
                else if(static_cast<bool>(mouseevent.button & apr::mouse_button::middle))
                {
                    if(!relative_mouse)
                    {
                        apr::enable_relative_mouse(application);
                        relative_mouse = true;
                    }
                    else
                    {
                        apr::disable_relative_mouse(application);
                        relative_mouse = false;
                    }
                }
            }
            else if(mouseevent.type == apr::mouse_event::moved && relative_mouse)
            {
                std::cout << "Mouse move of a distance of " << mouseevent.relative_x << "; " << mouseevent.relative_y << std::endl;
            }
        }
        else if(std::holds_alternative<apr::text_event>(event))
        {
            auto&& textevent{std::get<apr::text_event>(event)};

            text += textevent.text;

            std::cout << ">" << text << std::endl;
        }
    }

    apr::end_text_input(application);
}
