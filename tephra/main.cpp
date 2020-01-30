#include <iostream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <random>

#include "src/application.hpp"
#include "src/renderer.hpp"
#include "src/render_target.hpp"
#include "src/shader.hpp"
#include "src/descriptor.hpp"
#include "src/buffer.hpp"
#include "src/image.hpp"
#include "src/texture.hpp"
#include "src/pipeline.hpp"
#include "src/commands.hpp"

#include "window.hpp"
#include "vertex.hpp"

constexpr std::size_t image_count{2};

static const std::array vertices
{
    utils::vertex{{-0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 0.5f}},
    utils::vertex{{-0.5f, 0.5f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
    utils::vertex{{0.5f, 0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
    utils::vertex{{-0.5f, -0.5f}, {0.0f, 0.0}, {1.0f, 1.0f, 1.0f, 0.5f}},
    utils::vertex{{0.5f, 0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
    utils::vertex{{0.5f, -0.5f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 0.5f}},
};

static utils::uniform_buffer_object ubo
{
    glm::rotate(glm::mat4{1.0f}, glm::radians(180.0f), glm::vec3{0.0f, 0.0f, 1.0f}),
    glm::mat4{1.0f},
    glm::mat4{1.0f}
};

void offsreen_run()
{
    tph::application application{u8"tephra_test", tph::version{1, 0, 0}, tph::application_options::enable_validation};
    const tph::physical_device& physical_device{application.default_physical_device()};
    tph::renderer renderer{application, physical_device, tph::renderer_options::none};

    tph::texture target_texture{renderer, 640, 480, tph::texture_usage::color_attachment | tph::texture_usage::transfer_source};
    tph::render_target render_target{renderer, target_texture, tph::render_target_options::clipping};
    render_target.set_clear_color_value(0.0f, 0.0f, 0.0f, 0.0f);

    tph::buffer staging_buffer{renderer, sizeof(ubo) + sizeof(vertices), tph::buffer_usage::staging | tph::buffer_usage::transfer_source};
    auto* buffer_data{staging_buffer.map()};
    std::memcpy(buffer_data, &ubo, sizeof(ubo));
    std::memcpy(buffer_data + sizeof(ubo), &vertices, sizeof(vertices));
    tph::buffer entity_buffer{renderer, std::size(vertices) * sizeof(utils::vertex) + sizeof(ubo), tph::buffer_usage::device_only | tph::buffer_usage::vertex | tph::buffer_usage::uniform | tph::buffer_usage::transfer_destination};

    tph::image image{renderer, "hum.png", tph::load_from_file, tph::image_usage::transfer_source};
    tph::texture texture{renderer, static_cast<std::uint32_t>(image.width()), static_cast<std::uint32_t>(image.height()), tph::sampling_options{tph::filter::linear}, tph::texture_usage::sampled | tph::texture_usage::transfer_destination};

    tph::shader vertex_shader{renderer, tph::shader_stage::vertex, "shaders/vertex.vert.spv", tph::load_from_file};
    tph::shader fragment_shader{renderer, tph::shader_stage::fragment, "shaders/fragment.frag.spv", tph::load_from_file};

    std::vector<tph::descriptor_set_layout_binding> bindings{};
    bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::vertex, 0, tph::descriptor_type::uniform_buffer});
    bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, 1, tph::descriptor_type::image_sampler});
    tph::descriptor_set_layout descriptor_set_layout{renderer, bindings};

    std::vector<tph::descriptor_pool_size> pool_sizes{};
    pool_sizes.push_back(tph::descriptor_pool_size{tph::descriptor_type::uniform_buffer});
    pool_sizes.push_back(tph::descriptor_pool_size{tph::descriptor_type::image_sampler});
    tph::descriptor_pool descriptor_pool{renderer, pool_sizes};

    tph::descriptor_set descriptor_set{tph::descriptor_set{renderer, descriptor_pool, descriptor_set_layout}};
    tph::write_descriptor(renderer, descriptor_set, 0, entity_buffer, 0, sizeof(ubo));
    tph::write_descriptor(renderer, descriptor_set, 1, texture);

    tph::pipeline_layout pipeline_layout{renderer, {descriptor_set_layout}};

    tph::pipeline_info pipeline_info{};
    pipeline_info.stages.push_back(tph::pipeline_shader_stage{vertex_shader});
    pipeline_info.stages.push_back(tph::pipeline_shader_stage{fragment_shader});
    pipeline_info.vertex_input.bindings.push_back(tph::vertex_input_binding{0, sizeof(utils::vertex)});
    pipeline_info.vertex_input.attributes.push_back(tph::vertex_input_attribute{0, 0, tph::vertex_format::vec2f, offsetof(utils::vertex, position)});
    pipeline_info.vertex_input.attributes.push_back(tph::vertex_input_attribute{1, 0, tph::vertex_format::vec2f, offsetof(utils::vertex, texture_coord)});
    pipeline_info.vertex_input.attributes.push_back(tph::vertex_input_attribute{2, 0, tph::vertex_format::vec4f, offsetof(utils::vertex, color)});
    pipeline_info.viewport.viewport_count = 1;
    pipeline_info.viewport.viewports.push_back(tph::viewport{0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f});
    pipeline_info.viewport.scissors.push_back(tph::scissor{0, 0, 640, 480});
    pipeline_info.color_blend.attachments.push_back(tph::pipeline_color_blend_attachment{true});
    tph::pipeline pipeline{renderer, render_target, pipeline_info, pipeline_layout};

    tph::image render_image{renderer, 640, 480, tph::image_usage::transfer_destination};

    tph::command_pool command_pool{renderer};

    tph::command_buffer command_buffer{tph::cmd::begin(command_pool, tph::command_buffer_level::primary, tph::command_buffer_flags::one_time_submit)};
    tph::cmd::copy(command_buffer, staging_buffer, entity_buffer);
    tph::cmd::copy(command_buffer, image, texture);
    tph::cmd::prepare(command_buffer, texture, tph::pipeline_stage::fragment_shader);

    tph::cmd::begin_render_pass(command_buffer, render_target, 0);
    tph::cmd::bind_pipeline(command_buffer, pipeline);
    tph::cmd::bind_vertex_buffer(command_buffer, entity_buffer, sizeof(ubo));
    tph::cmd::bind_descriptor_set(command_buffer, descriptor_set, pipeline_layout);
    tph::cmd::draw(command_buffer, std::size(vertices), 1, 0, 0);
    tph::cmd::end_render_pass(command_buffer);

    tph::cmd::copy(command_buffer, target_texture, render_image);
    tph::cmd::end(command_buffer);

    tph::submit_info submit{};
    submit.command_buffers.push_back(command_buffer);

    tph::fence fence{renderer};
    tph::submit(renderer, submit, fence);
    fence.wait();

    render_image.write("test.png", tph::image_format::png);
}

void run()
{
    //Create an application. This object is used to setup a renderer.
    tph::application application{u8"tephra_test", tph::version{1, 0, 0}, tph::application_options::enable_validation};

    //Create a window. You can use any library to create a window, this example uses SDL2.
    utils::window window{};

    //The surface is a link to the window.
    //Once again, create the surface as you want, this example uses SDL_Vulkan_CreateSurface.
    tph::surface surface{window.make_surface(application)};

    //You can choose a physical device in different way.
    //Here we use tph::application::default_physical_device with a surface, which returns a default physical device that supports presentation for the specified surface.
    const tph::physical_device& physical_device{application.default_physical_device(surface)};

    //The renderer is the logical representation of a physical device.
    //It is not named "device" because you can not use it for something else than rendering.
    tph::renderer renderer{application, physical_device, tph::renderer_options::none};

    //Create a render target on the surface (and indirectly on the window)
    //The fifo present mode corresponf to stong vertical sync and is guaranteed to be available.
    //If image count is 2, which correspond to double buffering. (not guaranteed to be available, but very common)
    //We enable clipping for better performances.
    tph::render_target render_target{renderer, surface, tph::present_mode::fifo, image_count, tph::render_target_options::clipping};
    render_target.set_clear_color_value(1.0f, 1.0f, 1.0f);

    tph::buffer staging_buffer{renderer, sizeof(ubo) + sizeof(vertices), tph::buffer_usage::staging | tph::buffer_usage::transfer_source};
    auto* buffer_data{staging_buffer.map()};
    std::memcpy(buffer_data, &ubo, sizeof(ubo));
    std::memcpy(buffer_data + sizeof(ubo), &vertices, sizeof(vertices));
    tph::buffer entity_buffer{renderer, std::size(vertices) * sizeof(utils::vertex) + sizeof(ubo), tph::buffer_usage::device_only | tph::buffer_usage::vertex | tph::buffer_usage::uniform | tph::buffer_usage::transfer_destination};

    tph::image image{renderer, "hum.png", tph::load_from_file, tph::image_usage::transfer_source};
    tph::texture texture{renderer, static_cast<std::uint32_t>(image.width()), static_cast<std::uint32_t>(image.height()), tph::sampling_options{tph::filter::linear}, tph::texture_usage::sampled | tph::texture_usage::transfer_destination};

    tph::shader vertex_shader{renderer, tph::shader_stage::vertex, "shaders/vertex.vert.spv", tph::load_from_file};
    tph::shader fragment_shader{renderer, tph::shader_stage::fragment, "shaders/fragment.frag.spv", tph::load_from_file};

    std::vector<tph::descriptor_set_layout_binding> bindings{};
    bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::vertex, 0, tph::descriptor_type::uniform_buffer});
    bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, 1, tph::descriptor_type::image_sampler});
    tph::descriptor_set_layout descriptor_set_layout{renderer, bindings};

    std::vector<tph::descriptor_pool_size> pool_sizes{};
    pool_sizes.push_back(tph::descriptor_pool_size{tph::descriptor_type::uniform_buffer, image_count});
    pool_sizes.push_back(tph::descriptor_pool_size{tph::descriptor_type::image_sampler, image_count});
    tph::descriptor_pool descriptor_pool{renderer, pool_sizes, image_count};

    std::array<tph::descriptor_set, image_count> descriptor_sets{};
    for(auto& descriptor_set : descriptor_sets)
    {
        descriptor_set = tph::descriptor_set{renderer, descriptor_pool, descriptor_set_layout};
        tph::write_descriptor(renderer, descriptor_set, 0, entity_buffer, 0, sizeof(ubo));
        tph::write_descriptor(renderer, descriptor_set, 1, texture);
    }

    tph::pipeline_layout pipeline_layout{renderer, {descriptor_set_layout}};

    tph::pipeline_info pipeline_info{};
    pipeline_info.stages.push_back(tph::pipeline_shader_stage{vertex_shader});
    pipeline_info.stages.push_back(tph::pipeline_shader_stage{fragment_shader});
    pipeline_info.vertex_input.bindings.push_back(tph::vertex_input_binding{0, sizeof(utils::vertex)});
    pipeline_info.vertex_input.attributes.push_back(tph::vertex_input_attribute{0, 0, tph::vertex_format::vec2f, offsetof(utils::vertex, position)});
    pipeline_info.vertex_input.attributes.push_back(tph::vertex_input_attribute{1, 0, tph::vertex_format::vec2f, offsetof(utils::vertex, texture_coord)});
    pipeline_info.vertex_input.attributes.push_back(tph::vertex_input_attribute{2, 0, tph::vertex_format::vec4f, offsetof(utils::vertex, color)});
    pipeline_info.viewport.viewport_count = 1;
    pipeline_info.color_blend.attachments.push_back(tph::pipeline_color_blend_attachment{true});
    pipeline_info.dynamic_states.push_back(tph::dynamic_state::viewport);
    pipeline_info.dynamic_states.push_back(tph::dynamic_state::scissor);
    tph::pipeline pipeline{renderer, render_target, pipeline_info, pipeline_layout};

    tph::command_pool command_pool{renderer};

    tph::command_buffer transfer_command_buffer{tph::cmd::begin(command_pool, tph::command_buffer_level::primary, tph::command_buffer_flags::one_time_submit)};
    tph::cmd::copy(transfer_command_buffer, staging_buffer, entity_buffer);
    tph::cmd::copy(transfer_command_buffer, image, texture);
    tph::cmd::prepare(transfer_command_buffer, texture, tph::pipeline_stage::fragment_shader);
    tph::cmd::end(transfer_command_buffer);

    tph::fence tranfer_ends{renderer};

    tph::submit_info submit_info{};
    submit_info.command_buffers.push_back(transfer_command_buffer);
    tph::submit(renderer, submit_info, tranfer_ends);

    tranfer_ends.wait();

    std::array<tph::command_buffer, image_count> command_buffers{};

    const auto record_command_buffers = [&]()
    {
        const auto window_size{window.size()};

        command_pool.reset();

        std::size_t image_index{};
        for(tph::command_buffer& command_buffer : command_buffers)
        {
            command_buffer = tph::cmd::begin(command_pool);

            tph::cmd::begin_render_pass(command_buffer, render_target, image_index);
            tph::cmd::set_viewport(command_buffer, tph::viewport{0.0f, 0.0f, static_cast<float>(window_size.first), static_cast<float>(window_size.second), 0.0f, 1.0f});
            tph::cmd::set_scissor(command_buffer, tph::scissor{0, 0, window_size.first, window_size.second});
            tph::cmd::bind_pipeline(command_buffer, pipeline);
            tph::cmd::bind_vertex_buffer(command_buffer, entity_buffer, sizeof(ubo));
            tph::cmd::bind_descriptor_set(command_buffer, descriptor_sets[image_index], pipeline_layout);
            tph::cmd::draw(command_buffer, std::size(vertices), 1, 0, 0);
            tph::cmd::end_render_pass(command_buffer);

            tph::cmd::end(command_buffer);

            image_index = (image_index + 1) % image_count;
        }
    };

    record_command_buffers();

    std::array<tph::fence, image_count> fences{};
    for(auto& fence : fences)
        fence = tph::fence{renderer, true};

    std::array<tph::semaphore, image_count> image_available_semaphores{};
    for(auto& semaphore : image_available_semaphores)
        semaphore = tph::semaphore{renderer};

    std::array<tph::semaphore, image_count> render_finished_semaphores{};
    for(auto& semaphore : render_finished_semaphores)
        semaphore = tph::semaphore{renderer};

    std::size_t image_index{};
    std::uint32_t frame_count{};
    bool reset{};
    auto tp1{std::chrono::system_clock::now()};

    while(window.update())
    {
        if(reset)
        {
            image_index = 0;
            reset = false;
        }

        if(std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::system_clock::now() - tp1).count() >= 1.0)
        {
            window.change_title("Tephra - " + std::to_string(frame_count) + " FPS");
            frame_count = 0;
            tp1 = std::chrono::system_clock::now();
        }

        fences[image_index].wait();

        if(tph::render_target_status status{render_target.acquire(image_available_semaphores[image_index], tph::nullref)}; status == tph::render_target_status::out_of_date || status == tph::render_target_status::surface_lost)
        {
            renderer.wait();

            if(surface.size(renderer) == std::make_pair(0u, 0u))
                if(!window.wait_restore())
                    break;

            render_target.recreate();
            record_command_buffers();
            reset = true;

            continue;
        }

        tph::submit_info submit_info{};
        submit_info.wait_semaphores.push_back(image_available_semaphores[image_index]);
        submit_info.wait_stages.push_back(tph::pipeline_stage::color_attachment_output);
        submit_info.command_buffers.push_back(command_buffers[image_index]);
        submit_info.signal_semaphores.push_back(render_finished_semaphores[image_index]);

        fences[image_index].reset();
        tph::submit(renderer, submit_info, fences[image_index]);

        if(tph::render_target_status status{render_target.present(render_finished_semaphores[image_index])}; status != tph::render_target_status::valid)
        {
            renderer.wait();

            if(surface.size(renderer) == std::make_pair(0u, 0u))
                if(!window.wait_restore())
                    break;

            render_target.recreate();
            record_command_buffers();
            reset = true;
        }

        ++frame_count;
        image_index = (image_index + 1) % image_count;
    }

    renderer.wait();
}

int main()
{
    try
    {
        offsreen_run();
        //run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "An error has occured: " << e.what() << std::endl;
    }
}
