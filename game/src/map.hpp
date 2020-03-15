#ifndef MY_PROJECT_MAP_HPP_INCLUDED
#define MY_PROJECT_MAP_HPP_INCLUDED

#include "config.hpp"

#include <captal/tiled_map.hpp>
#include <captal/physics.hpp>
#include <captal/render_texture.hpp>
#include <captal/tileset.hpp>

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

private:
    cpt::tiled::map m_tiled_map{};
    cpt::framed_buffer_ptr m_directional_light_buffer{};
    entt::registry m_world{};
    std::unordered_map<std::string, entt::entity> m_entities{};
    std::vector<std::pair<std::uint32_t, cpt::tileset_ptr>> m_tilesets{};
    cpt::physical_world m_physical_world{};

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
