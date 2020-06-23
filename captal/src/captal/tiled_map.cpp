#include "tiled_map.hpp"

#include <iostream>
#include <fstream>
#include <charconv>
#include <numbers>
#include <cassert>

#include "external/pugixml.hpp"

#include <captal_foundation/encoding.hpp>

#include "zlib.hpp"

namespace cpt
{

namespace tiled
{

using namespace std::string_view_literals;

using tmx_data_t = std::vector<std::uint32_t>;

static constexpr std::array<std::uint8_t, 128> base64_table
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x3f,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
    0x3c, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x17, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
    0x31, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00
};

static constexpr std::uint32_t from_base64(char value) noexcept
{
    return base64_table[static_cast<std::uint8_t>(value)];
}

static std::vector<std::uint8_t> parse_base64(std::string_view data)
{
    assert(std::size(data) % 4 == 0 && "Bad base64 string");

    std::vector<std::uint8_t> output{};
    output.reserve((std::size(data) / 4) * 3);

    for(std::size_t i{}; i < std::size(data); i += 4)
    {
        std::uint32_t buffer{};
        buffer |= from_base64(data[i]) << 18;
        buffer |= from_base64(data[i + 1]) << 12;
        buffer |= from_base64(data[i + 2]) << 6;
        buffer |= from_base64(data[i + 3]);

        output.emplace_back(static_cast<std::uint8_t>(buffer >> 16));
        output.emplace_back(static_cast<std::uint8_t>(buffer >> 8));
        output.emplace_back(static_cast<std::uint8_t>(buffer));
    }

    return output;
}

static std::vector<std::uint8_t> uncompress(const std::vector<std::uint8_t>& data, std::size_t output_size)
{
    std::vector<std::uint8_t> output{};
    output.resize(output_size);

    const auto [it, success] = decompress<zlib_inflate>(std::begin(data), std::end(data), std::begin(output), std::end(output));

    if(!success || it != std::end(output))
    {
        throw std::runtime_error{"Can not decompress tiled map layer data."};
    }

    return output;
}

static tmx_data_t parse_data(const pugi::xml_node& node, std::uint32_t width, std::uint32_t height)
{
    const std::string_view encoding{node.attribute("encoding").as_string()};
    const std::string_view compression{node.attribute("compression").as_string()};
    std::string data{node.child_value()};

    if(encoding == "csv")
    {
        data.erase(std::remove_if(std::begin(data), std::end(data), [](char c){return !std::isdigit(c) && c != ',';}), std::end(data));
        data += ',';

        std::vector<std::uint32_t> output{};
        output.reserve(width * height);

        std::uint32_t value{};
        const char* const end{std::data(data) + std::size(data)};

        for(const char* it{std::data(data)}; it != end; ++it)
        {
            if(const auto [ptr, result] = std::from_chars(it, end, value); result == std::errc{})
            {
                output.emplace_back(value);
                it = ptr;
            }
            else
            {
                throw std::runtime_error{"Invalid data field in tmx file."};
            }
        }

        return output;
    }
    else if(encoding == "base64")
    {
        data.erase(std::remove_if(std::begin(data), std::end(data), [](char c){return !std::isalnum(c) && c != '+' && c != '/' && c != '=';}), std::end(data));

        std::vector<std::uint8_t> raw_data{parse_base64(data)};

        if(compression == "zlib" || compression == "gzip")
        {
            raw_data = uncompress(raw_data, width * height * sizeof(std::uint32_t));
        }

        std::vector<std::uint32_t> output{};
        output.reserve(std::size(raw_data) / 4);

        for(std::size_t i{}; i < std::size(raw_data); i += 4)
        {
            std::uint32_t value{};
            value |= raw_data[i + 0] << 0;
            value |= raw_data[i + 1] << 8;
            value |= raw_data[i + 2] << 16;
            value |= raw_data[i + 3] << 24;

            output.emplace_back(value);
        }

        return output;
    }

    return tmx_data_t{};
}

static cpt::color parse_color(std::string_view attribute)
{
    if(std::size(attribute) == 9)
    {
        std::uint32_t argb{};
        if(auto [ptr, result] = std::from_chars(std::data(attribute) + 1, std::data(attribute) + std::size(attribute), argb, 16); result == std::errc{})
        {
            return cpt::color{argb};
        }
    }
    else if(std::size(attribute) == 7)
    {
        std::uint32_t rgb{};
        if(auto [ptr, result] = std::from_chars(std::data(attribute) + 1, std::data(attribute) + std::size(attribute), rgb, 16); result == std::errc{})
        {
            return cpt::color{0xFF000000 | rgb};
        }
    }

    throw std::runtime_error{"Invalid color in tmx map."};
}

static image parse_image(const pugi::xml_node& node, const std::filesystem::path& root, const external_load_callback_type& load_callback)
{
    image output{};
    output.source = std::filesystem::u8path(load_callback(root / node.attribute("source").as_string(), external_resource_type::image));
    output.width = node.attribute("width").as_uint();
    output.width = node.attribute("height").as_uint();

    return output;
}

static properties_set parse_properties(const pugi::xml_node& node, const std::filesystem::path& root, const external_load_callback_type& load_callback)
{
    properties_set output{};

    for(auto&& child : node)
    {
        property& property{output[child.attribute("name").as_string()]};

        const std::string_view type{child.attribute("type").as_string("string")};
        std::string_view value{child.attribute("value").as_string()};

        if(std::empty(value)) //future-proof
        {
            value = child.child_value();
        }

        if(type == "string")
        {
            property = convert<narrow, utf8>(value);
        }
        else if(type == "file")
        {
            property = std::filesystem::path(convert<narrow, utf8>(load_callback(root / convert<narrow, utf8>(value), external_resource_type::file)));
        }
        else if(type == "int")
        {
            std::int32_t int_value{};
            if(auto [ptr, result] = std::from_chars(std::data(value), std::data(value) + std::size(value), int_value); result != std::errc{})
                throw std::runtime_error{"Can not parse integer property in txm file."};

            property = int_value;
        }
        else if(type == "float")
        {
            property = std::stof(std::string{value});
        }
        else if(type == "bool")
        {
            property = (value == "true");
        }
        else if(type == "color")
        {
            property = parse_color(value);
        }
    }

    return output;
}

static std::vector<tile::animation> parse_animations(const pugi::xml_node& node)
{
    std::vector<tile::animation> output{};

    for(auto&& child : node)
    {
        if(child.name() == "frame"sv) //future-proof
        {
            tile::animation animation{};
            animation.lid = child.attribute("tileid").as_uint();
            animation.duration = static_cast<float>(child.attribute("duration").as_uint()) / 1000.0f;

            output.emplace_back(animation);
        }
    }

    return output;
}

static object parse_object(const pugi::xml_node& node, const std::filesystem::path& root, const external_load_callback_type& load_callback)
{
    object output{};
    output.id = node.attribute("id").as_uint();
    output.name = node.attribute("name").as_string();
    output.type = node.attribute("type").as_string();
    output.visible = node.attribute("visible").as_uint(1) == 1;

    for(auto&& child : node)
    {
        if(child.name() == "point"sv)
        {
            object::point point{};
            point.position.x = node.attribute("x").as_float();
            point.position.y = node.attribute("y").as_float();

            output.content = point;
        }
        else if(child.name() == "text"sv)
        {
            object::text text{};
            text.text = child.child_value();
            text.font_family = child.attribute("fontfamily").as_string();
            text.pixel_size = child.attribute("pixelsize").as_uint();
            text.position.x = node.attribute("x").as_float();
            text.position.y = node.attribute("y").as_float();
            text.width = node.attribute("width").as_float();
            text.height = node.attribute("height").as_float();
            text.angle = node.attribute("rotation").as_float() * (std::numbers::pi_v<float> * 180.0f);
            text.color = parse_color(child.attribute("color").as_string("#000000"));

            if(child.attribute("bold").as_uint() != 0)
            {
                text.style |= font_style::bold;
            }

            if(child.attribute("underline").as_uint() != 0)
            {
                text.style |= font_style::underlined;
            }

            if(child.attribute("strikeout").as_uint() != 0)
            {
                text.style |= font_style::strikethrough;
            }

            text.italic = child.attribute("italic").as_uint() != 0;

            if(child.attribute("kerning").as_uint(1) != 0)
            {
                text.drawer_options |= text_drawer_options::kerning;
            }

            output.content = std::move(text);
        }
        else if(child.name() == "properties"sv)
        {
            output.properties = parse_properties(child, root, load_callback);
        }
    }

    if(const auto attribute{node.attribute("gid")}; !std::empty(attribute))
    {
        object::tile tile{};
        tile.gid = attribute.as_uint();
        tile.position.x = node.attribute("x").as_float();
        tile.position.y = node.attribute("y").as_float();
        tile.width = node.attribute("width").as_float();
        tile.height = node.attribute("height").as_float();
        tile.angle = node.attribute("rotation").as_float() * (std::numbers::pi_v<float> * 180.0f);

        output.content = tile;
    }

    if(std::holds_alternative<std::monostate>(output.content))
    {
        object::square square{};
        square.position.x = node.attribute("x").as_float();
        square.position.y = node.attribute("y").as_float();
        square.width = node.attribute("width").as_float();
        square.height = node.attribute("height").as_float();
        square.angle = node.attribute("rotation").as_float() * (std::numbers::pi_v<float> * 180.0f);

        output.content = square;
    }

    return output;
}

static std::vector<object> parse_hitboxes(const pugi::xml_node& node, const std::filesystem::path& root, const external_load_callback_type& load_callback)
{
    std::vector<object> output{};

    for(auto&& child : node)
    {
        if(child.name() == "object"sv)
        {
            output.emplace_back(parse_object(child, root, load_callback));
        }
    }

    return output;
}

static tile parse_tile(const pugi::xml_node& node, const std::filesystem::path& root, const external_load_callback_type& load_callback)
{
    tile output{};
    output.type = node.attribute("type").as_string();

    for(auto&& child : node)
    {
        if(child.name() == "animation"sv)
        {
            output.animations = parse_animations(child);
        }
        else if(child.name() == "image"sv)
        {
            output.image = parse_image(child, root.parent_path(), load_callback);
        }
        else if(child.name() == "objectgroup"sv)
        {
            output.hitboxes = parse_hitboxes(child, root, load_callback);
        }
        else if(child.name() == "properties"sv)
        {
            output.properties = parse_properties(child, root, load_callback);
        }
    }

    return output;
}

static void parse_tileset(const pugi::xml_node& node, tileset& output, const std::filesystem::path& root, const external_load_callback_type& load_callback)
{
    output.tile_width = node.attribute("tilewidth").as_uint();
    output.tile_height = node.attribute("tileheight").as_uint();
    output.width = node.attribute("columns").as_uint();
    output.height = node.attribute("tilecount").as_uint() / output.width;
    output.spacing = node.attribute("spacing").as_int();
    output.margin = node.attribute("margin").as_int();
    output.tiles.resize(output.width * output.height);

    for(auto&& child : node)
    {
        if(child.name() == "tileoffset"sv)
        {
            output.offset.x = static_cast<float>(child.attribute("x").as_int());
            output.offset.y = static_cast<float>(child.attribute("y").as_int());
        }
        else if(child.name() == "image"sv)
        {
            output.image = parse_image(child, root.parent_path(), load_callback);
        }
        else if(child.name() == "tile"sv)
        {
            output.tiles[child.attribute("id").as_uint()] = parse_tile(child, root, load_callback);
        }
        else if(child.name() == "properties"sv)
        {
            output.properties = parse_properties(child, root, load_callback);
        }
    }
}

static tileset parse_map_tileset(const pugi::xml_node& node, const external_load_callback_type& load_callback)
{
    tileset output{};
    output.first_gid = node.attribute("firstgid").as_uint();

    if(const auto attribute{node.attribute("source")}; !std::empty(attribute))
    {
        const auto path{std::filesystem::u8path(attribute.as_string())};
        const std::string data{load_callback(path, external_resource_type::tileset)};

        pugi::xml_document document{};
        if(auto result{document.load_string(std::data(data))}; !result)
            throw std::runtime_error{"Can not parse TMX file: " + std::string{result.description()}};

        parse_tileset(document.child("tileset"), output, path, load_callback);
    }
    else
    {
        parse_tileset(node, output, "", load_callback);
    }

    return output;
}

static layer parse_layer(const pugi::xml_node& node, const std::filesystem::path& root, const external_load_callback_type& load_callback)
{
    layer output{};
    output.name = node.attribute("name").as_string();
    output.opacity = node.attribute("opacity").as_float(1.0f);
    output.visible = node.attribute("visible").as_uint(1) == 1;
    output.position.x = node.attribute("offsetx").as_float();
    output.position.y = node.attribute("offsety").as_float();

    for(auto&& child : node)
    {
        if(child.name() == "data"sv)
        {
            const std::uint32_t width{node.attribute("width").as_uint()};
            const std::uint32_t height{node.attribute("height").as_uint()};

            layer::tiles tiles{};
            tiles.gid = parse_data(child, width, height);

            output.content = std::move(tiles);
        }
        else if(child.name() == "properties"sv)
        {
            output.properties = parse_properties(child, root, load_callback);
        }
    }

    return output;
}

static layer parse_object_group(const pugi::xml_node& node, const std::filesystem::path& root, const external_load_callback_type& load_callback)
{
    layer output{};
    output.name = node.attribute("name").as_string();
    output.opacity = node.attribute("opacity").as_float(1.0f);
    output.visible = node.attribute("visible").as_uint(1) == 1;
    output.position.x = node.attribute("offsetx").as_float();
    output.position.y = node.attribute("offsety").as_float();

    layer::objects objects{};
    objects.draw_order = node.attribute("draworder").as_string("topdown") == "index"sv ? objects_layer_draw_order::index : objects_layer_draw_order::topdown;

    for(auto&& child : node)
    {
        if(child.name() == "object"sv)
        {
            objects.objects.emplace_back(parse_object(child, root, load_callback));
        }
        else if(child.name() == "properties"sv)
        {
            output.properties = parse_properties(child, root, load_callback);
        }
    }

    output.content = std::move(objects);

    return output;
}

static layer parse_image_layer(const pugi::xml_node& node, const std::filesystem::path& root, const external_load_callback_type& load_callback)
{
    layer output{};
    output.name = node.attribute("name").as_string();
    output.opacity = node.attribute("opacity").as_float(1.0f);
    output.visible = node.attribute("visible").as_uint(1) == 1;
    output.position.x = node.attribute("offsetx").as_float();
    output.position.y = node.attribute("offsety").as_float();

    for(auto&& child : node)
    {
        if(child.name() == "group"sv)
        {
            output.content = parse_image(node.child("image"), "", load_callback);
        }
        else if(child.name() == "properties"sv)
        {
            output.properties = parse_properties(child, root, load_callback);
        }
    }

    return output;
}

static layer parse_group_layer(const pugi::xml_node& node, const std::filesystem::path& root, const external_load_callback_type& load_callback)
{
    layer output{};
    output.name = node.attribute("name").as_string();
    output.opacity = node.attribute("opacity").as_float(1.0f);
    output.visible = node.attribute("visible").as_uint(1) == 1;
    output.position.x = node.attribute("offsetx").as_float();
    output.position.y = node.attribute("offsety").as_float();

    layer::group group{};
    for(auto&& child : node)
    {
        if(child.name() == "layer"sv)
        {
            group.layers.emplace_back(parse_layer(child, root, load_callback));
        }
        else if(child.name() == "objectgroup"sv)
        {
            group.layers.emplace_back(parse_object_group(child, root, load_callback));
        }
        else if(child.name() == "imagelayer"sv)
        {
            group.layers.emplace_back(parse_image_layer(child, root, load_callback));
        }
        else if(child.name() == "group"sv)
        {
            group.layers.emplace_back(parse_group_layer(child, root, load_callback));
        }
        else if(child.name() == "properties"sv)
        {
            output.properties = parse_properties(child, root, load_callback);
        }
    }

    output.content = std::move(group);

    return output;
}

static map parse_map(const pugi::xml_node& node, const external_load_callback_type& load_callback)
{
    map output{};

    output.width = node.attribute("width").as_uint();
    output.height = node.attribute("height").as_uint();
    output.tile_width = node.attribute("tilewidth").as_uint();
    output.tile_height = node.attribute("tileheight").as_uint();

    if(const auto attribute{node.attribute("backgroundcolor")}; !std::empty(attribute))
    {
        output.background_color = parse_color(attribute.as_string());
    }

    for(auto&& child : node)
    {
        if(child.name() == "tileset"sv)
        {
            output.tilesets.emplace_back(parse_map_tileset(child, load_callback));
        }
        else if(child.name() == "layer"sv)
        {
            output.layers.emplace_back(parse_layer(child, "", load_callback));
        }
        else if(child.name() == "objectgroup"sv)
        {
            output.layers.emplace_back(parse_object_group(child, "", load_callback));
        }
        else if(child.name() == "imagelayer"sv)
        {
            output.layers.emplace_back(parse_image_layer(child, "", load_callback));
        }
        else if(child.name() == "group"sv)
        {
            output.layers.emplace_back(parse_group_layer(child, "", load_callback));
        }
        else if(child.name() == "properties"sv)
        {
            output.properties = parse_properties(child, "", load_callback);
        }
    }

    return output;
}

map load_map(const std::filesystem::path& path)
{
    assert(!std::empty(path) && "Invalid path.");

    const auto load_callback = [&path](const std::filesystem::path& other_path, external_resource_type resource_type) -> std::string
    {
        if(resource_type == external_resource_type::image || resource_type == external_resource_type::file)
        {
            const std::filesystem::path output{path.parent_path() / other_path};
            return convert<utf8, narrow>(output.string());
        }
        else
        {
            std::ifstream ifs{path.parent_path() / other_path, std::ios_base::binary};
            if(!ifs)
                throw std::runtime_error{"Can not open file \"" + path.string() + "\"."};

            return std::string{std::istreambuf_iterator<char>{ifs}, std::istreambuf_iterator<char>{}};
        }
    };

    return load_map(path, load_callback);
}

map load_map(const std::filesystem::path& path, const external_load_callback_type& load_callback)
{
    assert(!std::empty(path) && "Invalid path.");

    std::ifstream ifs{path, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not open file \"" + path.string() + "\"."};

    return load_map(ifs, load_callback);
}

map load_map(std::span<const std::uint8_t> tmx_file, const external_load_callback_type& load_callback)
{
    assert(!std::empty(tmx_file) && "Invalid tmx file.");

    pugi::xml_document document{};
    if(auto result{document.load_buffer(std::data(tmx_file), std::size(tmx_file))}; !result)
        throw std::runtime_error{"Can not parse TMX file: " + std::string{result.description()}};

    return parse_map(document.child("map"), load_callback);
}

map load_map(std::istream& tmx_file, const external_load_callback_type& load_callback)
{
    assert(tmx_file && "Invalid stream");

    const std::vector<std::uint8_t> data{std::istreambuf_iterator<char>{tmx_file}, std::istreambuf_iterator<char>{}};
    assert(!std::empty(data) && "Invalid tmx file.");

    return load_map(data, load_callback);
}

}

}
