#ifndef CAPTAL_TILEMAP_HPP_INCLUDED
#define CAPTAL_TILEMAP_HPP_INCLUDED

#include "config.hpp"

#include "renderable.hpp"
#include "tileset.hpp"
#include "color.hpp"

namespace cpt
{

class CAPTAL_API tilemap : public renderable
{
public:
    tilemap() = default;
    tilemap(std::uint32_t width, std::uint32_t height, std::uint32_t tile_width, std::uint32_t tile_height);
    tilemap(std::uint32_t width, std::uint32_t height, tileset_ptr tileset);

    ~tilemap() = default;
    tilemap(const tilemap&) = delete;
    tilemap& operator=(const tilemap&) = delete;
    tilemap(tilemap&&) noexcept = default;
    tilemap& operator=(tilemap&&) noexcept = default;

    void set_color(std::uint32_t row, std::uint32_t col, const color& color) noexcept;
    void set_color(std::uint32_t row, std::uint32_t col, float red, float green, float blue, float alpha = 1.0f) noexcept;

    void set_texture_coords(std::uint32_t row, std::uint32_t col, std::int32_t x1, std::int32_t y1, std::uint32_t x2, std::uint32_t y2) noexcept;
    void set_texture_rect(std::uint32_t row, std::uint32_t col, std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) noexcept;
    void set_texture_rect(std::uint32_t row, std::uint32_t col, const tileset::texture_rect& rect) noexcept;

    void set_relative_texture_coords(std::uint32_t row, std::uint32_t col, float x1, float y1, float x2, float y2) noexcept;
    void set_relative_texture_rect(std::uint32_t row, std::uint32_t col, float x, float y, float width, float height) noexcept;

    std::uint32_t width() const noexcept
    {
        return m_width;
    }

    std::uint32_t height() const noexcept
    {
        return m_height;
    }

    std::uint32_t tile_width() const noexcept
    {
        return m_width;
    }

    std::uint32_t tile_height() const noexcept
    {
        return m_height;
    }

private:
    void init();

private:
    std::uint32_t m_width{};
    std::uint32_t m_height{};
    std::uint32_t m_tile_width{};
    std::uint32_t m_tile_height{};
};

using tilemap_ptr = std::shared_ptr<tilemap>;

template<typename... Args>
tilemap_ptr make_tilemap(Args&&... args)
{
    return std::make_shared<tilemap>(std::forward<Args>(args)...);
}

}

#endif
