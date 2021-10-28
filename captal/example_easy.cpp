#include <captal/engine.hpp>
#include <captal/view.hpp>
#include <captal/renderable.hpp>

int main()
{
    //Initialize the engine
    cpt::engine engine{"captal_test", cpt::version{0, 1, 0}};

    //Create the window
    cpt::window_ptr window{cpt::make_window("Example", 640, 480)};

    //Create the render window that targets our window. 
    //The default video mode is basically double buffering + VSync.
    cpt::render_window_ptr target{cpt::make_render_window(window, cpt::video_mode{})};

    //Create a view on our render window.
    //It will use the default engine render layout, 
    //and it will create the default render technique that uses the default engine's shaders
    //It is basically all defaults :)
    cpt::view view{target};

    //"fit" is an helper functions that set the view size, viewport and scissor 
    //to the dimension of the window or texture.
    view.fit(window);

    //A 40x40px sprite, could have been any renderable
    cpt::sprite sprite{40, 40, cpt::colors::dodgerblue};
    //Center the sprite
    sprite.move_to(cpt::vec3f{300.0f, 220.0f, 0.0f});

    //We need to ensure that our object are up to date on the GPU
    cpt::memory_transfer_info transfer_info{cpt::engine::instance().begin_transfer()};

    //cpt::view and cpt::basic_renderable have an helper function "upload" 
    //that record everything that is needed for you.
    view.upload(transfer_info);
    sprite.upload(transfer_info);

    //Perform the transfer for real.
    cpt::engine::instance().submit_transfers();

    //We need to close the window manually, the close event just tell us that 
    //the user want to close it, not that the window has been actually closed.
    window->on_close().connect([](cpt::window& window, const apr::window_event&)
    {
        window.close();
    });

    //The main loop, once the window will be closed a quit event will be generated
    //so the engine will return false
    while(cpt::engine::instance().run())
    {
        //Tell the window to poll all events and send them through signals
        window->dispatch_events();

        //Here we do our rendering, since we never change anything in our scene,
        //we never use `cpt::begin_render_options::reset`
        auto render_info{target->begin_render(cpt::begin_render_options::none)};
        if(render_info)
        {
            //Bind the view, only one view can be bound at a time for each render target.
            view.bind(*render_info);

            //Bind and draw the sprite, the view must be specified again because 
            //the renderable needs
            sprite.draw(*render_info, view);
        }

        //Actually send the work to the GPU
        //Since it is a window, it also actually present the next swapchain image to the screen.
        target->present();
    }
}
