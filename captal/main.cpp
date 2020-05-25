#include <iostream>

#include "src/engine.hpp"
#include "src/texture.hpp"
#include "src/sprite.hpp"
#include "src/sound.hpp"
#include "src/view.hpp"
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
#include "src/systems/sorting.hpp"

class sawtooth_generator : public swl::sound_reader
{
public:
    sawtooth_generator(std::uint32_t frequency, std::uint32_t channels, std::uint32_t wave_frequency, std::uint32_t max)
    :m_frequency{frequency}
    ,m_channels{channels}
    ,m_wave_frequency{wave_frequency}
    ,m_max{max}
    {

    }

    ~sawtooth_generator() = default;
    sawtooth_generator(const sawtooth_generator&) = delete;
    sawtooth_generator& operator=(const sawtooth_generator&) = delete;
    sawtooth_generator(sawtooth_generator&& other) noexcept = default;
    sawtooth_generator& operator=(sawtooth_generator&& other) noexcept = default;

protected:
    bool read_samples(float* output, std::size_t frame_count) override
    {
        for(std::size_t i{}; i < frame_count; ++i)
        {
            const float value{next_value()};

            for(std::size_t j{}; j < m_channels; ++j)
            {
                output[i * m_channels + j] = value;
            }
        }

        return true;
    }

    void seek_samples(std::uint64_t frame_offset) override
    {
        m_current_index = frame_offset;
    }

    std::uint64_t get_frame_count() override
    {
        return std::numeric_limits<std::uint64_t>::max();
    }

    std::uint32_t get_frequency() override
    {
        return m_frequency;
    }

    std::uint32_t get_channels() override
    {
        return m_channels;
    }

private:
    float next_value() noexcept
    {
        float value{};
        for(std::uint32_t k{}; k < m_max; ++k)
        {
            const float kf{static_cast<float>(k)};
            const float ff{static_cast<float>(m_wave_frequency)};
            const float tf{static_cast<float>(m_current_index) / static_cast<float>(m_frequency)};

            value += std::pow(-1.0f, kf + 1.0f) * (std::sin(2 * cpt::pi<float> * kf * ff * tf) / kf);
        }

        ++m_current_index;

        return (2.0f * cpt::pi<float>) * value;
    }

private:
    std::uint64_t m_current_index{};
    std::uint32_t m_frequency{};
    std::uint32_t m_channels{};
    std::uint32_t m_wave_frequency{};
    std::uint32_t m_max{};
};

//Needed information to control player's physical body
struct physical_body_controller
{
    cpt::physical_world_ptr physical_world{};
    cpt::physical_body_ptr player_controller{};
    cpt::physical_constraint_ptr player_pivot_joint{};
    cpt::physical_constraint_ptr player_gear_joint{};
    entt::entity player_entity{};
};

static constexpr cpt::collision_type_t player_type{1};
static constexpr cpt::collision_type_t wall_type{2};

static physical_body_controller add_physics(entt::registry& world, const cpt::physical_world_ptr& physical_world)
{
    //A background (to slighlty increase scene's complexity)
    const auto background_entity{world.create()};
    world.assign<cpt::components::node>(background_entity);
    world.assign<cpt::components::drawable>(background_entity, cpt::make_sprite(640, 480, cpt::colors::yellowgreen));

    //Add some squares to the scene
    constexpr std::array<glm::vec2, 4> positions{glm::vec2{200.0f, 140.0f}, glm::vec2{540.0f, 140.0f}, glm::vec2{200.0f, 340.0f}, glm::vec2{540.0f, 340.0f}};
    for(auto&& position : positions)
    {
        cpt::physical_body_ptr sprite_body{cpt::make_physical_body(physical_world, cpt::physical_body_type::dynamic)};
        sprite_body->set_position(position);

        const auto item{world.create()};
        world.assign<cpt::components::node>(item, glm::vec3{position, 1.0f}, glm::vec3{12.0f, 12.0f, 0.0f});
        world.assign<cpt::components::drawable>(item, cpt::make_sprite(24, 24, cpt::colors::blue));
        world.assign<cpt::components::physical_body>(item, std::move(sprite_body)).add_shape(24.0f, 24.0f)->set_collision_type(player_type);
    }

    //Walls are placed at window's limits
    auto walls{world.create()};
    auto& walls_body{world.assign<cpt::components::physical_body>(walls)};
    walls_body.attach(cpt::make_physical_body(physical_world, cpt::physical_body_type::steady));
    walls_body.add_shape(glm::vec2{0.0f, 0.0f},   glm::vec2{0.0f, 480.0f}  )->set_collision_type(wall_type);
    walls_body.add_shape(glm::vec2{0.0f, 0.0f},   glm::vec2{640.0f, 0.0f}  )->set_collision_type(wall_type);
    walls_body.add_shape(glm::vec2{640.0f, 0.0f}, glm::vec2{640.0f, 480.0f})->set_collision_type(wall_type);
    walls_body.add_shape(glm::vec2{0.0f, 480.0f}, glm::vec2{640.0f, 480.0f})->set_collision_type(wall_type);

    //The player
    cpt::physical_body_ptr player_physical_body{cpt::make_physical_body(physical_world, cpt::physical_body_type::dynamic)};
    player_physical_body->set_position(glm::vec2{320.0f, 240.0f});

    const auto player{world.create()};
    world.assign<cpt::components::node>(player, glm::vec3{0.0f, 0.0f, 1.0f}, glm::vec3{16.0f, 16.0f, 0.0f});
    world.assign<cpt::components::drawable>(player, cpt::make_sprite(32, 32, cpt::colors::black));
    world.assign<cpt::components::physical_body>(player, player_physical_body).add_shape(32.0f, 32.0f);
    world.assign<cpt::components::listener>(player);
    world.assign<cpt::components::audio_emiter>(player, cpt::make_sound(swl::sound_file_reader{std::make_unique<sawtooth_generator>(44100, 2, 1000, 100)}))->enable_spatialization();

    //The player is controlled by a kinetic body jointed to the dynamic one
    const auto player_controller{cpt::make_physical_body(physical_world, cpt::physical_body_type::kinematic)};
    player_controller->set_position(glm::vec2{320.0f, 240.0f});

    const auto player_joint{cpt::make_physical_constraint(cpt::pivot_joint, player_controller, player_physical_body, glm::vec2{}, glm::vec2{})};
    player_joint->set_max_bias(0.0f);
    player_joint->set_max_force(10000.0f);

    const auto player_pivot{cpt::make_physical_constraint(cpt::gear_joint, player_controller, player_physical_body, 0.0f, 1.0f)};
    player_pivot->set_error_bias(0.0f);
    player_pivot->set_max_bias(1.0f);
    player_pivot->set_max_force(10000.0f);

    //This set of variables is required to control the player entity
    return physical_body_controller{physical_world, player_controller, player_joint, player_pivot, player};
}

static void add_logic(const cpt::render_window_ptr& window, entt::registry& world, const cpt::physical_world_ptr& physical_world, entt::entity camera)
{
    //Add all physics to the world.
    auto item_controller{add_physics(world, physical_world)};

    //Display current FPS in window title
    cpt::engine::instance().frame_per_second_update_signal().connect([&window](std::uint32_t frame_per_second)
    {
        window->change_title("Captal test - " + std::to_string(frame_per_second) + " FPS");
    });

    //Add a zoom support
    window->on_mouse_wheel_scroll().connect([&world, camera](const apr::mouse_event& event)
    {
        if(event.wheel > 0)
        {
            world.get<cpt::components::node>(camera).scale(1.0f / 3.0f);
        }
        else
        {
            world.get<cpt::components::node>(camera).scale(3.0f / 1.0f);
        }
    });

    //These booleans will holds WASD keys states for player's movements
    //We store this information as a booleans array because we want smooth movements.
    std::shared_ptr<bool[]> pressed_keys{new bool[4]{}};

    //Check what keys have been pressed.
    window->on_key_pressed().connect([pressed_keys](const apr::keyboard_event& event)
    {
        if(event.scan == apr::scancode::d) pressed_keys[0] = true;
        if(event.scan == apr::scancode::s) pressed_keys[1] = true;
        if(event.scan == apr::scancode::a) pressed_keys[2] = true;
        if(event.scan == apr::scancode::w) pressed_keys[3] = true;
    });

    //Check what keys have been released.
    window->on_key_released().connect([pressed_keys](const apr::keyboard_event& event)
    {
        if(event.scan == apr::scancode::d) pressed_keys[0] = false;
        if(event.scan == apr::scancode::s) pressed_keys[1] = false;
        if(event.scan == apr::scancode::a) pressed_keys[2] = false;
        if(event.scan == apr::scancode::w) pressed_keys[3] = false;
    });

    const auto player{item_controller.player_entity};
    auto current_collisions{std::make_shared<std::uint32_t>()};

    cpt::physical_world::collision_handler collision_handler{};

    collision_handler.collision_begin = [&world, player, current_collisions](auto&, auto&, auto&, auto, auto*)
    {
        *current_collisions += 1;

        if(*current_collisions == 1)
        {
            world.get<cpt::components::audio_emiter>(player)->start();
        }

        return true;
    };

    collision_handler.collision_end = [&world, player, current_collisions](auto&, auto&, auto&, auto, auto*)
    {
        *current_collisions -= 1;

        if(*current_collisions == 0)
        {
            world.get<cpt::components::audio_emiter>(player)->stop();
        }

        return true;
    };

    physical_world->add_collision(player_type, wall_type, std::move(collision_handler));

    //This signal will be called withing cpt::engine::instanc()::run()
    //We could have write this code inside the main loop instead.
    cpt::engine::instance().on_update().connect([pressed_keys, item_controller](float time)
    {
        glm::vec2 new_velocity{};

        if(pressed_keys[0]) new_velocity += glm::vec2{96.0f, 0.0f};
        if(pressed_keys[1]) new_velocity += glm::vec2{0.0f, 96.0f};
        if(pressed_keys[2]) new_velocity += glm::vec2{-96.0f, 0.0f};
        if(pressed_keys[3]) new_velocity += glm::vec2{0.0f, -96.0f};

        //Update player controller based on user inputs.
        item_controller.player_controller->set_velocity(new_velocity);
        //Update the physical world with the elapsed time.
        item_controller.physical_world->update(time);
    });
}

static void run()
{
    //-The first value is the window width (640 pixels). (no default value)
    //-The second value is the window height (480 pixels). (no default value)
    //-The third value is the number of image in the swapchain of the window's surface. (default: 2)
    //    2 means double buffering, 3 triple buffering and so on.
    //    This value is limited in a specific interval by the implementation, but 2 is one of the commonest value and should work everywhere.
    //-The fourth value is the present mode. (default: tph::present_mode::fifo)
    //    FIFO is the only one available on any hardware (cf. Vulkan Specification), and correspond to V-Sync.
    //-The fifth value correpond to additionnal render target options. (default: tph::render_target_options::clipping)
    //    Clipping may increase perfomance when parts of the window are not visible (for any reason)
    //    by skipping the fragment shader stage for hidden pixels
    //-The sample count enable MSAA (MultiSample Anti-Aliasing). (default: tph::sample_count::msaa_x1)
    //    MSAA will smoother the edges of polygons rendered in the window.
    //    MSAAx4 and no MSAA (MSAAx1), are always available (cf. Vulkan Specification)
    constexpr cpt::video_mode video_mode{640, 480, 2, tph::present_mode::fifo, tph::render_target_options::clipping, tph::sample_count::msaa_x4};

    //Create the window
    cpt::render_window_ptr window{cpt::engine::instance().make_window("Captal test", video_mode)};
    //Clear color is a part of tph::render_target, returned by cpt::render_target::get_target()
    window->get_target().set_clear_color_value(1.0f, 1.0f, 1.0f);

    //Our world. Captal does not provide ECS (Entity Components Systems), but is designed to work with Entt.
    //Check out how Entt works on its Github repo: https://github.com/skypjack/entt
    entt::registry world{};

    //Our camera, it will hold the cpt::view for our scene.
    const auto camera{world.create()};
    world.assign<cpt::components::node>(camera, glm::vec3{320.0f, 240.0f, 1.0f}, glm::vec3{window->width() / 2.0f, window->height() / 2.0f, 0.0f});
    world.assign<cpt::components::camera>(camera, cpt::make_view(window))->fit_to(window);

    //Our physical world. See add_logic() function for more informations.
    cpt::physical_world_ptr physical_world{cpt::make_physical_world()};
    //Physical world's objects' velocity will be multiplied by the damping each second.
    //So more the damping is low, less physical world's objects will preserve their velocity.
    physical_world->set_damping(0.1f);
    //Sleep threshold will put idle objects asleep if they didn't move for more than the specified time. This will increase perfomances.
    physical_world->set_sleep_threshold(0.5f);

    //See above.
    add_logic(window, world, physical_world, camera);

    //The game engine will return true if there is at least one window opened.
    //Run function will update all windows created within the engine and process their events.
    //It also trigger on_update().
    //It also keeps track of the elapsed time within to call of run().
    //This function is usually be used as the main loop of your game.
    while(cpt::engine::instance().run())
    {
        //Physics system will update objects' nodes based on value given by the physical world.
        //Will call this system first so other systems will have the newest positions data.
        cpt::systems::physics(world);

        //Audio systems will update objects' and listener' position withing the audio world.
        cpt::systems::audio(world);

        //We must not present a window that has rendering disabled.
        //Window rendering may be disabled when it is closed or minimized.
        if(window->is_rendering_enable())
        {
            //Z-Sorting system will sort drawable components by their node's z component.
            cpt::systems::z_sorting(world);

            //Render system will update all views within the world,
            //and draw all drawable items to all render targets associated to the views.
            cpt::systems::render(world);

            //This will update window's swapchain.
            //Doing so will put the newly drawn image in the rendering queue of the system's presentation engine.
            //It will then display it on screen. It's accual behaviour will depends on window presention mode.
            window->present();
        }

        //End frame system marks the end of the current frame.
        //It will reset some states within the world to prepare it for a new frame.
        cpt::systems::end_frame(world);
    }

    //Just some info displayed at the end of the demo.
    const auto memory_used{cpt::engine::instance().renderer().allocator().used_memory()};
    const auto memory_alloc{cpt::engine::instance().renderer().allocator().allocated_memory()};

    std::cout << "Device local : " << memory_used.device_local << " / " << memory_alloc.device_local << "\n";
    std::cout << "Device shared : " << memory_used.device_shared << " / " << memory_alloc.device_shared << "\n";
    std::cout << "Host shared : " << memory_used.host_shared << " / " << memory_alloc.host_shared << "\n";
}


int main()
{
    try
    {
        const cpt::audio_parameters audio{2, 44100};
        const cpt::graphics_parameters graphics{tph::renderer_options::tiny_memory_heaps};
        cpt::engine engine{"captal_test", cpt::version{1, 0, 0}, audio, graphics};

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
