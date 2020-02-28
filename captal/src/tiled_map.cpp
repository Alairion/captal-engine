#include "tiled_map.hpp"

#include <iostream>
#include <fstream>
#include <charconv>
#include <cassert>

#include "external/pugixml.hpp"

namespace cpt
{

namespace tiled
{

using namespace std::string_view_literals;

cpt::color parse_color(std::string_view attribute)
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

    std::cout << "Invalid color in tmx map." << std::endl;
    return colors::black;
}

void parse_tileset(const pugi::xml_node& node, tileset& output, const external_load_callback_type& load_callback)
{
    output.tile_width = node.attribute("tilewidth").as_uint();
    output.tile_height = node.attribute("tileheight").as_uint();
    output.width = node.attribute("columns").as_uint();
    output.height = node.attribute("tilecount").as_uint() / output.width;
    output.spacing = node.attribute("spacing").as_int();
    output.margin = node.attribute("margin").as_int();
}

void parse_map_tileset(const pugi::xml_node& node, map& output, const external_load_callback_type& load_callback)
{
    tileset item{};
    item.first_gid = node.attribute("firstgid").as_uint();

    if(auto&& attribute{node.attribute("source")}; !std::empty(attribute))
    {
        std::string data{load_callback(std::filesystem::u8path(attribute.as_string()))};

        pugi::xml_document document{};
        if(auto result{document.load_buffer_inplace(std::data(data), std::size(data))}; !result)
            throw std::runtime_error{"Can not parse TMX file: " + std::string{result.description()}};

       parse_tileset(document.root(), item, load_callback);
    }
    else
    {
       parse_tileset(node, item, load_callback);
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
                parse_tileset(child, output, load_callback);
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
