#include <iostream>
#include <iomanip>

#include <captal/engine.hpp>
#include <captal/texture.hpp>
#include <captal/sound.hpp>
#include <captal/view.hpp>
#include <captal/physics.hpp>
#include <captal/text.hpp>
#include <captal/shapes.hpp>

#include <captal/components/node.hpp>
#include <captal/components/camera.hpp>
#include <captal/components/drawable.hpp>
#include <captal/components/audio_emiter.hpp>
#include <captal/components/listener.hpp>
#include <captal/components/rigid_body.hpp>
#include <captal/components/controller.hpp>

#include <captal/systems/frame.hpp>
#include <captal/systems/audio.hpp>
#include <captal/systems/render.hpp>
#include <captal/systems/physics.hpp>

#include <apyre/messagebox.hpp>

using namespace cpt::enum_operations;

class sawtooth_generator : public swl::sound_reader
{
public:
    sawtooth_generator(std::uint32_t frequency, std::uint32_t channels, std::uint32_t wave_lenght)
    :m_wave_lenght{wave_lenght}
    {
        const swl::sound_info info
        {
            .frame_count = std::numeric_limits<std::uint64_t>::max(),
            .frequency = frequency,
            .channel_count = channels,
        };

        set_info(info);
    }

    ~sawtooth_generator() = default;
    sawtooth_generator(const sawtooth_generator&) = delete;
    sawtooth_generator& operator=(const sawtooth_generator&) = delete;
    sawtooth_generator(sawtooth_generator&& other) noexcept = default;
    sawtooth_generator& operator=(sawtooth_generator&& other) noexcept = default;

    bool read(float* output, std::size_t frame_count) override
    {
        for(std::size_t i{}; i < frame_count; ++i)
        {
            const float value{next_value()};

            for(std::size_t j{}; j < info().channel_count; ++j)
            {
                output[i * info().channel_count + j] = value;
            }
        }

        return true;
    }

private:
    float next_value() noexcept
    {
        //Pretty easy implementation
        m_value += 2.0f / static_cast<float>(m_wave_lenght);

        if(m_value >= 1.0f)
        {
            m_value -= 2.0f;
        }

        return m_value;
    }

private:
    float m_value{-1.0f};
    std::uint32_t m_wave_lenght{};
};

static constexpr auto sansation_regular_font_data = std::to_array<std::uint8_t>(
{
    #include "Sansation_Regular.ttf.txt"
});

static constexpr cpt::collision_type_t player_type{1};
static constexpr cpt::collision_type_t wall_type{2};

static entt::entity add_player(entt::registry& world, cpt::physical_world& physical_world)
{
    //The player entity
    const auto player{world.create()};

    //The player node for its position and rotation
    world.emplace<cpt::components::node>(player, cpt::vec3f{320.0f, 240.0f, 0.5f});

    //The player will emit sounds when a wall is hit
    world.emplace<cpt::components::audio_emiter>(player, std::make_unique<sawtooth_generator>(44100, 2, 250))->set_volume(0.5f);

    //The player sprite, we use an ellipse. Why ? Because why not !
    const auto points{cpt::ellipse(48.0f, 32.0f)};
    cpt::polygon sprite{points, cpt::colors::white};
     //Let's add some colors !
    for(std::uint32_t i{}; i < std::size(points); ++i)
    {
        //It's easier to make a rainbow with HSV
        const float hue{(static_cast<float>(i) / std::size(points)) * 360.0f};
        sprite.set_point_color(i, cpt::hsv_to_rgb(hue, 1.0f, 1.0f));
    }

    world.emplace<cpt::components::drawable>(player, std::move(sprite));

    //The player physical body, we use cpt::polygon_moment to compute the moment of inertia for our shape.
    //Note that we use the points generated by cpt::ellipse, we don't need to recompute anything.
    auto& player_body{world.emplace<cpt::components::rigid_body>(player, physical_world, cpt::physical_body_type::dynamic, 10.0f, cpt::polygon_moment(10.0f, points))};
    player_body->set_position(cpt::vec2f{320.0f, 240.0f});
    player_body.attach_shape(points).set_collision_type(player_type);

    //A controller is a kinetic body linked to a dynamic one by constraints.
    //By using the good constraints we can control the dynamic body behaviour without affecting the simulation.
    auto& controller{world.emplace<cpt::components::controller>(player, physical_world)};
    //Pivot joints synchronize bodies' velocity
    auto& pivot{controller.attach_constraint(cpt::pivot_joint, *player_body, cpt::vec2f{}, cpt::vec2f{})};
    pivot.set_max_bias(0.0f);
    pivot.set_max_force(100000.0f);
    //Gear joints synchronize bodies' rotation
    auto& gear{controller.attach_constraint(cpt::gear_joint, *player_body, 0.0f, 1.0f)};
    gear.set_error_bias(0.0f);
    gear.set_max_bias(1.0f);
    gear.set_max_force(100000.0f);
    //So when we give velocity (move) to our controller the body will have the same velocity, and same for rotation.

    return player;
}

static entt::entity fill_world(entt::registry& world, cpt::physical_world& physical_world)
{
    //A background (to slighlty increase scene's complexity)
    const auto background_entity{world.create()};
    world.emplace<cpt::components::node>(background_entity, cpt::vec3f{0.0f, 0.0f, 0.0f});
    world.emplace<cpt::components::drawable>(background_entity, std::in_place_type<cpt::sprite>, 640, 480, cpt::colors::yellowgreen);

    //Add some squares to the scene
    constexpr std::array positions{cpt::vec2f{200.0f, 140.0f}, cpt::vec2f{540.0f, 140.0f}, cpt::vec2f{200.0f, 340.0f}, cpt::vec2f{540.0f, 340.0f}};
    for(auto&& position : positions)
    {
        cpt::physical_body body{physical_world, cpt::physical_body_type::dynamic, 3.0f, cpt::square_moment(3.0f, 24.0f, 24.0f)};
        body.set_position(position);

        const auto item{world.create()};
        world.emplace<cpt::components::node>(item, cpt::vec3f{position, 0.5f}, cpt::vec3f{12.0f, 12.0f, 0.0f});
        world.emplace<cpt::components::drawable>(item, std::in_place_type<cpt::sprite>, 24, 24, cpt::colors::blue);
        world.emplace<cpt::components::rigid_body>(item, std::move(body)).attach_shape(24.0f, 24.0f);
    }

    //Walls are placed at window's limits
    auto walls{world.create()};
    auto& walls_body{world.emplace<cpt::components::rigid_body>(walls, physical_world, cpt::physical_body_type::steady)};
    walls_body.attach_shape(cpt::vec2f{0.0f, 0.0f},   cpt::vec2f{0.0f, 480.0f}  ).set_collision_type(wall_type);
    walls_body.attach_shape(cpt::vec2f{0.0f, 0.0f},   cpt::vec2f{640.0f, 0.0f}  ).set_collision_type(wall_type);
    walls_body.attach_shape(cpt::vec2f{640.0f, 0.0f}, cpt::vec2f{640.0f, 480.0f}).set_collision_type(wall_type);
    walls_body.attach_shape(cpt::vec2f{0.0f, 480.0f}, cpt::vec2f{640.0f, 480.0f}).set_collision_type(wall_type);

    return add_player(world, physical_world);
}

static void add_logic(const cpt::render_window_ptr& window, entt::registry& world, cpt::physical_world& physical_world, entt::entity camera, const std::shared_ptr<cpt::frame_time_t>& time)
{
    cpt::text_drawer drawer{cpt::font{sansation_regular_font_data, 16}/*, cpt::text_drawer_options::none, tph::sampling_options{tph::filter::linear, tph::filter::linear}*/};
    drawer.set_name("sansation");

    const auto text{world.create()};
    world.emplace<cpt::components::node>(text, cpt::vec3f{4.0f, 4.0f, 1.0f});
    world.emplace<cpt::components::drawable>(text, drawer.draw("Text", cpt::text_style::regular, cpt::colors::black));

    for(std::uint32_t i{20}; i > 12; i -= 1)
    {
        drawer.resize(i);
        const auto style{i % 2 == 1 ? cpt::text_style::bold : cpt::text_style::regular};
        drawer.draw("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890&é\"'(-è_çà)=~#{[|`\\^@]}^$*ù!:;,?./§µ%£¨ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãåæçèéêëìíîïñòóôö÷øùúûüþÿĀāĂăĄąĆćĈĉĊċČčĎďĐēĔĖėĘęĚěĜĝĞğĠġĢģĤĥĩĪīĬĭĮįİıĴĵĶķĸĹĻļĽľĿŀŁłŃńŅņŇňŉŊŋ", style);
    }

    drawer.resize(16);

    drawer.upload();

    //Display current FPS in window title, and GPU memory usage (only memory allocated using Tephra's renderer's allocator)
    cpt::engine::instance().frame_per_second_update_signal().connect([&world, text, drawer = std::move(drawer), time](std::uint32_t frame_per_second) mutable
    {
        const auto format_data = [](std::size_t amount)
        {
            std::stringstream ss{};
            ss << std::setprecision(2);

            if(amount < 1024)
            {
                ss << amount << " o";
            }
            else if(amount < 1024 * 1024)
            {
                ss << std::fixed << static_cast<double>(amount) / 1024.0 << " kio";
            }
            else
            {
                ss << std::fixed << static_cast<double>(amount) / (1024.0 * 1024.0) << " Mio";
            }

            return ss.str();
        };

        const auto memory_heaps{cpt::engine::instance().renderer().allocator().heap_count()};
        const auto memory_used{cpt::engine::instance().renderer().allocator().used_memory()};
        const auto memory_alloc{cpt::engine::instance().renderer().allocator().allocated_memory()};

        std::string info{};
        info += "Device local (" + std::to_string(memory_heaps.device_local) + "): " + format_data(memory_used.device_local) + " / " + format_data(memory_alloc.device_local) + "\n";
        info += "Device shared (" + std::to_string(memory_heaps.device_shared) + "): " + format_data(memory_used.device_shared) + " / " + format_data(memory_alloc.device_shared) + "\n";
        info += "Host shared (" + std::to_string(memory_heaps.host_shared) + "): " + format_data(memory_used.host_shared) + " / " + format_data(memory_alloc.host_shared) + "\n";
        info += std::to_string(frame_per_second) + " FPS\n";

        auto fps{std::to_string(std::chrono::duration<double, std::milli>{*time}.count())};
        fps.resize(4);
        info += "Frame time: " + fps + "ms";

        cpt::engine::instance().renderer().allocator().clean_dedicated();

        world.get<cpt::components::drawable>(text).attach(drawer.draw(info, cpt::text_style::regular, cpt::colors::black));
        world.get<cpt::components::node>(text).update();

        drawer.upload();
    });

    //Add a zoom support
    window->on_mouse_wheel_scroll().connect([&world, camera](const apr::mouse_event& event)
    {
        if(event.wheel > 0)
        {
            world.get<cpt::components::node>(camera).scale(cpt::vec3f{1.0f / 2.0f, 1.0f / 2.0f, 1.0f});
        }
        else
        {
            world.get<cpt::components::node>(camera).scale(cpt::vec3f{2.0f / 1.0f, 2.0f / 1.0f, 1.0f});
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

    //Add all physics to the world.
    auto player{fill_world(world, physical_world)};

    //Add some physics based behaviour.
    //The player can collide with multiple walls at the same time, so we don't use a boolean but an integer.
    auto current_collisions{std::make_shared<std::uint32_t>()};

    //Contains all callbacks for collision handling.
    cpt::physical_world::collision_handler collision_handler{};

    //We don't use the parameters.
    collision_handler.collision_begin = [&world, player, current_collisions](auto&, auto&, auto&, auto, auto*)
    {
        *current_collisions += 1;

        //Start the sawtooth when we first collide.
        if(*current_collisions == 1)
        {
            world.get<cpt::components::audio_emiter>(player)->start();
        }

        return true;
    };

    collision_handler.collision_end = [&world, player, current_collisions](auto&, auto&, auto&, auto, auto*)
    {
        *current_collisions -= 1;

        //Stop the sawtooth when we no longer collide with any wall.
        if(*current_collisions == 0)
        {
            world.get<cpt::components::audio_emiter>(player)->stop();
        }

        return true;
    };

    //When the player and a wall collide, our callbacks will be called.
    physical_world.add_collision(player_type, wall_type, std::move(collision_handler));

    //This signal will be called withing cpt::engine::instanc()::run()
    //We could have write this code inside the main loop instead.
    cpt::engine::instance().on_update().connect([pressed_keys, &physical_world, &world, player](float time)
    {
        cpt::vec2f new_velocity{};

        if(pressed_keys[0]) new_velocity += cpt::vec2f{256.0f, 0.0f};
        if(pressed_keys[1]) new_velocity += cpt::vec2f{0.0f, 256.0f};
        if(pressed_keys[2]) new_velocity += cpt::vec2f{-256.0f, 0.0f};
        if(pressed_keys[3]) new_velocity += cpt::vec2f{0.0f, -256.0f};

        //Update player controller based on user inputs.
        world.get<cpt::components::controller>(player)->set_velocity(new_velocity);
        //Update the physical world with the elapsed time.
        physical_world.update(time);
    });
}

static void run()
{
    //-The first value is the window width (640 pixels). (no default value)
    //-The second value is the window height (480 pixels). (no default value)
    //-The third value is the number of image in the swapchain of the window's surface. (default: 2)
    //    2 means double buffering, 3 triple buffering and so on.
    //    This value is limited in a specific interval by the implementation, but 2 is one of the commonest value and should work everywhere.
    //-The present mode defines window behaviour on presentation. (default: tph::present_mode::fifo)
    //    FIFO is the only one available on any hardware (cf. Vulkan Specification), and correspond to V-Sync.
    //-The sample count enable MSAA (MultiSample Anti-Aliasing). (default: tph::sample_count::msaa_x1)
    //    MSAA will smoother the edges of polygons rendered in the window.
    //    MSAAx4 and no MSAA (MSAAx1), are always available (cf. Vulkan Specification)
    //-The texture format is given to enable depth buffering.
    //    Depth format "d32_sfloat" is widely available, so it is hardcoded. But in real appliction should check for it's availability.
    constexpr cpt::video_mode video_mode{640, 480, 2, tph::present_mode::mailbox, tph::sample_count::msaa_x4, tph::texture_format::d32_sfloat};

    //Create the window
    cpt::render_window_ptr window{cpt::make_render_window("Captal test", video_mode, apr::window_options::resizable)};
    //Clear color is a part of tph::render_target, returned by cpt::render_target::get_target()
    window->set_clear_color(cpt::colors::white);

    //Our physical world. See add_logic() function for more informations.
    //You must destroy the physical world AFTER all objects which refer to it so you should construct it before your entt::registry
    cpt::physical_world physical_world{};
    //Physical world's objects' velocity will be multiplied by the damping each second.
    //So more the damping is low, less physical world's objects will preserve their velocity.
    physical_world.set_damping(0.1f);
    //Sleep threshold will put idle objects asleep if they didn't move for more than the specified time. This will increase perfomances.
    physical_world.set_sleep_threshold(0.5f);

    //Our world. Captal does not provide ECS (Entity Components Systems), but is designed to work with Entt.
    //Check out how Entt works on its Github repo: https://github.com/skypjack/entt
    entt::registry world{};

    //Since we use multisampling, we must use a compatible pipeline.
    //A render technique describes how a view will render the scene it seens.
    //Here we need to turn on multisampling within the technique's pipeline...
    cpt::render_technique_info technique_info{};
    technique_info.multisample.sample_count = tph::sample_count::msaa_x4;
    technique_info.multisample.sample_shading = 1.0f;
    //And also depth buffering.
    technique_info.depth_stencil.depth_test = true;
    technique_info.depth_stencil.depth_write = true;
    technique_info.depth_stencil.depth_compare_op = tph::compare_op::greater_or_equal;

    //Our camera, it will hold the cpt::view for our scene.
    const auto camera{world.create()};
    world.emplace<cpt::components::node>(camera, cpt::vec3f{320.0f, 240.0f, 1.0f}, cpt::vec3f{320.0f, 240.0f, 0.0f});
    world.emplace<cpt::components::camera>(camera, window, technique_info)->fit_to(window);

    //See above.
    auto time_ptr{std::make_shared<cpt::frame_time_t>()};
    add_logic(window, world, physical_world, camera, time_ptr);

    //The game engine will return true if there is at least one window opened.
    //Run function will update all windows created within the engine and process their events.
    //It also trigger on_update().
    //It also keeps track of the elapsed time within two call of run().
    //This function is usually be used as the main loop of your game.
    while(cpt::engine::instance().run())
    {
        //Process window events
        window->update();

        //Physics system will update objects' nodes based on value given by the physical world.
        //Will call this system first so other systems will have the newest positions data.
        cpt::systems::physics(world);

        //Audio systems will update objects' and listener' position withing the audio world.
        cpt::systems::audio(world);

        //We must not present a window that has rendering disabled.
        //Window rendering may be disabled when it is closed or minimized.
        if(window->is_rendering_enable())
        {
            //We register the frame time
            auto& signal{window->register_frame_time()};
            signal.connect([time_ptr](cpt::frame_time_t time)
            {
                *time_ptr = time;
            });

            //Render system will update all views within the world,
            //and draw all drawable items to all render targets associated to the views.
            cpt::systems::render(world);

            //Before executing work on the GPU, we first need to execute memory transfers that occured during the frame
            cpt::engine::instance().submit_transfers();

            //This will update window's swapchain.
            //Doing so will put the newly drawn image in the rendering queue of the system's presentation engine.
            //It will then display it on screen. It's accual behaviour will depends on window presention mode.
            window->present();
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{1});
        }

        //End frame system marks the end of the current frame.
        //It will reset some states within the world to prepare it for a new frame.
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
