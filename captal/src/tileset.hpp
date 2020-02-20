#ifndef CAPTAL_TILESET_HPP_INCLUDED
#define CAPTAL_TILESET_HPP_INCLUDED

#include "config.hpp"

#include <glm/vec2.hpp>

#include "texture.hpp"

namespace cpt
{

class CAPTAL_API tileset : public texture
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
    tileset(tph::texture other, std::uint32_t tile_width, std::uint32_t tile_height);
    tileset(std::string_view file, cpt::load_from_file_t, std::uint32_t tile_width, std::uint32_t tile_height, const tph::sampling_options& sampling = tph::sampling_options{});
    tileset(std::string_view data, cpt::load_from_memory_t, std::uint32_t tile_width, std::uint32_t tile_height, const tph::sampling_options& sampling = tph::sampling_options{});
    tileset(std::uint32_t width, std::uint32_t height, const std::uint8_t* rgba, std::uint32_t tile_width, std::uint32_t tile_height, const tph::sampling_options& sampling = tph::sampling_options{});
    tileset(tph::image image, std::uint32_t tile_width, std::uint32_t tile_height, const tph::sampling_options& sampling = tph::sampling_options{});

    virtual ~tileset() = default;
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

        output.top_left     = glm::vec2{static_cast<float>(( col      * m_tile_width) / width()), static_cast<float>(( row * m_tile_height) / height())};
        output.top_right    = glm::vec2{static_cast<float>(((col + 1) * m_tile_width) / width()), static_cast<float>(( row * m_tile_height) / height())};
        output.bottom_right = glm::vec2{static_cast<float>(((col + 1) * m_tile_width) / width()), static_cast<float>(((row + 1) * m_tile_height) / height())};
        output.bottom_left  = glm::vec2{static_cast<float>(( col      * m_tile_width) / width()), static_cast<float>(((row + 1) * m_tile_height) / height())};

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
        return width() / m_tile_width;
    }

    std::uint32_t row_count() const noexcept
    {
        return height() / m_tile_height;
    }

private:
    std::uint32_t m_tile_width{};
    std::uint32_t m_tile_height{};
};

using tileset_ptr = std::shared_ptr<tileset>;
using tileset_weak_ptr = std::weak_ptr<tileset>;

template<typename... Args>
tileset_ptr make_tileset(Args&&... args)
{
    return std::make_shared<tileset>(std::forward<Args>(args)...);
}

}

#endif
