#include <iostream>
#include <fstream>
#include <random>

#include "src/engine.hpp"
#include "src/state.hpp"

#include "src/render_texture.hpp"
#include "src/texture.hpp"
#include "src/sprite.hpp"
#include "src/tilemap.hpp"
#include "src/sound.hpp"
#include "src/view.hpp"
#include "src/text.hpp"

#include "src/components/node.hpp"
#include "src/components/camera.hpp"
#include "src/components/drawable.hpp"
#include "src/components/audio_emiter.hpp"
#include "src/components/listener.hpp"

#include "src/systems/frame.hpp"
#include "src/systems/audio.hpp"
#include "src/systems/render.hpp"

void run()
{
    const cpt::audio_parameters audio{2, 44100};
    const cpt::graphics_parameters graphics{tph::renderer_options::tiny_memory_heaps};
    cpt::engine engine{"captal_text", tph::version{1, 0, 0}, audio, graphics};

    cpt::render_window& window{engine.make_window("Captal test", cpt::video_mode{640, 480, 2, tph::present_mode::fifo, tph::render_target_options::clipping, tph::sample_count::msaa_x1}, apr::window_options::resizable)};
    window.get_target().set_clear_color_value(0.6f, 0.8f, 1.0f);

    std::ifstream ifs{"font.ttf", std::ios_base::binary};
    const std::string data{std::istreambuf_iterator<char>{ifs}, std::istreambuf_iterator<char>{}};

    cpt::font font{data, cpt::load_from_memory, 40};
    cpt::text_drawer drawer{std::move(font), cpt::text_drawer_options::kerning};

    cpt::text_ptr sprite{drawer.draw(u8"Hello world!")};

    cpt::sound_ptr sound{cpt::make_sound("bim_bam_boom.wav", swl::load_from_file, swl::sound_reader_options::none)};
    sound->enable_spatialization();
    sound->set_minimum_distance(200.0f);

    cpt::view_ptr view{cpt::make_view(window)};
    view->fit_to(window);

    entt::registry world{};

    auto camera{world.create()};
    world.assign<cpt::components::node>(camera).set_origin(window.width() / 2.0f, window.height() / 2.0f);
    world.assign<cpt::components::camera>(camera).attach(view);
    world.assign<cpt::components::listener>(camera);

    auto item{world.create()};
    world.assign<cpt::components::node>(item).set_origin(sprite->width() / 2.0f, sprite->height() / 2.0f);
    world.assign<cpt::components::drawable>(item).attach(sprite);
    world.assign<cpt::components::audio_emiter>(item).attach(sound);

    engine.frame_per_second_update_signal().connect([&window](std::uint32_t frame_per_second)
    {
        window.change_title("Captal test - " + std::to_string(frame_per_second) + " FPS");
    });

    window.on_resized().connect([&window, &world, camera, &view](const apr::window_event& event)
    {
        view->fit_to(window);
        world.get<cpt::components::node>(camera).set_origin(event.width / 2.0f, event.height / 2.0f);
    });

    std::array<bool, 4> pressed_keys{};

    window.on_key_pressed().connect([&pressed_keys, sound](const apr::keyboard_event& event)
    {
        if(event.key == apr::keycode::right || event.scan == apr::scancode::d)
            pressed_keys[0] = true;
        if(event.key == apr::keycode::down || event.scan == apr::scancode::s)
            pressed_keys[1] = true;
        if(event.key == apr::keycode::left || event.scan == apr::scancode::a)
            pressed_keys[2] = true;
        if(event.key == apr::keycode::up || event.scan == apr::scancode::w)
            pressed_keys[3] = true;

        if(event.scan == apr::scancode::space)
            sound->start();
    });

    window.on_key_released().connect([&pressed_keys](const apr::keyboard_event& event)
    {
        if(event.key == apr::keycode::right || event.scan == apr::scancode::d)
            pressed_keys[0] = false;
        if(event.key == apr::keycode::down || event.scan == apr::scancode::s)
            pressed_keys[1] = false;
        if(event.key == apr::keycode::left || event.scan == apr::scancode::a)
            pressed_keys[2] = false;
        if(event.key == apr::keycode::up || event.scan == apr::scancode::w)
            pressed_keys[3] = false;
    });

    window.on_mouse_wheel_scroll().connect([&world, camera](const apr::mouse_event& event)
    {
        if(event.wheel > 0)
            world.get<cpt::components::node>(camera).scale(3.0f / 4.0f);
        else
            world.get<cpt::components::node>(camera).scale(4.0f / 3.0f);
    });

    engine.on_update().connect([&world, &pressed_keys, camera](float time)
    {/*
        world.view<cpt::components::node, cpt::components::drawable>().each([time](auto, cpt::components::node& node, const cpt::components::drawable&)
        {
            node.rotate(cpt::pi<float> * time / 2.0f);
        });*/

        cpt::components::node& node{world.get<cpt::components::node>(camera)};

        if(pressed_keys[0])
            node.move(512.0f * time * node.scale(), 0.0f);
        if(pressed_keys[1])
            node.move(0.0f, 512.0f * time * node.scale());
        if(pressed_keys[2])
            node.move(-512.0f * time * node.scale(), 0.0f);
        if(pressed_keys[3])
            node.move(0.0f, -512.0f * time * node.scale());
    });

    while(engine.run())
    {
        if(window.is_rendering_enable())
        {
            cpt::systems::audio(world);
            cpt::systems::render(world);
            cpt::systems::end_frame(world);

            window.present();
        }
    }
}

int main()
{
    try
    {
        run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch(...)
    {
        std::cerr << "Unknown error (exception's type does not inherit from std::exception)" << std::endl;
    }
}
