#include "tiled_map.hpp"

#include <iostream>
#include <fstream>
#include <charconv>
#include <cassert>

#include <zlib/zlib.h>

#include "external/pugixml.hpp"

namespace cpt
{

namespace tiled
{

using namespace std::string_view_literals;

static cpt::color parse_color(std::string_view attribute)
{
    if(std::size(attribute) == 9)
    {
        std::uint32_t rgba{};
        if(auto [ptr, result] = std::from_chars(std::begin(attribute) + 1, std::end(attribute), rgba, 16); result == std::errc{})
            return cpt::color{rgba};
    }
    else if(std::size(attribute) == 7)
    {
        std::uint32_t rgba{};
        if(auto [ptr, result] = std::from_chars(std::begin(attribute) + 1, std::end(attribute), rgba, 16); result == std::errc{})
            return cpt::color{0xFF000000 | rgba};
    }

    throw std::runtime_error{"Invalid color in tmx map."};
}

using tmx_data_t = std::variant<std::vector<std::uint8_t>, std::vector<std::uint32_t>>;

static std::uint32_t from_base64(char value) noexcept
{
    if(value >= 'A' && value <= 'Z')
    {
        return static_cast<std::uint32_t>(value - 'A');
    }
    else if(value >= 'a' && value <= 'z')
    {
        return static_cast<std::uint32_t>(value - 'a' + 26);
    }
    else if(value >= '0' && value <= '9')
    {
        return static_cast<std::uint32_t>(value - '0' + 52);
    }
    else if(value == '+')
    {
        return 62;
    }
    else if (value == '/')
    {
        return 63;
    }

    return 0;
}

static std::vector<std::uint8_t> parse_base64(std::string_view data)
{
    std::vector<std::uint8_t> output{};
    output.reserve((std::size(data) / 4) * 3);

    for(std::size_t i{}; i < std::size(data); i += 4)
    {
        std::uint32_t buffer{};
        buffer |= from_base64(data[i]) << 18;
        buffer |= from_base64(data[i + 1]) << 12;
        buffer |= from_base64(data[i + 2]) << 6;
        buffer |= from_base64(data[i + 3]);

        output.push_back(static_cast<std::uint8_t>(buffer >> 16));
        output.push_back(static_cast<std::uint8_t>(buffer >> 8));
        output.push_back(static_cast<std::uint8_t>(buffer));
    }

    return output;
}

static std::vector<std::uint8_t> uncompress(const std::vector<std::uint8_t>& data)
{
    struct stream_deleter
    {
        ~stream_deleter() {inflateEnd(&stream);}
        z_stream& stream;
    };

    z_stream stream{};
    if(inflateInit(&stream) != Z_OK)
        throw std::runtime_error{"Can not init inflate stream."};
    stream_deleter deleter{stream};

    stream.next_in = std::data(data);
    stream.avail_in = static_cast<std::uint32_t>(std::size(data));

    std::array<std::uint8_t, 1024 * 16> buffer{};
    std::vector<std::uint8_t> output{};

    std::int32_t status{Z_OK};
    while(status != Z_STREAM_END)
    {
        stream.next_out = std::data(buffer);
        stream.avail_out = static_cast<std::uint32_t>(std::size(buffer));

        status = inflate(&stream, Z_NO_FLUSH);
        if(status != Z_OK && status != Z_STREAM_END)
            throw std::runtime_error{"Error during data decompression in tmx loader."};

        const std::size_t buffered{std::size(buffer) - stream.avail_out};
        output.insert(std::end(output), std::begin(buffer), std::begin(buffer) + buffered);
    }

    return output;
}

static tmx_data_t parse_data(const pugi::xml_node& node)
{
    const std::string_view encoding{node.attribute("encoding").as_string()};
    const std::string_view compression{node.attribute("compression").as_string()};
    std::string data{node.child_value()};

    if(encoding == "cvt")
    {
        data.erase(std::remove_if(std::begin(data), std::end(data), [](char c){return !std::isdigit(c) || c != ',';}), std::end(data));

        std::uint32_t value{};
        std::vector<std::uint32_t> output{};
        output.reserve(static_cast<std::size_t>(std::count(std::begin(data), std::end(data), ',')) + 1);

        const char* const end{std::data(data) + std::size(data)};
        for(const char* it{std::data(data)}; it != end; ++it)
        {
            if(auto [ptr, result] = std::from_chars(it, end, value); result == std::errc{})
            {
                output.push_back(value);
                it = ptr;
            }
            else
            {
                throw std::runtime_error{"Invalid data field in tmx file."};
            }
        }

        return tmx_data_t{output};
    }
    else if(encoding == "base64")
    {
        std::vector<std::uint8_t> raw_data{parse_base64(data)};

        if(compression == "zlib" || compression == "gzip")
            return tmx_data_t{uncompress(raw_data)};

        return tmx_data_t{std::move(raw_data)};
    }

    return tmx_data_t{std::vector<std::uint8_t>{}};
}

static tph::image parse_image(const pugi::xml_node& node, const std::filesystem::path& root, const external_load_callback_type& load_callback)
{
    const std::string file_data{load_callback(root / node.attribute("source").as_string())};

    return tph::image{cpt::engine::instance().renderer(), file_data, tph::load_from_memory, tph::image_usage::transfer_source};
}

static void parse_tileset(const pugi::xml_node& node, tileset& output, const std::filesystem::path& root, const external_load_callback_type& load_callback)
{
    output.tile_width = node.attribute("tilewidth").as_uint();
    output.tile_height = node.attribute("tileheight").as_uint();
    output.width = node.attribute("columns").as_uint();
    output.height = node.attribute("tilecount").as_uint() / output.width;
    output.spacing = node.attribute("spacing").as_int();
    output.margin = node.attribute("margin").as_int();

    for(auto&& child : node)
    {
        if(child.name() == "tileoffset"sv)
        {
            output.offset.x = static_cast<float>(child.attribute("x").as_int());
            output.offset.y = static_cast<float>(child.attribute("y").as_int());
        }
        else if(child.name() == "image"sv)
        {
            output.image = parse_image(child, root, load_callback);
        }
    }
}

static void parse_map_tileset(const pugi::xml_node& node, map& output, const external_load_callback_type& load_callback)
{
    tileset item{};
    item.first_gid = node.attribute("firstgid").as_uint();

    if(auto&& attribute{node.attribute("source")}; !std::empty(attribute))
    {
        const auto path{std::filesystem::u8path(attribute.as_string())};
        std::string data{load_callback(path)};

        pugi::xml_document document{};
        if(auto result{document.load_buffer_inplace(std::data(data), std::size(data))}; !result)
            throw std::runtime_error{"Can not parse TMX file: " + std::string{result.description()}};

       parse_tileset(document.root(), item, path, load_callback);
    }
    else
    {
       parse_tileset(node, item, std::filesystem::u8path(u8"."), load_callback);
    }

    output.tilesets.push_back(std::move(item));
}

map load_map(const std::filesystem::path& path)
{
    std::ifstream ifs{path, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not open file \"" + path.u8string() + "\"."};

    std::string data{std::istreambuf_iterator<char>{ifs}, std::istreambuf_iterator<char>{}};

    pugi::xml_document document{};
    if(auto result{document.load_buffer_inplace(std::data(data), std::size(data))}; !result)
        throw std::runtime_error{"Can not parse TMX file: " + std::string{result.description()}};

    map output{};

    const auto load_callback = [&path](const std::filesystem::path& other_path) -> std::string
    {
        std::ifstream ifs{path / other_path, std::ios_base::binary};
        if(!ifs)
            throw std::runtime_error{"Can not open file \"" + path.u8string() + "\"."};

        return std::string{std::istreambuf_iterator<char>{ifs}, std::istreambuf_iterator<char>{}};
    };

    for(auto&& map : document)
    {
        output.width = map.attribute("width").as_uint();
        output.height = map.attribute("height").as_uint();
        output.tile_width = map.attribute("tilewidth").as_uint();
        output.tile_height = map.attribute("tileheight").as_uint();
        if(const auto attribute{map.attribute("backgroundcolor")}; !std::empty(attribute))
            output.background_color = parse_color(attribute.as_string());

        for(auto&& child : map)
        {
            if(child.name() == "tileset"sv)
                parse_map_tileset(child, output, load_callback);
        }
    }

    return output;
}

}

/*
tiled_map::tiled_map(std::string_view file, cpt::load_from_file_t)
{
    std::ifstream ifs{std::string{file}, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not open file \"" + std::string{file} + "\"."};

    const std::string data{std::istreambuf_iterator<char>{ifs}, std::istreambuf_iterator<char>{}};
    m_map = tmx_load_buffer(std::data(data), static_cast<int>(std::size(data)));
    if(!m_map)
        throw std::runtime_error{"Invalid tmx map."};
}

tiled_map::tiled_map(std::string_view data, cpt::load_from_memory_t)
{
    m_map = tmx_load_buffer(std::data(data), static_cast<int>(std::size(data)));
    if(!m_map)
        throw std::runtime_error{"Invalid tmx map."};
}

tiled_map::~tiled_map()
{
    if(m_map)
        tmx_map_free(reinterpret_cast<tmx_map*>(m_map));
}

std::uint32_t tiled_map::width() const noexcept
{
    assert(m_map);
    return reinterpret_cast<const tmx_map*>(m_map)->width;
}

std::uint32_t tiled_map::height() const noexcept
{
    assert(m_map);
    return reinterpret_cast<const tmx_map*>(m_map)->height;
}

std::uint32_t tiled_map::tile_width() const noexcept
{
    assert(m_map);
    return reinterpret_cast<const tmx_map*>(m_map)->tile_width;
}

std::uint32_t tiled_map::tile_height() const noexcept
{
    assert(m_map);
    return reinterpret_cast<const tmx_map*>(m_map)->tile_height;
}

*/
}
