#ifndef CAPTAL_TILED_MAP_HPP_INCLUDED
#define CAPTAL_TILED_MAP_HPP_INCLUDED

#include "config.hpp"

#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <optional>
#include <cmath>
#include <filesystem>

#include <glm/vec2.hpp>

#include "color.hpp"
#include "text.hpp"

namespace cpt
{

namespace tiled
{

using property = std::variant<std::string, std::filesystem::path, std::int32_t, float, cpt::color, bool>;

struct object
{
    struct square
    {
        glm::vec2 position{};
        float width{};
        float height{};
        float angle{};
    };

    struct tile
    {
        std::uint32_t gid{};
        float width{};
        float height{};
        float angle{};
    };

    struct text
    {
        std::string font_family{};
        std::uint32_t pixel_size{};
        float width{};
        float height{};
        float angle{};
        cpt::color color{};
        font_style style{};
        text_drawer_options drawer_options{};
        std::string text{};
    };

    struct point
    {
        glm::vec2 position{};
    };

    using content_type = std::variant<std::monostate, square, tile, text, point>;

    std::uint32_t id{};
    std::string name{};
    std::string type{};
    bool visible{};
    content_type content{};
    std::unordered_map<std::string, property> properties{};
};

enum class objects_layer_draw_order : std::uint32_t
{
    unknown,
    index,
    topdown
};

struct layer
{
    struct tiles
    {
        std::vector<std::uint32_t> gid{};
    };

    struct objects
    {
        color color{};
        objects_layer_draw_order draw_order{};
        std::vector<object> objects{};
    };

    struct image
    {
        std::filesystem::path path{};
        std::uint32_t width{};
        std::uint32_t height{};
    };

    struct group
    {
        std::vector<layer> layers{};
    };

    using content_type = std::variant<std::monostate, tiles, objects, image, group>;

    std::string name{};
    glm::vec2 offset{};
    float opacity{};
    bool visible{};
    content_type content{};
    std::unordered_map<std::string, property> properties{};
};

struct tile
{
    struct animation
    {
        std::uint32_t lid{};
        float duration{};
    };

    std::uint32_t id{};
    std::string type{};
    std::optional<tph::image> image{};
    std::optional<object> hitbox{};
    std::vector<animation> animations{};
    std::unordered_map<std::string, property> properties{};
};

struct tileset
{
    std::string name{};
    std::uint32_t first_gid{};
    std::uint32_t tile_width{};
    std::uint32_t tile_height{};
    std::uint32_t width{};
    std::uint32_t height{};
    std::int32_t spacing{};
    std::int32_t margin{};
    glm::vec2 offset{};
    std::optional<tph::image> image{};
    std::vector<tile> tiles{};
    std::unordered_map<std::string, property> properties{};
};

struct map
{
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint32_t tile_width{};
    std::uint32_t tile_height{};
    color background_color{};
    std::vector<tileset> tilesets{};
    std::vector<layer> layers{};
    std::vector<std::reference_wrapper<tile>> tiles{};
    std::unordered_map<std::string, property> properties{};
};

using external_load_callback_type = std::function<std::string(const std::filesystem::path& path)>;

map load_map(const std::filesystem::path& path);

}

class tiled_world
{
public:
    tiled_world(std::filesystem::path tmx_path);
    ~tiled_world() = default;

private:

};

}

#endif
