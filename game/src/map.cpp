#include "map.hpp"

#include <iostream>
#include <sstream>

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
#include <captal/systems/audio.hpp>
#include <captal/systems/sorting.hpp>
#include <captal/systems/physics.hpp>

namespace mpr
{

chunk::chunk(map& map, std::uint32_t x, std::uint32_t y)
:m_map{&map}
,m_x{x}
,m_y{y}
{
    const std::filesystem::path path{file_path()};

    if(std::filesystem::exists(path))
    {
        m_tiled_map = cpt::tiled::load_map(path);

        for(auto&& tileset : m_tiled_map.tilesets)
            m_tilesets.emplace_back(tileset.first_gid, cpt::tileset{map.texture_pool().load(tileset.image.source), m_tiled_map.tile_width, m_tiled_map.tile_height});

        parse_layers(m_tiled_map.layers, 0);
    }
}

std::pair<std::string, entt::entity> chunk::drain(std::string entity_name)
{
    const auto it{m_entities.find(entity_name)};
    if(it != std::end(m_entities))
    {
        auto output{std::move(*it)};
        m_entities.erase(it);

        return output;
    }

    return {};
}

void chunk::add_entity(std::pair<std::string, entt::entity> entity)
{
    m_entities.emplace(std::move(entity));
}

std::filesystem::path chunk::file_path()
{
    std::stringstream ss{};
    ss << std::setfill('0') << "maps/chunk_" << std::setw(3) << m_x << "_" << std::setw(3) << m_y << ".tmx";

    return std::filesystem::u8path(ss.str());
}

std::uint64_t chunk::parse_layers(const std::vector<cpt::tiled::layer>& layers, std::uint64_t index)
{
    for(auto&& layer : layers)
    {
        const auto entity{m_map->world().create()};
        m_entities[layer.name] = entity;

        m_map->world().assign<cpt::components::node>(entity).move_to(chunk_offset() + glm::vec3{layer.position, 0.0f});
        m_map->world().assign<cpt::components::draw_index>(entity).index = index;
        m_map->world().assign<cpt::components::drawable>(entity);
        auto& layer_body = m_map->world().assign<cpt::components::physical_body>(entity, cpt::make_physical_body(m_map->physical_world(), cpt::physical_body_type::steady));
        layer_body.attachment()->set_position(glm::vec2{chunk_offset()} + layer.position);

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

cpt::tilemap_ptr chunk::parse_tiles(const cpt::tiled::layer::tiles& tiles, cpt::components::physical_body& body)
{
    cpt::tilemap_ptr tilemap{cpt::make_tilemap(m_tiled_map.width, m_tiled_map.height, m_tiled_map.tile_width, m_tiled_map.tile_height)};

    const auto it{std::find_if(std::begin(tiles.gid), std::end(tiles.gid), [](std::uint32_t value){return value != 0;})};
    if(it != std::end(tiles.gid))
    {
        const cpt::tiled::tileset& map_tileset{m_tiled_map.tilesets[tileset_index(*it)]};
        auto&& [first_gid, tileset] = tileset_from_gid(*it);

        for(std::uint32_t i{}; i < m_tiled_map.width * m_tiled_map.height; ++i)
        {
            const std::uint32_t lid{tiles.gid[i] - first_gid};

            if(lid != 0)
            {
                tilemap->set_texture_rect(i / m_tiled_map.width, i % m_tiled_map.width, tileset.compute_rect(lid));

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

        tilemap->set_texture(tileset.texture());
        tilemap->add_uniform_binding(height_map_binding, load_height_map(map_tileset.properties));
        tilemap->add_uniform_binding(normal_map_binding, load_normal_map(map_tileset.properties));
        tilemap->add_uniform_binding(specular_map_binding, load_specular_map(map_tileset.properties));
        tilemap->add_uniform_binding(emission_map_binding, load_emission_map(map_tileset.properties));
    }

    return tilemap;
}

void chunk::parse_object(const cpt::tiled::object& object, std::uint64_t index [[maybe_unused]])
{
    if(object.type == "spawn")
    {

    }
}

const std::pair<std::uint32_t, cpt::tileset>& chunk::tileset_from_gid(std::uint32_t gid)
{
    for(const auto& tileset : m_tilesets)
    {
        if(tileset.first >= gid)
            return tileset;
    }

    throw std::runtime_error{"Invalid layer data (missing tileset)."};
}

std::size_t chunk::tileset_index(std::uint32_t gid)
{
    for(std::size_t i{}; i < std::size(m_tilesets); ++i)
    {
        if(m_tilesets[i].first >= gid)
            return i;
    }

    throw std::runtime_error{"Invalid layer data (missing tileset)."};
}

cpt::texture_ptr chunk::load_normal_map(const cpt::tiled::properties_set& properties) const
{
    const auto it{properties.find("normal_map")};
    if(it != std::end(properties))
    {
        return m_map->texture_pool().load(std::get<std::filesystem::path>(it->second));
    }

    return m_map->texture_pool().load(dummy_normal_map_name);
}

cpt::texture_ptr chunk::load_height_map(const cpt::tiled::properties_set& properties) const
{
    const auto it{properties.find("height_map")};
    if(it != std::end(properties))
    {
        return m_map->texture_pool().load(std::get<std::filesystem::path>(it->second));
    }

    return m_map->texture_pool().load(dummy_height_map_name);
}

cpt::texture_ptr chunk::load_specular_map(const cpt::tiled::properties_set& properties) const
{
    const auto it{properties.find("specular_map")};
    if(it != std::end(properties))
    {
        return m_map->texture_pool().load(std::get<std::filesystem::path>(it->second));
    }

    return m_map->texture_pool().load(dummy_specular_map_name);
}

cpt::texture_ptr chunk::load_emission_map(const cpt::tiled::properties_set& properties) const
{
    const auto it{properties.find("emission_map")};
    if(it != std::end(properties))
    {
        return m_map->texture_pool().load(std::get<std::filesystem::path>(it->second));
    }

    return m_map->texture_pool().load(dummy_emission_map_name);
}

map::map()
:m_physical_world{cpt::make_physical_world()}
{
    init_render();
    init_entities();

    m_physical_world->set_damping(0.1f);

    m_texture_pool.emplace(dummy_normal_map_name, cpt::make_texture(1, 1, std::data(dummy_normal_map_data), tph::sampling_options{tph::filter::nearest, tph::filter::nearest, tph::address_mode::repeat}));
    m_texture_pool.emplace(dummy_height_map_name, cpt::make_texture(1, 1, std::data(dummy_height_map_data), tph::sampling_options{tph::filter::nearest, tph::filter::nearest, tph::address_mode::repeat}));
    m_texture_pool.emplace(dummy_specular_map_name, cpt::make_texture(1, 1, std::data(dummy_specular_map_data), tph::sampling_options{tph::filter::nearest, tph::filter::nearest, tph::address_mode::repeat}));
    m_texture_pool.emplace(dummy_emission_map_name, cpt::make_texture(1, 1, std::data(dummy_emission_map_data), tph::sampling_options{tph::filter::nearest, tph::filter::nearest, tph::address_mode::repeat}));
}

void map::update(float time)
{
    m_physical_world->update(time);
}

void map::render()
{
    const auto camera_entity{m_entities.at(camera_entity_name)};
    auto& camera{m_world.get<cpt::components::camera>(camera_entity)};

    cpt::systems::physics(m_world);
    cpt::systems::audio(m_world);
    cpt::systems::index_z_sorting(m_world);

    camera.attach(m_height_map_view);
    cpt::systems::render(m_world);
    camera.attach(m_diffuse_map_view);
    cpt::systems::render(m_world);

    cpt::systems::end_frame(m_world);

    m_height_map->present();
    m_diffuse_map->present();
}

void map::view(float x, float y, std::uint32_t width, std::uint32_t height)
{
    const auto camera_entity{m_entities.at(camera_entity_name)};

    auto& camera_node{m_world.assign<cpt::components::node>(camera_entity)};
    camera_node.move_to(x, y);

    const auto& camera{m_world.assign<cpt::components::camera>(camera_entity).attachment()};
    camera->resize(static_cast<float>(width), static_cast<float>(height));
    camera->set_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f);
    camera->set_scissor(0, 0, width, height);
}

void map::init_render()
{
    m_lights_buffer = cpt::make_framed_buffer(uniform_lights{});

    //Height map
    tph::shader height_vertex_shader{cpt::engine::instance().renderer(), tph::shader_stage::vertex, std::filesystem::u8path(u8"shaders/height.vert.spv")};
    tph::shader height_fragment_shader{cpt::engine::instance().renderer(), tph::shader_stage::fragment, std::filesystem::u8path(u8"shaders/height.frag.spv")};
    cpt::render_technique_info height_info{};
    height_info.stages.emplace_back(tph::pipeline_shader_stage{height_vertex_shader});
    height_info.stages.emplace_back(tph::pipeline_shader_stage{height_fragment_shader});
    height_info.stages_bindings.emplace_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, height_map_binding, tph::descriptor_type::image_sampler});

    m_height_map = cpt::make_render_texture(1920, 540, tph::sampling_options{});
    m_height_map->get_target().set_clear_color_value(0.0f, 0.0f, 0.0f, 1.0f);
    m_height_map_view = cpt::make_view(m_height_map, height_info);
    m_height_map_view->fit_to(m_height_map);

    //Color "diffuse" map
    tph::shader diffuse_vertex_shader{cpt::engine::instance().renderer(), tph::shader_stage::vertex, std::filesystem::u8path(u8"shaders/lighting.vert.spv")};
    tph::shader diffuse_fragment_shader{cpt::engine::instance().renderer(), tph::shader_stage::fragment, std::filesystem::u8path(u8"shaders/lighting.frag.spv")};
    cpt::render_technique_info diffuse_info{};
    diffuse_info.stages.emplace_back(tph::pipeline_shader_stage{diffuse_vertex_shader});
    diffuse_info.stages.emplace_back(tph::pipeline_shader_stage{diffuse_fragment_shader});
    diffuse_info.stages_bindings.emplace_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, normal_map_binding, tph::descriptor_type::image_sampler});
    diffuse_info.stages_bindings.emplace_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, height_map_binding, tph::descriptor_type::image_sampler});
    diffuse_info.stages_bindings.emplace_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, specular_map_binding, tph::descriptor_type::image_sampler});
    diffuse_info.stages_bindings.emplace_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, emission_map_binding, tph::descriptor_type::image_sampler});
    diffuse_info.stages_bindings.emplace_back(tph::descriptor_set_layout_binding{tph::shader_stage::fragment, directional_light_binding, tph::descriptor_type::uniform_buffer});

    cpt::render_texture_ptr m_diffuse_map = cpt::make_render_texture(1920, 540, tph::sampling_options{});
    cpt::view_ptr m_diffuse_map_view = cpt::make_view(m_diffuse_map, diffuse_info);
    m_diffuse_map_view->fit_to(m_diffuse_map);
    m_diffuse_map_view->add_uniform_binding(directional_light_binding, m_lights_buffer);
}

void map::init_entities()
{
    const auto camera_entity{m_world.create()};
    m_entities.emplace(camera_entity_name, camera_entity);

    m_world.assign<cpt::components::node>(camera_entity);
    m_world.assign<cpt::components::camera>(camera_entity);
    m_world.assign<cpt::components::listener>(camera_entity);

    const auto player_entity{m_world.create()};
    m_entities.emplace(player_entity_name, player_entity);

    m_world.assign<cpt::components::node>(player_entity, glm::vec3{}, glm::vec3{8.0f, 8.0f, 0.0f});
    m_world.assign<cpt::components::physical_body>(player_entity, cpt::make_physical_body(m_physical_world, cpt::physical_body_type::dynamic, 1.0f, std::numeric_limits<float>::infinity()));
    m_world.assign<cpt::components::drawable>(player_entity, cpt::make_sprite(16, 16));
}

}
