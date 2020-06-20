#include <iostream>

#include <captal_foundation/encoding.hpp>

#include "src/application.hpp"
#include "src/window.hpp"
#include "src/messagebox.hpp"

int main()
{
    //Initialize Apyre.
    apr::application application{};

    //Create a window.
    //The parameters are pretty simple, the app, the window's title, it's size (width then height), and some additionnal options.
    apr::window window{application, u8"Test window", 640, 480, apr::window_options::resizable};

    //Let's play with monitors, we can grab on which monitor the windows actually is.
    const apr::monitor& monitor{window.current_monitor()};

    //Display some informations.
    std::cout << "Window is on monitor: " << cpt::convert<cpt::utf8, cpt::narrow>(monitor.name()) << '.' << std::endl;
    std::cout << "It is located at (" << monitor.x() << "; " << monitor.y() << ") on the virtual screen." << std::endl;
    std::cout << "It has a size of " << monitor.width() << "x" << monitor.height() << "px." << std::endl;

    if(monitor.is_main_monitor())
    {
        std::cout << "It's the main monitor." << std::endl;
    }
    else
    {
        std::cout << "It's not the main monitor." << std::endl;
    }

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
            if(keyevent.type == apr::keyboard_event::key_pressed && keyevent.scan == apr::scancode::space)
            {
                //Message box buttons, they all have an ID, a text, and optionally a default binding on escape or enter.
                const std::array buttons{apr::message_box_button{0, u8"It was good!"}, apr::message_box_button{1, u8"It wasn't great."}};

                //This function is blocking.
                //It will return the button clicked by the user (based on buttons id), or apr::no_selection if they closed the message box.
                const auto id{apr::message_box(window, apr::message_box_type::warning, u8"Hello!", u8"Did you have a good day?", buttons)};

                if(id == 0)
                {
                    apr::message_box(window, apr::message_box_type::information, u8"Cool!", u8"I hope you like Apyre!");
                }
                else if(id == 1)
                {
                    apr::message_box(window, apr::message_box_type::information, u8"Oh.", u8"Don't worry, tomorrow will be a better day! <3");
                }
                else //id == apr::no_selection
                {
                    apr::message_box(window, apr::message_box_type::information, u8"Hum...", u8"Kinda shy aren't you?");
                }
            }
        }
    }
}
