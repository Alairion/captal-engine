#include "tileset.hpp"

namespace cpt
{

tileset::tileset(tph::texture other, std::uint32_t tile_width, std::uint32_t tile_height)
:texture{std::move(other)}
,m_tile_width{tile_width}
,m_tile_height{tile_height}
{

}

tileset::tileset(const std::filesystem::path& file, std::uint32_t tile_width, std::uint32_t tile_height, const tph::sampling_options& sampling)
:texture{file, sampling}
,m_tile_width{tile_width}
,m_tile_height{tile_height}
{

}

tileset::tileset(std::string_view data, std::uint32_t tile_width, std::uint32_t tile_height, const tph::sampling_options& sampling)
:texture{data, sampling}
,m_tile_width{tile_width}
,m_tile_height{tile_height}
{

}

tileset::tileset(std::istream& stream, std::uint32_t tile_width, std::uint32_t tile_height, const tph::sampling_options& sampling)
:texture{stream, sampling}
,m_tile_width{tile_width}
,m_tile_height{tile_height}
{

}

tileset::tileset(std::uint32_t width, std::uint32_t height, const std::uint8_t* rgba, std::uint32_t tile_width, std::uint32_t tile_height, const tph::sampling_options& sampling)
:texture{width, height, rgba, sampling}
,m_tile_width{tile_width}
,m_tile_height{tile_height}
{

}


tileset::tileset(tph::image image, std::uint32_t tile_width, std::uint32_t tile_height, const tph::sampling_options& sampling)
:texture{std::move(image), sampling}
,m_tile_width{tile_width}
,m_tile_height{tile_height}
{

}

}
