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

constexpr std::string_view lorem_ipsum{"AV Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
                                       "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
                                       "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
                                       "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."};

namespace comp = cpt::components;

static void setup(entt::registry& world)
{
    //columns
    const auto background_left{world.create()};
    world.emplace<comp::node>(background_left);
    world.emplace<comp::drawable>(background_left, std::in_place_type<cpt::sprite>, 320, 800, cpt::colors::lightgray);

    const auto background_center{world.create()};
    world.emplace<comp::node>(background_center, cpt::vec3f{640.0f, 0.0f, 0.0f});
    world.emplace<comp::drawable>(background_center, std::in_place_type<cpt::sprite>, 320, 800, cpt::colors::lightgray);

    cpt::font_set fonts
    {
        .regular     = cpt::font{u8"segoeui.ttf" , 19},
        .italic      = cpt::font{u8"segoeuii.ttf", 19},
        .bold        = cpt::font{u8"segoeuib.ttf", 19},
        .italic_bold = cpt::font{u8"segoeuiz.ttf", 19},
    };

    cpt::text_drawer drawer{std::move(fonts)};
    drawer.set_color(cpt::colors::black);
    drawer.set_style(cpt::text_style::strikethrough | cpt::text_style::underlined);

    //Left alignement
    const auto bounds_left_value{drawer.bounds(lorem_ipsum, 320)};
    const auto bounds_left{world.create()};
    world.emplace<comp::node>(bounds_left, cpt::vec3f{0.0f, 0.0f, 0.5f});
    world.emplace<comp::drawable>(bounds_left, std::in_place_type<cpt::sprite>, bounds_left_value.width, bounds_left_value.height, cpt::colors::orange);

    const auto text_left{world.create()};
    world.emplace<comp::node>(text_left, cpt::vec3f{0.0f, 0.0f, 1.0f});
    world.emplace<comp::drawable>(text_left, drawer.draw(lorem_ipsum, 320));

    //Right alignement
    drawer.set_align(cpt::text_align::right);
    drawer.set_style(cpt::text_style::strikethrough | cpt::text_style::underlined | cpt::text_style::bold);

    const auto bounds_right_value{drawer.bounds(lorem_ipsum, 320)};
    const auto bounds_right{world.create()};
    world.emplace<comp::node>(bounds_right, cpt::vec3f{640.0f - bounds_right_value.width, 0.0f, 0.5f});
    world.emplace<comp::drawable>(bounds_right, std::in_place_type<cpt::sprite>, bounds_right_value.width, bounds_right_value.height, cpt::colors::orange);

    const auto text_right{world.create()};
    world.emplace<comp::drawable>(text_right, drawer.draw(lorem_ipsum, 320));
    const auto right_width{world.get<comp::drawable>(text_right).get<cpt::text>().width()};
    world.emplace<comp::node>(text_right, cpt::vec3f{640.0f - right_width, 0.0f, 1.0f});

    //Center alignement
    drawer.set_align(cpt::text_align::center);
    drawer.set_style(cpt::text_style::strikethrough | cpt::text_style::underlined | cpt::text_style::italic | cpt::text_style::bold);

    const auto bounds_center_value{drawer.bounds(lorem_ipsum, 320)};
    const auto bounds_center{world.create()};
    world.emplace<comp::node>(bounds_center, cpt::vec3f{std::floor(640.0f + (320.0f - bounds_center_value.width) / 2.0f), 0.0f, 0.5f});
    world.emplace<comp::drawable>(bounds_center, std::in_place_type<cpt::sprite>, bounds_center_value.width, bounds_center_value.height, cpt::colors::orange);

    const auto text_center{world.create()};
    world.emplace<comp::drawable>(text_center, drawer.draw(lorem_ipsum, 320));
    const auto center_width{world.get<comp::drawable>(text_center).get<cpt::text>().width()};
    world.emplace<comp::node>(text_center, cpt::vec3f{std::floor(640.0f + (320.0f - center_width) / 2.0f), 0.0f, 1.0f});

    //Justify alignement
    drawer.set_align(cpt::text_align::justify);
    drawer.set_style(cpt::text_style::strikethrough | cpt::text_style::underlined | cpt::text_style::italic);

    const auto bounds_justify_value{drawer.bounds(lorem_ipsum, 320)};
    const auto bounds_justify{world.create()};
    world.emplace<comp::node>(bounds_justify, cpt::vec3f{960.0f, 0.0f, 0.5f});
    world.emplace<comp::drawable>(bounds_justify, std::in_place_type<cpt::sprite>, bounds_justify_value.width, bounds_justify_value.height, cpt::colors::orange);

    const auto text_justify{world.create()};
    world.emplace<comp::drawable>(text_justify, drawer.draw(lorem_ipsum, 320));
    world.emplace<comp::node>(text_justify, cpt::vec3f{960.0f, 0.0f, 1.0f});

    drawer.upload();
}

static void run()
{
    auto window{cpt::make_render_window("Captal test", cpt::video_mode{1280, 800}, apr::window_options::resizable)};
    window->set_clear_color(cpt::colors::white);

    entt::registry world{};

    const auto camera{world.create()};
    world.emplace<comp::node>(camera, cpt::vec3f{0.0f, 0.0f, 1.0f});
    world.emplace<comp::camera>(camera, window)->fit_to(window);

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
