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
#include "src/color.hpp"
#include "src/physics.hpp"

#include "src/components/node.hpp"
#include "src/components/camera.hpp"
#include "src/components/drawable.hpp"
#include "src/components/audio_emiter.hpp"
#include "src/components/listener.hpp"
#include "src/components/physical_body.hpp"

#include "src/systems/frame.hpp"
#include "src/systems/audio.hpp"
#include "src/systems/render.hpp"
#include "src/systems/physics.hpp"

void run()
{
    const cpt::audio_parameters audio{2, 44100};
    const cpt::graphics_parameters graphics{tph::renderer_options::tiny_memory_heaps};
    cpt::engine engine{"captal_text", tph::version{1, 0, 0}, audio, graphics};

    cpt::render_window& window{engine.make_window("Captal test", cpt::video_mode{640, 480, 2, tph::present_mode::fifo, tph::render_target_options::clipping, tph::sample_count::msaa_x4}, apr::window_options::resizable)};
    window.get_target().set_clear_color_value(0.6f, 0.8f, 1.0f);

    cpt::physical_world_ptr physical_world{cpt::make_physical_world()};
    physical_world->set_damping(0.5f);
    physical_world->set_gravity(glm::vec2{0.0f, 512.0f});

    entt::registry world{};

    cpt::view_ptr view{cpt::make_view(window)};
    view->fit_to(window);

    auto camera{world.create()};
    world.assign<cpt::components::node>(camera).set_origin(window.width() / 2.0f, window.height() / 2.0f);
    world.get<cpt::components::node>(camera).move_to(320.0f, 240.0f, 1.0f);
    world.assign<cpt::components::camera>(camera).attach(view);
    world.assign<cpt::components::listener>(camera);

    cpt::sprite_ptr sprite{cpt::make_sprite(100, 100)};

    cpt::sound_ptr sound{cpt::make_sound("bim_bam_boom.wav", swl::load_from_file)};
    sound->enable_spatialization();
    sound->set_minimum_distance(200.0f);
    sound->set_volume(0.5f);

    cpt::physical_body_ptr sprite_body{cpt::make_physical_body(physical_world, cpt::physical_body_type::dynamic)};
    sprite_body->set_position(glm::vec2{320.0f, 240.0f});

    auto item{world.create()};
    world.assign<cpt::components::node>(item).set_origin(sprite->width() / 2.0f, sprite->height() / 2.0f);
    world.assign<cpt::components::drawable>(item).attach(sprite);
    world.assign<cpt::components::audio_emiter>(item).attach(sound);

    auto& item_body{world.assign<cpt::components::physical_body>(item)};
    item_body.attach(sprite_body);
    item_body.add_shape(static_cast<float>(sprite->width()), static_cast<float>(sprite->height()));
    item_body.shape(0)->set_collision_type(0);
    item_body.shape(0)->set_elasticity(0.0f);
    item_body.shape(0)->set_friction(0.0f);

    auto walls{world.create()};
    auto& walls_body{world.assign<cpt::components::physical_body>(walls)};
    walls_body.attach(cpt::make_physical_body(physical_world, cpt::physical_body_type::steady));
    walls_body.add_shape(glm::vec2{0.0f, 0.0f}, glm::vec2{0.0f, 480.0f});
    walls_body.add_shape(glm::vec2{0.0f, 0.0f}, glm::vec2{640.0f, 0.0f});
    walls_body.add_shape(glm::vec2{640.0f, 0.0f}, glm::vec2{640.0f, 480.0f});
    walls_body.add_shape(glm::vec2{0.0f, 480.0f}, glm::vec2{640.0f, 480.0f});

    for(std::size_t i{}; i < 4; ++i)
    {
        walls_body.shape(i)->set_elasticity(0.0f);
        walls_body.shape(i)->set_friction(0.0f);
        walls_body.shape(i)->set_collision_type(1);
    }

    physical_world->add_collision(0, 1, cpt::physical_world::collision_handler
    {
        [sound](cpt::physical_world&, cpt::physical_body&, cpt::physical_body&, cpt::physical_collision_arbiter arbiter, void*)
        {
            if(arbiter.is_first_contact())
                sound->start();

            return true;
        }
    });

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

    engine.on_update().connect([&pressed_keys, &sprite_body, &physical_world](float time)
    {
        glm::vec2 new_velocity{};

        if(pressed_keys[0])
            new_velocity += glm::vec2{512.0f * time, 0.0f};
        if(pressed_keys[1])
            new_velocity += glm::vec2{0.0f, 512.0f * time};
        if(pressed_keys[2])
            new_velocity += glm::vec2{-512.0f * time, 0.0f};
        if(pressed_keys[3])
            new_velocity += glm::vec2{0.0f, -512.0f * time};

        sprite_body->set_velocity(sprite_body->velocity() + new_velocity);
        physical_world->update(time);
    });

    while(engine.run())
    {
        if(window.is_rendering_enable())
        {
            cpt::systems::physics(world);
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
