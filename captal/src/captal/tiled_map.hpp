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

#include <captal_foundation/math.hpp>

#include "color.hpp"
#include "text.hpp"
#include "zlib.hpp"

namespace cpt
{

namespace tiled
{

using property = std::variant<std::string, std::filesystem::path, std::int32_t, float, color, bool>;
using properties_set = std::unordered_map<std::string, property>;

struct object
{
    struct point
    {
        vec2f position{};
    };

    struct square
    {
        vec2f position{};
        float width{};
        float height{};
        float angle{};
    };

    struct ellipse
    {
        vec2f position{};
        float width{};
        float height{};
    };

    struct tile
    {
        std::uint32_t gid{};
        vec2f position{};
        float width{};
        float height{};
        float angle{};
    };

    struct text
    {
        std::string string{};
        std::string font_family{};
        std::uint32_t pixel_size{};
        vec2f position{};
        float width{};
        float height{};
        float angle{};
        color color{};
        text_style style{};
        bool italic{};
        text_drawer_options drawer_options{};
    };

    using content_type = std::variant<std::monostate, point, square, ellipse, tile, text>;

    std::uint32_t id{};
    std::string name{};
    std::string type{};
    bool visible{};
    content_type content{};
    properties_set properties{};
};

struct image
{
    std::filesystem::path source{};
    std::uint32_t width{};
    std::uint32_t height{};
};

enum class objects_layer_draw_order : std::uint32_t
{
    unknown,
    topdown,
    index,
};

struct layer
{
    struct tiles
    {
        std::vector<std::uint32_t> gid{};
    };

    struct objects
    {
        objects_layer_draw_order draw_order{};
        std::vector<object> childrens{};
    };

    struct group
    {
        std::vector<layer> layers{};
    };

    using content_type = std::variant<std::monostate, tiles, objects, image, group>;

    std::string name{};
    vec2f position{};
    float opacity{};
    bool visible{};
    content_type content{};
    properties_set properties{};
};

struct tile
{
    struct animation
    {
        std::uint32_t lid{};
        float duration{};
    };

    std::string type{};
    image image{};
    std::vector<object> hitboxes{};
    std::vector<animation> animations{};
    properties_set properties{};
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
    vec2f offset{};
    image image{};
    std::vector<tile> tiles{};
    properties_set properties{};
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
    properties_set properties{};
};

enum class external_resource_type : std::uint32_t
{
    tileset = 1,
    object_template = 2,
    image = 3,
    file = 4
};

using external_load_callback_type = std::function<std::string(const std::filesystem::path& path, external_resource_type resource_type)>;

CAPTAL_API map load_map(const std::filesystem::path& path);
CAPTAL_API map load_map(const std::filesystem::path& path, const external_load_callback_type& load_callback);
CAPTAL_API map load_map(std::span<const std::uint8_t> tmx_file, const external_load_callback_type& load_callback);
CAPTAL_API map load_map(std::istream& tmx_file, const external_load_callback_type& load_callback);

}

}

#endif
