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

constexpr std::string_view lorem_ipsum{"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
                                       "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
                                       "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
                                       "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."};

static void setup(entt::registry& world)
{
    //columns
    const auto background_left{world.create()};
    world.emplace<cpt::components::node>(background_left);
    world.emplace<cpt::components::drawable>(background_left, std::in_place_type<cpt::sprite>, 320, 800, cpt::colors::lightgray);

    const auto background_right{world.create()};
    world.emplace<cpt::components::node>(background_right, cpt::vec3f{640.0f, 0.0f, 0.0f});
    world.emplace<cpt::components::drawable>(background_right, std::in_place_type<cpt::sprite>, 320, 800, cpt::colors::lightgray);

    cpt::text_drawer drawer{cpt::font{sansation_regular_font_data, 20}};

    //Left aligned
    const auto text_left{world.create()};
    world.emplace<cpt::components::node>(text_left, cpt::vec3f{0.0f, 0.0f, 1.0f});
    world.emplace<cpt::components::drawable>(text_left, drawer.draw(lorem_ipsum, 320, cpt::text_align::left, cpt::text_style::regular, cpt::colors::black));

    const auto computed_bounds_value{drawer.bounds(lorem_ipsum, 320, cpt::text_align::left, cpt::text_style::regular)};
    const auto computed_bounds{world.create()};
    world.emplace<cpt::components::node>(computed_bounds, cpt::vec3f{0.0f, 0.0f, 0.5f});
    world.emplace<cpt::components::drawable>(computed_bounds, std::in_place_type<cpt::sprite>, computed_bounds_value.width, computed_bounds_value.height, cpt::colors::orange);

    //right aligned
    /*const auto text2{world.create()};
    world.emplace<cpt::components::node>(text2, cpt::vec3f{320.0f, 0.0f, 1.0f});
    world.emplace<cpt::components::drawable>(text2, drawer.draw("Hello world!", cpt::text_style::regular, cpt::colors::black));
*/
    drawer.upload();
}

static void run()
{
    auto window{cpt::make_render_window("Captal test", cpt::video_mode{1280, 800}, apr::window_options::resizable)};
    window->set_clear_color(cpt::colors::white);

    entt::registry world{};

    const auto camera{world.create()};
    world.emplace<cpt::components::node>(camera, cpt::vec3f{0.0f, 0.0f, 1.0f});
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
