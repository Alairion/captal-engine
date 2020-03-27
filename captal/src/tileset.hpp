#ifndef CAPTAL_TILESET_HPP_INCLUDED
#define CAPTAL_TILESET_HPP_INCLUDED

#include "config.hpp"

#include <glm/vec2.hpp>

#include "texture.hpp"

namespace cpt
{

class tileset
{
public:
    struct texture_rect
    {
        glm::vec2 top_left{};
        glm::vec2 top_right{};
        glm::vec2 bottom_right{};
        glm::vec2 bottom_left{};
    };

public:
    tileset() = default;

    tileset(texture_ptr texture, std::uint32_t tile_width, std::uint32_t tile_height)
    :m_texture{std::move(texture)}
    ,m_tile_width{tile_width}
    ,m_tile_height{tile_height}
    {

    }

    ~tileset() = default;
    tileset(const tileset&) = delete;
    tileset& operator=(const tileset&) = delete;
    tileset(tileset&&) noexcept = default;
    tileset& operator=(tileset&&) noexcept = default;

    texture_rect compute_rect(std::uint32_t index) const noexcept
    {
        return compute_rect(index % col_count(), index / row_count());
    }

    texture_rect compute_rect(std::uint32_t col, std::uint32_t row) const noexcept
    {
        texture_rect output{};

        output.top_left     = glm::vec2{static_cast<float>(( col      * m_tile_width) / m_texture->width()), static_cast<float>(( row      * m_tile_height) / m_texture->height())};
        output.top_right    = glm::vec2{static_cast<float>(((col + 1) * m_tile_width) / m_texture->width()), static_cast<float>(( row      * m_tile_height) / m_texture->height())};
        output.bottom_right = glm::vec2{static_cast<float>(((col + 1) * m_tile_width) / m_texture->width()), static_cast<float>(((row + 1) * m_tile_height) / m_texture->height())};
        output.bottom_left  = glm::vec2{static_cast<float>(( col      * m_tile_width) / m_texture->width()), static_cast<float>(((row + 1) * m_tile_height) / m_texture->height())};

        return output;
    }

    std::uint32_t tile_width() const noexcept
    {
        return m_tile_width;
    }

    std::uint32_t tile_height() const noexcept
    {
        return m_tile_height;
    }

    std::uint32_t col_count() const noexcept
    {
        return m_texture->width() / m_tile_width;
    }

    std::uint32_t row_count() const noexcept
    {
        return m_texture->height() / m_tile_height;
    }

    const texture_ptr& texture() const noexcept
    {
        return m_texture;
    }

private:
    texture_ptr m_texture{};
    std::uint32_t m_tile_width{};
    std::uint32_t m_tile_height{};
};

}

#endif
