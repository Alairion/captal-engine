#include "tilemap.hpp"

namespace cpt
{

tilemap::tilemap(std::uint32_t width, std::uint32_t height, std::uint32_t tile_width, std::uint32_t tile_height)
:renderable{width * height * 6, width * height * 4}
,m_width{width}
,m_height{height}
,m_tile_width{tile_width}
,m_tile_height{tile_height}
{
    init();
}

tilemap::tilemap(std::uint32_t width, std::uint32_t height, tileset_ptr tileset)
:renderable{width * height * 6, width * height * 4}
,m_width{width}
,m_height{height}
,m_tile_width{tileset->tile_width()}
,m_tile_height{tileset->tile_height()}
{
    init();
    set_texture(std::move(tileset));
}

void tilemap::set_color(std::uint32_t row, std::uint32_t col, const color& color) noexcept
{
    vertex* const current{get_vertices() + (col * m_width) + row};

    current[0].color = static_cast<glm::vec4>(color);
    current[1].color = static_cast<glm::vec4>(color);
    current[2].color = static_cast<glm::vec4>(color);
    current[3].color = static_cast<glm::vec4>(color);

    update();
}

void tilemap::set_color(std::uint32_t row, std::uint32_t col, float red, float green, float blue, float alpha) noexcept
{
    set_color(row, col, glm::vec4{red, green, blue, alpha});
}

void tilemap::set_texture_coords(std::uint32_t row, std::uint32_t col, std::int32_t x1, std::int32_t y1, std::uint32_t x2, std::uint32_t y2) noexcept
{
    set_relative_texture_coords(row, col,
                                static_cast<float>(x1) / static_cast<float>(texture()->width()),
                                static_cast<float>(y1) / static_cast<float>(texture()->height()),
                                static_cast<float>(x2) / static_cast<float>(texture()->width()),
                                static_cast<float>(y2) / static_cast<float>(texture()->height()));
}

void tilemap::set_texture_rect(std::uint32_t row, std::uint32_t col, std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) noexcept
{
    set_texture_coords(row, col, x, y, x + width, y + height);
}

void tilemap::set_texture_rect(std::uint32_t row, std::uint32_t col, const tileset::texture_rect& rect) noexcept
{
    vertex* const current{get_vertices() + (col * m_width) + row};

    current[0].texture_coord = rect.top_left;
    current[1].texture_coord = rect.top_right;
    current[2].texture_coord = rect.bottom_right;
    current[3].texture_coord = rect.bottom_left;

    update();
}

void tilemap::set_relative_texture_coords(std::uint32_t row, std::uint32_t col, float x1, float y1, float x2, float y2) noexcept
{
    vertex* const current{get_vertices() + (col * m_width) + row};

    current[0].texture_coord = glm::vec2{x1, y1};
    current[1].texture_coord = glm::vec2{x2, y1};
    current[2].texture_coord = glm::vec2{x2, y2};
    current[3].texture_coord = glm::vec2{x1, y2};

    update();
}

void tilemap::set_relative_texture_rect(std::uint32_t row, std::uint32_t col, float x, float y, float width, float height) noexcept
{
    set_relative_texture_coords(row, col, x, y, x + width, y + height);
}

void tilemap::init()
{
    std::uint16_t* const indices{get_indices()};
    for(std::uint32_t j{}; j < m_height; ++j)
    {
        for(std::uint32_t i{}; i < m_width; ++i)
        {
            const std::uint32_t shift{((j * m_width) + i) * 4};
            std::uint16_t* const current{indices + ((j * m_width) + i) * 6};

            current[0] = shift + 0;
            current[1] = shift + 1;
            current[2] = shift + 2;
            current[3] = shift + 2;
            current[4] = shift + 3;
            current[5] = shift + 0;
        }
    }

    vertex* const vertices{get_vertices()};
    for(std::uint32_t j{}; j < m_height; ++j)
    {
        for(std::uint32_t i{}; i < m_width; ++i)
        {
            vertex* const current{vertices + ((j * m_width) + i) * 4};

            current[0].position = glm::vec3{i * m_tile_width, j * m_tile_height, 0.0f};
            current[1].position = glm::vec3{(i + 1) * m_tile_width, j * m_tile_height, 0.0f};
            current[2].position = glm::vec3{(i + 1) * m_tile_width, (j + 1) * m_tile_height, 0.0f};
            current[3].position = glm::vec3{i * m_tile_width, (j + 1) * m_tile_height, 0.0f};

            current[0].color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
            current[1].color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
            current[2].color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
            current[3].color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        }
    }
}

}
