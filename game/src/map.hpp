#ifndef MY_PROJECT_MAP_HPP_INCLUDED
#define MY_PROJECT_MAP_HPP_INCLUDED

#include "config.hpp"

#include <captal/tiled_map.hpp>
#include <captal/physics.hpp>
#include <captal/render_texture.hpp>
#include <captal/tileset.hpp>
#include <captal/tilemap.hpp>

#include <captal/components/physical_body.hpp>

#include <entt/entity/registry.hpp>

namespace mpr
{

struct directional_light
{
    glm::vec4 direction{};
    glm::vec4 ambiant{};
    glm::vec4 diffuse{};
    glm::vec4 specular{};
};

class map
{
public:
    map(const std::filesystem::path& path);

    const cpt::render_texture_ptr& texture() const noexcept
    {
        return m_diffuse_map;
    }

    void render();
    void view(float x, float y, std::uint32_t width, std::uint32_t height);

private:
    void init_render();
    std::uint64_t parse_layers(const std::vector<cpt::tiled::layer>& layers, std::uint64_t index);
    cpt::tilemap_ptr parse_tiles(const cpt::tiled::layer::tiles& tiles, cpt::components::physical_body& body);
    void parse_object(const cpt::tiled::object& object, std::uint64_t index);
    const std::pair<std::uint32_t, cpt::tileset_ptr>& tileset_from_gid(std::uint32_t gid);
    std::size_t tileset_index(std::uint32_t gid);
    cpt::texture_ptr load_height_map(const cpt::tiled::properties_set& properties) const;
    cpt::texture_ptr load_normal_map(const cpt::tiled::properties_set& properties) const;
    cpt::texture_ptr load_specular_map(const cpt::tiled::properties_set& properties) const;

private:
    cpt::tiled::map m_tiled_map{};
    entt::registry m_world{};
    entt::entity m_camera{};
    std::unordered_map<std::string, entt::entity> m_entities{};
    std::vector<std::pair<std::uint32_t, cpt::tileset_ptr>> m_tilesets{};
    cpt::physical_world m_physical_world{};

    cpt::framed_buffer_ptr m_directional_light_buffer{};
    cpt::render_texture_ptr m_height_map{};
    cpt::view_ptr m_height_map_view{};
    entt::registry m_shadow_world{};
    cpt::render_texture_ptr m_shadow_map{};
    cpt::view_ptr m_shadow_map_view{};
    cpt::render_texture_ptr m_diffuse_map{};
    cpt::view_ptr m_diffuse_map_view{};
};

}

#endif
