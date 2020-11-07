#include <array>

#include <captal/engine.hpp>

#include <captal/components/camera.hpp>
#include <captal/components/drawable.hpp>
#include <captal/components/node.hpp>

#include <captal/systems/render.hpp>
#include <captal/systems/sorting.hpp>
#include <captal/systems/frame.hpp>

#include <apyre/messagebox.hpp>

#include "sansation.hpp"

static void setup(entt::registry& world)
{
    cpt::text_drawer drawer{cpt::font{sansation_regular_font_data, 16}};

    const auto text{world.create()};
    world.emplace<cpt::components::node>(text);
    world.emplace<cpt::components::drawable>(text, drawer.draw("Hello world!", cpt::text_style::bold, cpt::colors::black));
}

static void run()
{
    auto window{cpt::make_render_window("Captal test", cpt::video_mode{1280, 800}, apr::window_options::resizable)};
    window->set_clear_color(cpt::colors::white);

    entt::registry world{};

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
            .channel_count = 2,
            .frequency = 44100
        };

        const cpt::graphics_parameters graphics
        {
            .options = tph::renderer_options::tiny_memory_heaps,
        };

        cpt::engine engine{"captal_test", cpt::version{0, 1, 0}, system, audio, graphics};

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
