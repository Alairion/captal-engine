#include <iostream>

#include <apyre/application.hpp>
#include <apyre/window.hpp>
#include <apyre/messagebox.hpp>
#include <apyre/inputs.hpp>

int main()
{
    //Initialize Apyre.
    apr::application application{apr::application_extension::extended_client_area};

    //Create a window.
    //The parameters are pretty simple, the app, the window's title, it's size (width then height), and some additionnal options.
    apr::window window{application, "Test window", 640, 480, apr::window_options::extended_client_area};

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

    apr::begin_text_input(application);

    //Here is the event loop.
    //The event iterator will wait indefinitly for window's event, due to apr::event_mode::wait.
    //There is also a polling mode (apr::event_mode::poll) that only runs the loop for available events, then leave it immediately.
    for(auto&& event : apr::event_iterator{application, window, apr::event_mode::wait})
    {
        //apr::event is a std::variant of various apr::xxx_event, so you may use it as you want (visitor pattern, holds_alternative, ...)
        if(std::holds_alternative<apr::window_event>(event))
        {
            //Since the loop will not stop by itself, we must break it at some point.
            if(std::get<apr::window_event>(event).type == apr::window_event::closed)
            {
                break;
            }
        }
        else if(std::holds_alternative<apr::keyboard_event>(event))
        {
            const auto& keyevent{std::get<apr::keyboard_event>(event)};

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
        }
        else if(std::holds_alternative<apr::text_event>(event))
        {
            const auto& textevent{std::get<apr::text_event>(event)};

            text += textevent.text;

            std::cout << ">" << text << std::endl;
        }
    }

    apr::end_text_input(application);
}
