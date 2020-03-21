#include "map.hpp"

#include <iostream>

#include <captal/sprite.hpp>
#include <captal/tilemap.hpp>

#include <captal/components/node.hpp>
#include <captal/components/drawable.hpp>
#include <captal/components/physical_body.hpp>
#include <captal/components/camera.hpp>
#include <captal/components/listener.hpp>
#include <captal/components/draw_index.hpp>

#include <captal/systems/render.hpp>
#include <captal/systems/frame.hpp>

namespace mpr
{

map::map(const std::filesystem::path& path)
:m_tiled_map{cpt::tiled::load_map(path)}
{
    init_render();

    for(auto&& tileset : m_tiled_map.tilesets)
    {
        m_tilesets.emplace_back(tileset.first_gid, cpt::make_tileset(std::move(tileset.image.value()), m_tiled_map.tile_width, m_tiled_map.tile_height));
    }

    parse_layers(m_tiled_map.layers, 0);
}

void map::render()
{
    m_world.get<cpt::components::camera>(m_camera).attach(m_height_map_view);
    cpt::systems::render(m_world);

    cpt::systems::render(m_shadow_world);
    cpt::systems::end_frame(m_shadow_world);

    m_world.get<cpt::components::camera>(m_camera).attach(m_diffuse_map_view);
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
    m_shadow_map_view->set_scissor(0, 0, width, height);
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

    m_camera = m_world.create();
    m_world.assign<cpt::components::node>(m_camera);
    m_world.assign<cpt::components::camera>(m_camera);
    m_world.assign<cpt::components::listener>(m_camera);
}

std::uint64_t map::parse_layers(const std::vector<cpt::tiled::layer>& layers, std::uint64_t index)
{
    for(auto&& layer : layers)
    {
        const auto entity{m_world.create()};
        m_entities[layer.name] = entity;

        m_world.assign<cpt::components::node>(entity).move_to(glm::vec3{layer.position, 0.0f});
        m_world.assign<cpt::components::draw_index>(entity).index = index;
        m_world.assign<cpt::components::drawable>(entity);
        auto& layer_body = m_world.assign<cpt::components::physical_body>(entity, cpt::make_physical_body(m_physical_world, cpt::physical_body_type::steady));

        if(std::holds_alternative<cpt::tiled::layer::tiles>(layer.content))
        {
            const cpt::tiled::layer::tiles& tiles{std::get<cpt::tiled::layer::tiles>(layer.content)};
            parse_tiles(tiles, layer_body);

            ++index;
        }
        else if(std::holds_alternative<cpt::tiled::layer::objects>(layer.content))
        {
            const cpt::tiled::layer::objects& objects{std::get<cpt::tiled::layer::objects>(layer.content)};

            for(auto&& object : objects.objects)
            {
                parse_object(object, index);
            }

            ++index;
        }
        else if(std::holds_alternative<cpt::tiled::layer::group>(layer.content))
        {
            const cpt::tiled::layer::group& group{std::get<cpt::tiled::layer::group>(layer.content)};
            index = parse_layers(group.layers, index);
        }
    }

    return index;
}

cpt::tilemap_ptr map::parse_tiles(const cpt::tiled::layer::tiles& tiles, cpt::components::physical_body& body)
{
    cpt::tilemap_ptr tilemap{cpt::make_tilemap(m_tiled_map.width, m_tiled_map.height, m_tiled_map.tile_width, m_tiled_map.tile_height)};

    const auto it{std::find_if(std::begin(tiles.gid), std::end(tiles.gid), [](std::uint32_t value){return value != 0;})};
    if(it != std::end(tiles.gid))
    {
        const cpt::tiled::tileset& map_tileset{m_tiled_map.tilesets[tileset_index(*it)]};
        auto&& [first_gid, tileset] = tileset_from_gid(*it);

        for(std::size_t i{}; i < m_tiled_map.width * m_tiled_map.height; ++i)
        {
            const std::uint32_t lid{tiles.gid[i] - first_gid};

            if(lid != 0)
            {
                tilemap->set_texture_rect(i / m_tiled_map.width, i % m_tiled_map.width, tileset->compute_rect(lid));

                for(auto&& hitbox : map_tileset.tiles[lid].hitboxes)
                {
                    if(std::holds_alternative<cpt::tiled::object::square>(hitbox.content))
                    {
                        const auto& square{std::get<cpt::tiled::object::square>(hitbox.content)};
                        const float x{static_cast<float>((i % m_tiled_map.width) * m_tiled_map.tile_width) + square.position.x};
                        const float y{static_cast<float>((i / m_tiled_map.width) * m_tiled_map.tile_height) + square.position.y};

                        body.add_shape(glm::vec2{x, y}, glm::vec2{x + square.width, y});
                        body.add_shape(glm::vec2{x + square.width, y}, glm::vec2{x + square.width, y + square.height});
                        body.add_shape(glm::vec2{x + square.width, y + square.height}, glm::vec2{x, y + square.height});
                        body.add_shape(glm::vec2{x, y + square.height}, glm::vec2{x, y});
                    }
                }
            }
        }

        tilemap->set_texture(std::move(tileset));
        tilemap->add_uniform_binding(height_map_binding, load_height_map(map_tileset.properties));
        tilemap->add_uniform_binding(normal_map_binding, load_normal_map(map_tileset.properties));
        tilemap->add_uniform_binding(specular_map_binding, load_specular_map(map_tileset.properties));
    }

    return tilemap;
}

void map::parse_object(const cpt::tiled::object& object, std::uint64_t index [[maybe_unused]])
{
    if(object.type == "spawn")
    {

    }
}

const std::pair<uint32_t, cpt::tileset_ptr>& map::tileset_from_gid(std::uint32_t gid)
{
    for(const auto& tileset : m_tilesets)
    {
        if(tileset.first >= gid)
            return tileset;
    }

    throw std::runtime_error{"Invalid layer data (missing tileset)."};
}

std::size_t map::tileset_index(std::uint32_t gid)
{
    for(std::size_t i{}; i < std::size(m_tilesets); ++i)
    {
        if(m_tilesets[i].first >= gid)
            return i;
    }

    throw std::runtime_error{"Invalid layer data (missing tileset)."};
}

cpt::texture_ptr map::load_height_map(const cpt::tiled::properties_set& properties) const
{
    const auto it{properties.find("height_map")};
    if(it != std::end(properties))
    {
        return cpt::make_texture(std::get<std::filesystem::path>(it->second));
    }

    return cpt::make_texture(2, 2, std::data(dummy_height_map_data), tph::sampling_options{tph::filter::nearest, tph::filter::nearest, tph::address_mode::repeat});
}

cpt::texture_ptr map::load_normal_map(const cpt::tiled::properties_set& properties) const
{
    const auto it{properties.find("normal_map")};
    if(it != std::end(properties))
    {
        return cpt::make_texture(std::get<std::filesystem::path>(it->second));
    }

    return cpt::make_texture(2, 2, std::data(dummy_normal_map_data), tph::sampling_options{tph::filter::nearest, tph::filter::nearest, tph::address_mode::repeat});
}

cpt::texture_ptr map::load_specular_map(const cpt::tiled::properties_set& properties) const
{
    const auto it{properties.find("specular_map")};
    if(it != std::end(properties))
    {
        return cpt::make_texture(std::get<std::filesystem::path>(it->second));
    }

    return cpt::make_texture(2, 2, std::data(dummy_specular_map_data), tph::sampling_options{tph::filter::nearest, tph::filter::nearest, tph::address_mode::repeat});
}

}
