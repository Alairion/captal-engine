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
