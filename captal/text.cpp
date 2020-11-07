#include <array>

#include <captal/engine.hpp>
#include <captal/entity.hpp>

#include <captal/components/camera.hpp>
#include <captal/components/drawable.hpp>
#include <captal/components/node.hpp>

#include <captal/systems/render.hpp>
#include <captal/systems/sorting.hpp>
#include <captal/systems/frame.hpp>

#include <apyre/messagebox.hpp>

#include "sansation.hpp"

static void setup(cpt::registry& world)
{

}

static void run()
{
    auto window{cpt::make_render_window("Captal test", cpt::video_mode{1280, 800}, apr::window_options::resizable)};
    window->set_clear_color(cpt::colors::white);

    cpt::registry world{};

    const auto camera{world.create()};
    world.emplace<cpt::components::node>(camera);
    world.emplace<cpt::components::camera>(camera, window)->fit_to(window);

    setup(world);

    while(cpt::engine::instance().run())
    {
        window->update();

        if(window->is_rendering_enable())
        {
            cpt::systems::z_sorting(world);
            cpt::systems::render(world);

            cpt::engine::instance().submit_transfers();
            window->present();
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{10});
        }

        cpt::systems::end_frame(world);
    }
}

int main()
{
    try
    {
        const cpt::system_parameters system{};

        const cpt::audio_parameters audio
        {
            //The number of channels, 2 is stereo.
            .channel_count = 2,
            //The frequency of the output stream.
            .frequency = 44100
        };

        const cpt::graphics_parameters graphics
        {
            //The renderer options (c.f. tph::renderer_options)
            .options = tph::renderer_options::tiny_memory_heaps,
            //The physical device's features. Real application must check feature availability
            .features = tph::physical_device_features
            {
                //Enable sample shading (i.e. MSAA inside textures)
                .sample_shading = true
            }
        };

        //The engine instance. It must be created before most call to captal functions.
        //The first value is your application name. It will be passed to Tephra's instance then to Vulkan's instance.
        //The second value is your application version. It will be passed to Tephra's instance then to Vulkan's instance.
        //Then give the underlying system, audio and graphics parameters.
        //If you don't need anything special, there is a constructor without these additionnal parameters
        cpt::engine engine{"captal_test", cpt::version{0, 1, 0}, system, audio, graphics};

        //The engine is reachable by its static address. We don't need to keep a reference on it.
        run();
    }
    catch(const std::exception& e)
    {
        apr::message_box(apr::message_box_type::error, "Error", "An exception as been throw:\n" + std::string{e.what()});
    }
    catch(...)
    {
        apr::message_box(apr::message_box_type::error, "Unknown error", "Exception's type does not inherit from std::exception");
    }
}
