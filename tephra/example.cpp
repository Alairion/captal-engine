#include <iostream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <random>

#include <tephra/application.hpp>
#include <tephra/renderer.hpp>
#include <tephra/render_target.hpp>
#include <tephra/shader.hpp>
#include <tephra/descriptor.hpp>
#include <tephra/buffer.hpp>
#include <tephra/image.hpp>
#include <tephra/texture.hpp>
#include <tephra/pipeline.hpp>
#include <tephra/synchronization.hpp>
#include <tephra/commands.hpp>
#include <tephra/debug_utils.hpp>

#include <captal_foundation/math.hpp>

struct vertex
{
    tph::vec2f position{};
    tph::vec2f texture_coord{};
    tph::vec4f color{};
};

struct uniform_buffer_object
{
    tph::mat4f model{};
    tph::mat4f view{};
    tph::mat4f proj{};
};

static const std::array vertices
{
    vertex{tph::vec2f{-1.0f, -1.0f}, tph::vec2f{0.0f, 0.0f}, tph::vec4f{0.0f, 0.0f, 1.0f, 1.0f}},
    vertex{tph::vec2f{-1.0f, 1.0f},  tph::vec2f{0.0f, 1.0f}, tph::vec4f{1.0f, 1.0f, 0.0f, 1.0f}},
    vertex{tph::vec2f{1.0f, 1.0f},   tph::vec2f{1.0f, 1.0f}, tph::vec4f{1.0f, 0.0f, 0.0f, 1.0f}},
    vertex{tph::vec2f{-1.0f, -1.0f}, tph::vec2f{0.0f, 0.0},  tph::vec4f{0.0f, 0.0f, 1.0f, 1.0f}},
    vertex{tph::vec2f{1.0f, 1.0f},   tph::vec2f{1.0f, 1.0f}, tph::vec4f{1.0f, 0.0f, 0.0f, 1.0f}},
    vertex{tph::vec2f{1.0f, -1.0f},  tph::vec2f{1.0f, 0.0f}, tph::vec4f{0.0f, 1.0f, 0.0f, 1.0f}},
};

static uniform_buffer_object ubo
{
    tph::rotate(0.0f, tph::vec3f{0.0f, 0.0f, 1.0f}),
    tph::mat4f{tph::identity},
    tph::mat4f{tph::identity}
};

inline constexpr auto color_format{tph::texture_format::r8g8b8a8_srgb};

static void run()
{
    //Enable use of the bitwise ops on enabled enumerations
    using namespace cpt::foundation::enum_operations;

    //Register our application to the driver
    tph::application application{"tephra_test", tph::version{1, 0, 0}, tph::application_layer::validation, tph::application_extension::debug_utils};

    //Select a physical device, a GPU
    const tph::physical_device& physical_device{application.default_physical_device()};
    tph::debug_messenger messenger{application, tph::debug_messenger_default_callback, tph::debug_message_severity::error, tph::debug_message_type::validation};

    //Creates the renderer, a virtual link to the physical device
    tph::renderer renderer{physical_device, tph::renderer_layer::validation, tph::renderer_extension::none};

    //Create the render pass, it describes the operations done between each subpasses.
    tph::render_pass_info render_pass_info{};

    auto& color_attachement{render_pass_info.attachments.emplace_back()};
    color_attachement.format = color_format;
    color_attachement.sample_count = tph::sample_count::msaa_x1;
    color_attachement.load_op = tph::attachment_load_op::clear;
    color_attachement.store_op = tph::attachment_store_op::store;
    color_attachement.stencil_load_op = tph::attachment_load_op::clear;
    color_attachement.stencil_store_op = tph::attachment_store_op::dont_care;
    color_attachement.initial_layout = tph::texture_layout::undefined;
    color_attachement.final_layout = tph::texture_layout::transfer_source_optimal;

    auto& subpass{render_pass_info.subpasses.emplace_back()};
    subpass.color_attachments.emplace_back(tph::attachment_reference{0, tph::texture_layout::color_attachment_optimal});

    tph::render_pass render_pass{renderer, render_pass_info};

    //The shader of our pipeline
    tph::shader vertex_shader  {renderer, tph::shader_stage::vertex,   u8"vertex.vert.spv"};
    tph::shader fragment_shader{renderer, tph::shader_stage::fragment, u8"fragment.frag.spv"};

    //The shaders bindings
    std::vector<tph::descriptor_set_layout_binding> bindings{};
    bindings.emplace_back(tph::shader_stage::vertex,   0, tph::descriptor_type::uniform_buffer);
    bindings.emplace_back(tph::shader_stage::fragment, 1, tph::descriptor_type::image_sampler);
    tph::descriptor_set_layout descriptor_set_layout{renderer, bindings};

    //The pipeline layout
    tph::pipeline_layout pipeline_layout{renderer, std::span{&descriptor_set_layout, 1}};

    //Finally, the pipeline itself
    tph::graphics_pipeline_info pipeline_info{};
    pipeline_info.stages.push_back(tph::pipeline_shader_stage{vertex_shader});
    pipeline_info.stages.push_back(tph::pipeline_shader_stage{fragment_shader});
    pipeline_info.vertex_input.bindings.push_back(tph::vertex_input_binding{0, sizeof(vertex)});
    pipeline_info.vertex_input.attributes.push_back(tph::vertex_input_attribute{0, 0, tph::vertex_format::vec2f, offsetof(vertex, position)});
    pipeline_info.vertex_input.attributes.push_back(tph::vertex_input_attribute{1, 0, tph::vertex_format::vec2f, offsetof(vertex, texture_coord)});
    pipeline_info.vertex_input.attributes.push_back(tph::vertex_input_attribute{2, 0, tph::vertex_format::vec4f, offsetof(vertex, color)});
    pipeline_info.viewport.viewport_count = 1;
    pipeline_info.viewport.viewports.push_back(tph::viewport{0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f});
    pipeline_info.viewport.scissors.push_back(tph::scissor{0, 0, 640, 480});
    pipeline_info.color_blend.attachments.push_back(tph::pipeline_color_blend_attachment{true});
    tph::pipeline pipeline{renderer, render_pass, pipeline_info, pipeline_layout};

    //CPU side data
    tph::buffer staging_buffer{renderer, sizeof(ubo) + sizeof(vertices), tph::buffer_usage::staging | tph::buffer_usage::transfer_source};
    auto* buffer_data{staging_buffer.map()};
    std::memcpy(buffer_data, &ubo, sizeof(ubo));
    std::memcpy(buffer_data + sizeof(ubo), &vertices, sizeof(vertices));

    tph::image image{renderer, std::filesystem::path{u8"fronce.jpg"}, tph::image_usage::transfer_source};

    //GPU side data
    constexpr tph::texture_info texture_info{tph::texture_format::r8g8b8a8_srgb, tph::texture_usage::sampled | tph::texture_usage::transfer_destination};
    constexpr tph::texture_info target_info {color_format, tph::texture_usage::color_attachment | tph::texture_usage::transfer_source};
    constexpr tph::sampler_info sampler_info{tph::filter::linear, tph::filter::linear};
    constexpr tph::buffer_usage buffer_usage{tph::buffer_usage::device_only | tph::buffer_usage::vertex | tph::buffer_usage::uniform | tph::buffer_usage::transfer_destination};

    tph::buffer buffer{renderer, std::size(vertices) * sizeof(vertex) + sizeof(ubo), buffer_usage};

    tph::texture      texture{renderer, static_cast<std::uint32_t>(image.width()), static_cast<std::uint32_t>(image.height()), texture_info};
    tph::sampler      sampler{renderer, sampler_info};
    tph::texture_view view{renderer, texture};

    tph::texture      target{renderer, 640, 480, target_info};
    tph::texture_view target_view{renderer, target};

    //The descriptor set, to tell the shaders what resources to use
    constexpr std::array pool_sizes
    {
        tph::descriptor_pool_size{tph::descriptor_type::uniform_buffer, 1},
        tph::descriptor_pool_size{tph::descriptor_type::image_sampler, 1}
    };

    tph::descriptor_pool descriptor_pool{renderer, pool_sizes};

    tph::descriptor_set descriptor_set{renderer, descriptor_pool, descriptor_set_layout};
    const std::array writes
    {
        tph::descriptor_write{descriptor_set, 0, 0, tph::descriptor_type::uniform_buffer, tph::descriptor_buffer_info {buffer, 0, sizeof(ubo)}},
        tph::descriptor_write{descriptor_set, 1, 0, tph::descriptor_type::image_sampler,  tph::descriptor_texture_info{&sampler, &view, tph::texture_layout::shader_read_only_optimal}}
    };

    tph::write_descriptors(renderer, writes);

    //The output
    tph::image output{renderer, 640, 480, tph::image_usage::transfer_destination};

    const std::array attachments{std::ref(target_view)};
    tph::framebuffer framebuffer{renderer, render_pass, attachments, 640, 480, 1};

    tph::command_pool   command_pool  {renderer};
    tph::command_buffer command_buffer{tph::cmd::begin(command_pool, tph::command_buffer_level::primary, tph::command_buffer_options::one_time_submit)};

    tph::cmd::copy(command_buffer, staging_buffer, buffer, tph::buffer_copy{0, 0, std::size(vertices) * sizeof(vertex) +  sizeof(ubo)});

    const std::array first_barrier{tph::texture_memory_barrier
    {
        texture, {},
        tph::resource_access::none, tph::resource_access::transfer_write,
        tph::texture_layout::undefined, tph::texture_layout::transfer_destination_optimal
    }};

    const std::array second_barrier{tph::texture_memory_barrier
    {
        texture, {},
        tph::resource_access::transfer_write, tph::resource_access::shader_read,
        tph::texture_layout::transfer_destination_optimal, tph::texture_layout::shader_read_only_optimal
    }};

    tph::cmd::pipeline_barrier(command_buffer, tph::pipeline_stage::top_of_pipe, tph::pipeline_stage::transfer, tph::dependency_flags::none, {}, {}, first_barrier);
    tph::cmd::copy(command_buffer, image, texture, tph::image_texture_copy{{}, {}, tph::copy_extent{640, 480, 1}});
    tph::cmd::pipeline_barrier(command_buffer, tph::pipeline_stage::transfer, tph::pipeline_stage::fragment_shader, tph::dependency_flags::none, {}, {}, second_barrier);

    tph::cmd::begin_render_pass(command_buffer, render_pass, framebuffer);
    tph::cmd::bind_pipeline(command_buffer, pipeline);
    tph::cmd::bind_vertex_buffer(command_buffer, buffer, sizeof(ubo));
    tph::cmd::bind_descriptor_set(command_buffer, 0, descriptor_set, pipeline_layout);
    tph::cmd::draw(command_buffer, std::size(vertices), 1, 0, 0);
    tph::cmd::end_render_pass(command_buffer);

    tph::cmd::copy(command_buffer, target, output, tph::image_texture_copy{{}, {}, tph::copy_extent{640, 480, 1}});

    tph::cmd::end(command_buffer);

    tph::submit_info submit{};
    submit.command_buffers.push_back(command_buffer);

    tph::fence fence{renderer};
    tph::submit(renderer, submit, fence);
    fence.wait();

    output.write("test.png", tph::image_format::png);
}

int main()
{
    try
    {
        run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "An error has occured: " << e.what() << std::endl;
    }
}
