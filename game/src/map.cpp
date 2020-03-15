#include "map.hpp"

#include <captal/sprite.hpp>
#include <captal/tilemap.hpp>

#include <captal/components/node.hpp>
#include <captal/components/drawable.hpp>
#include <captal/components/physical_body.hpp>
#include <captal/components/camera.hpp>

#include <captal/systems/render.hpp>
#include <captal/systems/frame.hpp>

namespace mpr
{

static constexpr float z_threshold{1.0f / 256.0f};

map::map(const std::filesystem::path& path)
:m_tiled_map{cpt::tiled::load_map(path)}
{
    const std::size_t layers_count{std::size(m_tiled_map.layers)};

    for(auto&& tileset : m_tiled_map.tilesets)
    {
        const cpt::tileset_ptr& ptr{m_tilesets.emplace_back(tileset.first_gid, cpt::make_tileset(std::move(tileset.image.value()), m_tiled_map.tile_width, m_tiled_map.tile_height)).second};

    }

    for(auto&& layer : m_tiled_map.layers)
    {
        const auto entity{m_world.create()};
        m_entities[layer.name] = entity;

        m_world.assign<cpt::components::node>(entity).move_to(glm::vec3{layer.position, 0.0f});
        m_world.assign<cpt::components::drawable>(entity);
        m_world.assign<cpt::components::physical_body>(entity);

        if(std::holds_alternative<cpt::tiled::layer::tiles>(layer.content))
        {
            cpt::tilemap_ptr tilemap{cpt::make_tilemap(m_tiled_map.width, m_tiled_map.height, m_tiled_map.tile_width, m_tiled_map.tile_height)};

            for(std::size_t i{}; i < m_tiled_map.width * m_tiled_map.height; ++i)
            {

            }
        }
    }
}

void map::render()
{
    //m_world.get<cpt::components::camera>(camera).attach(height_map_view);
    cpt::systems::render(m_world);

    cpt::systems::render(m_shadow_world);
    cpt::systems::end_frame(m_shadow_world);

    //world.get<cpt::components::camera>(camera).attach(diffuse_map_view);
    cpt::systems::render(m_world);

    m_height_map->present();
    m_shadow_map->present();
    m_diffuse_map->present();
}

void map::view(float x, float y, std::uint32_t width, std::uint32_t height)
{
    m_shadow_map_view->move_to(x, y);
    m_shadow_map_view->resize(static_cast<float>(width), static_cast<float>(height));
    m_shadow_map_view->set_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f);
}

void map::init_render()
{
    m_directional_light_buffer = cpt::make_framed_buffer(directional_light{});

    //Height map
    tph::shader height_vertex_shader{cpt::engine::instance().renderer(), tph::shader_stage::vertex, "shaders/height.vert.spv", tph::load_from_file};
    tph::shader height_fragment_shader{cpt::engine::instance().renderer(), tph::shader_stage::fragment, "shaders/height.frag.spv", tph::load_from_file};
    cpt::render_technique_info height_info{};
    height_info.stages.push_back(tph::pipeline_shader_stage{height_vertex_shader});
    height_info.stages.push_back(tph::pipeline_shader_stage{height_fragment_shader});
    height_info.stages_bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, height_map_binding, tph::descriptor_type::image_sampler});

    m_height_map = cpt::make_render_texture(1920, 540, tph::sampling_options{});
    m_height_map->get_target().set_clear_color_value(0.0f, 0.0f, 0.0f, 1.0f);
    m_height_map_view = cpt::make_view(m_height_map, height_info);
    m_height_map_view->fit_to(m_height_map);

    //Shadow map
    tph::shader shadow_vertex_shader{cpt::engine::instance().renderer(), tph::shader_stage::vertex, "shaders/shadow.vert.spv", tph::load_from_file};
    tph::shader shadow_fragment_shader{cpt::engine::instance().renderer(), tph::shader_stage::fragment, "shaders/shadow.frag.spv", tph::load_from_file};
    cpt::render_technique_info shadow_info{};
    shadow_info.stages.push_back(tph::pipeline_shader_stage{shadow_vertex_shader});
    shadow_info.stages.push_back(tph::pipeline_shader_stage{shadow_fragment_shader});
    shadow_info.stages_bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, height_map_binding, tph::descriptor_type::image_sampler});
    shadow_info.stages_bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, directional_light_binding, tph::descriptor_type::uniform_buffer});

    cpt::render_texture_ptr m_shadow_map = cpt::make_render_texture(1920, 540, tph::sampling_options{});
    cpt::view_ptr m_shadow_map_view = cpt::make_view(m_shadow_map, shadow_info);
    m_shadow_map_view->fit_to(m_shadow_map);
    m_shadow_map_view->add_uniform_binding(directional_light_binding, m_directional_light_buffer);

    auto shadow_camera{m_shadow_world.create()};
    m_shadow_world.assign<cpt::components::node>(shadow_camera).set_origin(m_shadow_map->width() / 2.0f, m_shadow_map->height() / 2.0f);
    m_shadow_world.get<cpt::components::node>(shadow_camera).move_to(m_shadow_map->width() / 2.0f, m_shadow_map->height() / 2.0f, 1.0f);
    m_shadow_world.assign<cpt::components::camera>(shadow_camera).attach(m_shadow_map_view);

    auto shadow_sprite{m_shadow_world.create()};
    m_shadow_world.assign<cpt::components::node>(shadow_sprite);
    m_shadow_world.assign<cpt::components::drawable>(shadow_sprite).attach(cpt::make_sprite(1920, 540));
    m_shadow_world.get<cpt::components::drawable>(shadow_sprite).attachment()->add_uniform_binding(height_map_binding, m_height_map);

    //Color "diffuse" map
    tph::shader diffuse_vertex_shader{cpt::engine::instance().renderer(), tph::shader_stage::vertex, "shaders/lighting.vert.spv", tph::load_from_file};
    tph::shader diffuse_fragment_shader{cpt::engine::instance().renderer(), tph::shader_stage::fragment, "shaders/lighting.frag.spv", tph::load_from_file};
    cpt::render_technique_info diffuse_info{};
    diffuse_info.stages.push_back(tph::pipeline_shader_stage{diffuse_vertex_shader});
    diffuse_info.stages.push_back(tph::pipeline_shader_stage{diffuse_fragment_shader});
    diffuse_info.stages_bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, normal_map_binding, tph::descriptor_type::image_sampler});
    diffuse_info.stages_bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, specular_map_binding, tph::descriptor_type::image_sampler});
    diffuse_info.stages_bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, directional_light_binding, tph::descriptor_type::uniform_buffer});
    diffuse_info.stages_bindings.push_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, shadow_map_binding, tph::descriptor_type::image_sampler});

    cpt::render_texture_ptr m_diffuse_map = cpt::make_render_texture(1920, 540, tph::sampling_options{});
    cpt::view_ptr m_diffuse_map_view = cpt::make_view(m_diffuse_map, diffuse_info);
    m_diffuse_map_view->fit_to(m_diffuse_map);
    m_diffuse_map_view->add_uniform_binding(directional_light_binding, m_directional_light_buffer);
    m_diffuse_map_view->add_uniform_binding(shadow_map_binding, m_shadow_map);

}

}
