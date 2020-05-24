#ifndef MY_PROJECT_MAP_HPP_INCLUDED
#define MY_PROJECT_MAP_HPP_INCLUDED

#include "config.hpp"

#include <captal/tiled_map.hpp>
#include <captal/physics.hpp>
#include <captal/render_texture.hpp>
#include <captal/tileset.hpp>
#include <captal/tilemap.hpp>
#include <captal/texture_pool.hpp>

#include <captal/components/physical_body.hpp>

#include <entt/entity/registry.hpp>

namespace mpr
{

inline constexpr std::uint32_t chunk_size{32};
inline constexpr std::uint32_t tile_size{24};

inline constexpr const char* camera_entity_name{"camera"};
inline constexpr const char* player_entity_name{"player"};
inline constexpr const char* player_controller_entity_name{"playerctrlr"};

class map;

class chunk
{
public:
    chunk() = default;
    chunk(map& map, std::uint32_t x, std::uint32_t y);
    ~chunk() = default;
    chunk(const chunk&) = delete;
    chunk& operator=(const chunk&) = delete;
    chunk(chunk&&) noexcept = default;
    chunk& operator=(chunk&&) noexcept = default;

    std::pair<std::string, entt::entity> drain(std::string entity_name);
    void add_entity(std::pair<std::string, entt::entity> entity);

    std::uint32_t x() const noexcept
    {
        return m_x;
    }

    std::uint32_t y() const noexcept
    {
        return m_y;
    }

    glm::vec3 chunk_offset() const noexcept
    {
        return glm::vec3{m_x * chunk_size * tile_size, m_y * chunk_size * tile_size, 0};
    }

private:
    std::filesystem::path file_path();
    std::uint64_t parse_layers(const std::vector<cpt::tiled::layer>& layers, std::uint64_t index);
    cpt::tilemap_ptr parse_tiles(const cpt::tiled::layer::tiles& tiles, cpt::components::physical_body& body);
    void parse_object(const cpt::tiled::object& object, std::uint64_t index);
    const std::pair<std::uint32_t, cpt::tileset>& tileset_from_gid(std::uint32_t gid);
    std::size_t tileset_index(std::uint32_t gid);
    cpt::texture_ptr load_normal_map(const cpt::tiled::properties_set& properties) const;
    cpt::texture_ptr load_height_map(const cpt::tiled::properties_set& properties) const;
    cpt::texture_ptr load_specular_map(const cpt::tiled::properties_set& properties) const;
    cpt::texture_ptr load_emission_map(const cpt::tiled::properties_set& properties) const;

private:
    map* m_map{};
    std::uint32_t m_x{};
    std::uint32_t m_y{};
    cpt::tiled::map m_tiled_map{};
    std::unordered_map<std::string, entt::entity> m_entities{};
    std::vector<std::pair<std::uint32_t, cpt::tileset>> m_tilesets{};
};

class map
{
public:
    map();
    ~map() = default;
    map(const map&) = delete;
    map& operator=(const map&) = delete;
    map(map&&) noexcept = delete;
    map& operator=(map&&) noexcept = delete;

    void update(float time);
    void render();
    void view(float x, float y, std::uint32_t width, std::uint32_t height);

    const cpt::render_texture_ptr& texture() const noexcept
    {
        return m_diffuse_map;
    }

    entt::registry& world() noexcept
    {
        return m_world;
    }

    const entt::registry& world() const noexcept
    {
        return m_world;
    }

    const cpt::physical_world_ptr& physical_world() const noexcept
    {
        return m_physical_world;
    }

    cpt::texture_pool& texture_pool() noexcept
    {
        return m_texture_pool;
    }

    const cpt::texture_pool& texture_pool() const noexcept
    {
        return m_texture_pool;
    }

private:
    void init_render();
    void init_entities();

private:
    //Core:
    entt::registry m_world{};
    std::unordered_map<std::string, entt::entity> m_entities{};
    cpt::physical_world_ptr m_physical_world{};
    cpt::texture_pool m_texture_pool{};
    std::vector<chunk> m_chunks{};

    //Render:
    cpt::framed_buffer_ptr m_lights_buffer{};
    cpt::render_texture_ptr m_height_map{};
    cpt::view_ptr m_height_map_view{};

    cpt::render_texture_ptr m_diffuse_map{};
    cpt::view_ptr m_diffuse_map_view{};
};

}

#endif
