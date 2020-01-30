#include "sprite.hpp"

namespace cpt
{

sprite::sprite(std::uint32_t width, std::uint32_t height)
:renderable{6, 4}
,m_width{width}
,m_height{height}
{
    init();
}

sprite::sprite(texture_ptr texture)
:renderable{6, 4}
,m_width{texture->width()}
,m_height{texture->height()}
{
    init();
    set_texture(std::move(texture));
}

void sprite::set_color(const glm::vec4& color) noexcept
{
    vertex* vertices{get_vertices()};

    vertices[0].color = color;
    vertices[1].color = color;
    vertices[2].color = color;
    vertices[3].color = color;

    update();
}

void sprite::set_color(float red, float green, float blue, float alpha) noexcept
{
    set_color(glm::vec4{red, green, blue, alpha});
}

void sprite::set_texture_coords(std::int32_t x1, std::int32_t y1, std::uint32_t x2, std::uint32_t y2) noexcept
{
    set_relative_texture_coords(static_cast<float>(x1) / static_cast<float>(texture()->width()),
                                static_cast<float>(y1) / static_cast<float>(texture()->height()),
                                static_cast<float>(x2) / static_cast<float>(texture()->width()),
                                static_cast<float>(y2) / static_cast<float>(texture()->height()));
}

void sprite::set_texture_rect(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) noexcept
{
    set_texture_coords(x, y, x + width, y + height);
}

void sprite::set_relative_texture_coords(float x1, float y1, float x2, float y2) noexcept
{
    vertex* vertices{get_vertices()};

    vertices[0].texture_coord = glm::vec2{x1, y1};
    vertices[1].texture_coord = glm::vec2{x2, y1};
    vertices[2].texture_coord = glm::vec2{x2, y2};
    vertices[3].texture_coord = glm::vec2{x1, y2};

    update();
}

void sprite::set_relative_texture_rect(float x, float y, float width, float height) noexcept
{
    set_relative_texture_coords(x, y, x + width, y + height);
}

void sprite::resize(std::uint32_t width, std::uint32_t height) noexcept
{
    m_width = width;
    m_height = height;

    vertex* vertices{get_vertices()};

    vertices[0].position = glm::vec3{0.0f, 0.0f, 0.0f};
    vertices[1].position = glm::vec3{static_cast<float>(m_width), 0.0f, 0.0f};
    vertices[2].position = glm::vec3{static_cast<float>(m_width), static_cast<float>(m_height), 0.0f};
    vertices[3].position = glm::vec3{0.0f, static_cast<float>(m_height), 0.0f};

    update();
}

void sprite::init()
{
    set_indices({0, 1, 2, 2, 3, 0});

    resize(m_width, m_height);
    set_color(glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
    set_relative_texture_coords(0.0f, 0.0f, 1.0f, 1.0f);
}

}
