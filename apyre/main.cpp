#include <iostream>

#include "src/application.hpp"
#include "src/window.hpp"

int main()
{
    apr::application application{};
    apr::window window{application, "test window", 640, 480, apr::window_options::resizable};

    const apr::monitor& monitor{window.current_monitor()};

    std::cout << monitor.name() << std::endl;
    std::cout << monitor.x() << "; " << monitor.y() << std::endl;
    std::cout << monitor.width() << "x" << monitor.height() << std::endl;
    std::cout << monitor.id() << std::endl;
    std::cout << std::boolalpha << monitor.is_main_monitor() << std::endl;

    for(auto&& event : apr::event_iterator{application, window, apr::event_mode::wait})
    {
        if(std::holds_alternative<apr::window_event>(event))
            if(std::get<apr::window_event>(event).type == apr::window_event::closed)
                break;
    }
}
