#include <iostream>

#include "src/engine.hpp"

#include "src/texture.hpp"
#include "src/sprite.hpp"
#include "src/sound.hpp"
#include "src/view.hpp"
#include "src/physics.hpp"
#include "src/render_texture.hpp"

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

struct physical_body_controller
{
    cpt::physical_world_ptr physical_world{};
    cpt::physical_body_ptr item_controller{};
    cpt::physical_constraint_ptr item_pivot_joint{};
    cpt::physical_constraint_ptr item_gear_joint{};
};

static constexpr std::uint32_t normal_map_binding{3};
static constexpr std::uint32_t height_map_binding{4};
static constexpr std::uint32_t specular_map_binding{5};
static constexpr std::uint32_t directional_light_binding{6};
static constexpr std::uint32_t shadow_map_binding{7};

static constexpr std::array<std::uint8_t, 16> dummy_normal_map_data{128, 128, 255, 255, 128, 128, 255, 255, 128, 128, 255, 255, 128, 128, 255, 255};
static constexpr std::array<std::uint8_t, 16> dummy_height_map_data{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static constexpr std::array<std::uint8_t, 16> dummy_specular_map_data{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

physical_body_controller add_physics(entt::registry& world, const cpt::physical_world_ptr& physical_world)
{
    cpt::sprite_ptr background{cpt::make_sprite(640, 480)};
    background->set_color(0xFF078900);
    background->add_uniform_binding(normal_map_binding, cpt::make_texture(2, 2, std::data(dummy_normal_map_data), tph::sampling_options{tph::filter::nearest, tph::filter::nearest, tph::address_mode::repeat}));
    background->add_uniform_binding(height_map_binding, cpt::make_texture(2, 2, std::data(dummy_height_map_data), tph::sampling_options{tph::filter::nearest, tph::filter::nearest, tph::address_mode::repeat}));
    background->add_uniform_binding(specular_map_binding, cpt::make_texture(2, 2, std::data(dummy_specular_map_data), tph::sampling_options{tph::filter::nearest, tph::filter::nearest, tph::address_mode::repeat}));

    auto background_entity{world.create()};
    world.assign<cpt::components::node>(background_entity);
    world.assign<cpt::components::drawable>(background_entity).attach(background);

    cpt::sprite_ptr sprite{cpt::make_sprite(cpt::make_texture("assets/diffuse.png", cpt::load_from_file))};
    sprite->add_uniform_binding(normal_map_binding, cpt::make_texture("assets/normal.png", cpt::load_from_file));
    sprite->add_uniform_binding(height_map_binding, cpt::make_texture("assets/height.png", cpt::load_from_file));
    sprite->add_uniform_binding(specular_map_binding, cpt::make_texture("assets/specular.png", cpt::load_from_file));
    sprite->set_shininess(32.0f);

    cpt::physical_body_ptr sprite_body{cpt::make_physical_body(physical_world, cpt::physical_body_type::dynamic, 1.0f, std::numeric_limits<float>::infinity())};
    sprite_body->set_position(glm::vec2{320.0f, 240.0f});

    auto item{world.create()};
    world.assign<cpt::components::node>(item).set_origin(sprite->width() / 2.0f, sprite->height() / 2.0f + sprite->height() / 4.0f);
    world.get<cpt::components::node>(item).move(0.0f, 0.0f, 0.0f);
    world.assign<cpt::components::drawable>(item).attach(sprite);

    auto& item_body{world.assign<cpt::components::physical_body>(item)};
    item_body.attach(sprite_body);
    item_body.add_shape(static_cast<float>(sprite->width()), static_cast<float>(sprite->height()) / 2.0f, 0.0f);

    constexpr std::array<glm::vec2, 4> positions{glm::vec2{300.0f, 230.0f}, glm::vec2{340.0f, 230.0f}, glm::vec2{300.0f, 250.0f}, glm::vec2{340.0f, 250.0f}};
    for(std::size_t i{}; i < 4; ++i)
    {
        cpt::sprite_ptr sprite{cpt::make_sprite(cpt::make_texture("assets/diffuse.png", cpt::load_from_file))};
        sprite->add_uniform_binding(normal_map_binding, cpt::make_texture("assets/normal.png", cpt::load_from_file));
        sprite->add_uniform_binding(height_map_binding, cpt::make_texture("assets/height.png", cpt::load_from_file));
        sprite->add_uniform_binding(specular_map_binding, cpt::make_texture("assets/specular.png", cpt::load_from_file));
        sprite->set_shininess(32.0f);

        cpt::physical_body_ptr sprite_body{cpt::make_physical_body(physical_world, cpt::physical_body_type::dynamic, 1.0f, std::numeric_limits<float>::infinity())};
        sprite_body->set_position(positions[i]);

        auto item{world.create()};
        world.assign<cpt::components::node>(item).set_origin(sprite->width() / 2.0f, sprite->height() / 2.0f + sprite->height() / 4.0f);
        world.assign<cpt::components::drawable>(item).attach(sprite);

        auto& item_body{world.assign<cpt::components::physical_body>(item)};
        item_body.attach(sprite_body);
        item_body.add_shape(static_cast<float>(sprite->width()), static_cast<float>(sprite->height()) / 2.0f, 0.0f);
        item_body.attachment()->set_velocity(glm::vec2{2.0f, 2.0f});
    }

    auto item_controller{cpt::make_physical_body(physical_world, cpt::physical_body_type::kinematic)};
    item_controller->set_position(glm::vec2{320.0f, 240.0f});
    auto item_joint{cpt::make_physical_constraint(cpt::physical_constraint::pivot_joint, item_controller, sprite_body, glm::vec2{}, glm::vec2{})};
    item_joint->set_max_bias(0.0f);
    item_joint->set_max_force(10000.0f);
    auto item_pivot{cpt::make_physical_constraint(cpt::physical_constraint::gear_joint, item_controller, sprite_body, 0.0f, 1.0f)};
    item_pivot->set_error_bias(0.0f);
    item_pivot->set_max_bias(1.0f);
    item_pivot->set_max_force(10000.0f);

    auto walls{world.create()};
    auto& walls_body{world.assign<cpt::components::physical_body>(walls)};
    walls_body.attach(cpt::make_physical_body(physical_world, cpt::physical_body_type::steady));
    walls_body.add_shape(glm::vec2{0.0f, 0.0f}, glm::vec2{0.0f, 480.0f});
    walls_body.add_shape(glm::vec2{0.0f, 0.0f}, glm::vec2{640.0f, 0.0f});
    walls_body.add_shape(glm::vec2{640.0f, 0.0f}, glm::vec2{640.0f, 480.0f});
    walls_body.add_shape(glm::vec2{0.0f, 480.0f}, glm::vec2{640.0f, 480.0f});

    return physical_body_controller{physical_world, item_controller, item_joint, item_pivot};
}

void add_logic(const cpt::render_window_ptr& window, entt::registry& world, const cpt::physical_world_ptr& physical_world, entt::entity camera)
{
    auto item_controller{add_physics(world, physical_world)};

    cpt::engine::instance().frame_per_second_update_signal().connect([&window](std::uint32_t frame_per_second)
    {
        window->change_title("Captal test - " + std::to_string(frame_per_second) + " FPS");
    });

    window->on_resized().connect([&window, &world, camera](const apr::window_event&)
    {
        world.get<cpt::components::camera>(camera).attachment()->fit_to(window);
        world.get<cpt::components::node>(camera).set_origin(window->width() / 2.0f, window->height() / 2.0f);
    });

    std::shared_ptr<bool[]> pressed_keys{new bool[4]{}};

    window->on_key_pressed().connect([pressed_keys](const apr::keyboard_event& event)
    {
        if(event.scan == apr::scancode::d) pressed_keys[0] = true;
        if(event.scan == apr::scancode::s) pressed_keys[1] = true;
        if(event.scan == apr::scancode::a) pressed_keys[2] = true;
        if(event.scan == apr::scancode::w) pressed_keys[3] = true;
    });

    window->on_key_released().connect([pressed_keys](const apr::keyboard_event& event)
    {
        if(event.scan == apr::scancode::d) pressed_keys[0] = false;
        if(event.scan == apr::scancode::s) pressed_keys[1] = false;
        if(event.scan == apr::scancode::a) pressed_keys[2] = false;
        if(event.scan == apr::scancode::w) pressed_keys[3] = false;
    });

    cpt::engine::instance().on_update().connect([pressed_keys, item_controller](float time)
    {
        glm::vec2 new_velocity{};

        if(pressed_keys[0]) new_velocity += glm::vec2{32.0f, 0.0f};
        if(pressed_keys[1]) new_velocity += glm::vec2{0.0f, 32.0f};
        if(pressed_keys[2]) new_velocity += glm::vec2{-32.0f, 0.0f};
        if(pressed_keys[3]) new_velocity += glm::vec2{0.0f, -32.0f};

        item_controller.item_controller->set_velocity(new_velocity);
        item_controller.physical_world->update(time);
    });
}

struct directional_light
{
    glm::vec4 direction{};
    glm::vec4 ambiant{};
    glm::vec4 diffuse{};
    glm::vec4 specular{};
};

void run()
{
    cpt::framed_buffer_ptr light_buffer{cpt::make_framed_buffer(directional_light{glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}, glm::vec4{0.25f, 0.25f, 0.25f, 1.0f}, glm::vec4{0.9f, 0.9f, 0.9f, 1.0f}, glm::vec4{0.75f, 0.75f, 0.75f, 1.0f}})};
    directional_light& light{light_buffer->get<directional_light>(0)};

    //Height
    tph::shader height_vertex_shader{cpt::engine::instance().renderer(), tph::shader_stage::vertex, "shaders/height.vert.spv", tph::load_from_file};
    tph::shader height_fragment_shader{cpt::engine::instance().renderer(), tph::shader_stage::fragment, "shaders/height.frag.spv", tph::load_from_file};
    cpt::render_technique_info height_info{};
    height_info.stages.push_back(tph::pipeline_shader_stage{height_vertex_shader});
    height_info.stages.push_back(tph::pipeline_shader_stage{height_fragment_shader});
    height_info.stages_bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, height_map_binding, tph::descriptor_type::image_sampler});

    cpt::render_texture_ptr height_map{cpt::make_render_texture(640, 480, tph::sampling_options{})};
    height_map->get_target().set_clear_color_value(0.0f, 0.0f, 0.0f, 1.0f);
    cpt::view_ptr height_map_view{cpt::make_view(height_map, height_info)};
    height_map_view->fit_to(height_map);

    entt::registry world{};

    auto camera{world.create()};
    world.assign<cpt::components::node>(camera).set_origin(height_map->width() / 2.0f, height_map->height() / 2.0f);
    world.get<cpt::components::node>(camera).move_to(height_map->width() / 2.0f, height_map->height() / 2.0f, 1.0f);
    world.assign<cpt::components::camera>(camera).attach(height_map_view);

    //Shadow
    tph::shader shadow_vertex_shader{cpt::engine::instance().renderer(), tph::shader_stage::vertex, "shaders/shadow.vert.spv", tph::load_from_file};
    tph::shader shadow_fragment_shader{cpt::engine::instance().renderer(), tph::shader_stage::fragment, "shaders/shadow.frag.spv", tph::load_from_file};
    cpt::render_technique_info shadow_info{};
    shadow_info.stages.push_back(tph::pipeline_shader_stage{shadow_vertex_shader});
    shadow_info.stages.push_back(tph::pipeline_shader_stage{shadow_fragment_shader});
    shadow_info.stages_bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, height_map_binding, tph::descriptor_type::image_sampler});
    shadow_info.stages_bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, directional_light_binding, tph::descriptor_type::uniform_buffer});

    cpt::render_texture_ptr shadow_map{cpt::make_render_texture(640, 480, tph::sampling_options{})};
    cpt::view_ptr shadow_map_view{cpt::make_view(shadow_map, shadow_info)};
    shadow_map_view->fit_to(shadow_map);
    shadow_map_view->add_uniform_binding(directional_light_binding, light_buffer);

    entt::registry shadow_world{};

    auto shadow_camera{shadow_world.create()};
    shadow_world.assign<cpt::components::node>(shadow_camera).set_origin(shadow_map->width() / 2.0f, shadow_map->height() / 2.0f);
    shadow_world.get<cpt::components::node>(shadow_camera).move_to(shadow_map->width() / 2.0f, shadow_map->height() / 2.0f, 1.0f);
    shadow_world.assign<cpt::components::camera>(shadow_camera).attach(shadow_map_view);

    auto shadow_sprite{shadow_world.create()};
    shadow_world.assign<cpt::components::node>(shadow_sprite);
    shadow_world.assign<cpt::components::drawable>(shadow_sprite).attach(cpt::make_sprite(640, 480));
    shadow_world.get<cpt::components::drawable>(shadow_sprite).attachment()->add_uniform_binding(height_map_binding, height_map);

    //Diffuse
    tph::shader diffuse_vertex_shader{cpt::engine::instance().renderer(), tph::shader_stage::vertex, "shaders/lighting.vert.spv", tph::load_from_file};
    tph::shader diffuse_fragment_shader{cpt::engine::instance().renderer(), tph::shader_stage::fragment, "shaders/lighting.frag.spv", tph::load_from_file};
    cpt::render_technique_info diffuse_info{};
    diffuse_info.stages.push_back(tph::pipeline_shader_stage{diffuse_vertex_shader});
    diffuse_info.stages.push_back(tph::pipeline_shader_stage{diffuse_fragment_shader});
    diffuse_info.stages_bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, normal_map_binding, tph::descriptor_type::image_sampler});
    diffuse_info.stages_bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, specular_map_binding, tph::descriptor_type::image_sampler});
    diffuse_info.stages_bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, directional_light_binding, tph::descriptor_type::uniform_buffer});
    diffuse_info.stages_bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, shadow_map_binding, tph::descriptor_type::image_sampler});

    cpt::render_texture_ptr diffuse_map{cpt::make_render_texture(640, 480, tph::sampling_options{})};
    cpt::view_ptr diffuse_map_view{cpt::make_view(diffuse_map, diffuse_info)};
    diffuse_map_view->fit_to(diffuse_map);
    diffuse_map_view->add_uniform_binding(directional_light_binding, light_buffer);
    diffuse_map_view->add_uniform_binding(shadow_map_binding, shadow_map);

    //Display
    cpt::render_window_ptr window{cpt::engine::instance().make_window("Captal test", cpt::video_mode{640, 480})};
    window->get_target().set_clear_color_value(1.0f, 1.0f, 1.0f);

    cpt::physical_world_ptr physical_world{cpt::make_physical_world()};
    physical_world->set_damping(std::numeric_limits<float>::epsilon());
    add_logic(window, world, physical_world, camera);

    entt::registry window_world{};

    cpt::view_ptr window_view{cpt::make_view(window)};
    window_view->fit_to(window);

    auto window_camera{window_world.create()};
    window_world.assign<cpt::components::node>(window_camera).set_origin(window->width() / 2.0f, window->height() / 2.0f);
    window_world.get<cpt::components::node>(window_camera).move_to(320.0f, 240.0f, 1.0f);
    window_world.assign<cpt::components::camera>(window_camera).attach(window_view);

    auto window_diffuse{window_world.create()};
    window_world.assign<cpt::components::node>(window_diffuse);
    window_world.assign<cpt::components::drawable>(window_diffuse).attach(cpt::make_sprite(640, 480));
    window_world.get<cpt::components::drawable>(window_diffuse).attachment()->set_texture(diffuse_map);

    window->on_mouse_wheel_scroll().connect([&window_world, window_camera](const apr::mouse_event& event)
    {
        if(event.wheel > 0)
            window_world.get<cpt::components::node>(window_camera).scale(1.0f / 3.0f);
        else
            window_world.get<cpt::components::node>(window_camera).scale(3.0f / 1.0f);
    });

    float angle{};
    cpt::engine::instance().on_update().connect([&angle, &light, &light_buffer](float time)
    {
        angle = std::fmod(angle + (time / 4.0f), cpt::pi<float>);
        light.direction = glm::vec4{std::cos(angle), std::cos(angle) / 2.0f, -std::abs(std::sin(angle)), 0.0f};

        const glm::vec4 color_factor{1.0f, 0.75f + std::min(-light.direction.z / 2.0f, 0.25f), 0.5f + std::min(-light.direction.z, 0.5f), 1.0f};
        light.ambiant = color_factor * glm::vec4{0.25f, 0.25f, 0.25f, 1.0f};
        light.diffuse = color_factor * glm::vec4{0.9f, 0.9f, 0.9f, 1.0f};
        light.specular = color_factor * glm::vec4{0.75f, 0.75f, 0.75f, 1.0f};

        light_buffer->upload();
    });

    window->on_key_pressed().connect([&angle](const apr::keyboard_event& event)
    {
        if(event.scan == apr::scancode::left)
            angle -= cpt::pi<float> / 32.0f;
        if(event.scan == apr::scancode::right)
            angle += cpt::pi<float> / 32.0f;
    });

    while(cpt::engine::instance().run())
    {
        if(window->is_rendering_enable())
        {
            cpt::systems::physics(world);
            cpt::systems::audio(world);
            cpt::systems::z_sorting(world);

            world.get<cpt::components::camera>(camera).attach(height_map_view);
            cpt::systems::render(world);
            height_map->present();

            cpt::systems::render(shadow_world);
            shadow_map->present();

            world.get<cpt::components::camera>(camera).attach(diffuse_map_view);
            cpt::systems::render(world);
            diffuse_map->present();

            cpt::systems::z_sorting(window_world);
            cpt::systems::render(window_world);
            window->present();

            cpt::systems::end_frame(window_world);
            cpt::systems::end_frame(world);
        }
    }

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
        cpt::engine engine{"captal_text", tph::version{1, 0, 0}, audio, graphics};

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
