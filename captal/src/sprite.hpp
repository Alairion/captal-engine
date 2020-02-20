#ifndef CAPTAL_SPRITE_HPP_INCLUDED
#define CAPTAL_SPRITE_HPP_INCLUDED

#include "config.hpp"

#include "renderable.hpp"
#include "color.hpp"

namespace cpt
{

class CAPTAL_API sprite : public renderable
{
public:
    sprite() = default;
    sprite(std::uint32_t width, std::uint32_t height);
    sprite(texture_ptr texture);
    ~sprite() = default;
    sprite(const sprite&) = delete;
    sprite& operator=(const sprite&) = delete;
    sprite(sprite&&) noexcept = default;
    sprite& operator=(sprite&&) noexcept = default;

    void set_color(const color& color) noexcept;
    void set_color(float red, float green, float blue, float alpha = 1.0f) noexcept;

    void set_texture_coords(std::int32_t x1, std::int32_t y1, std::uint32_t x2, std::uint32_t y2) noexcept;
    void set_texture_rect(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) noexcept;

    void set_relative_texture_coords(float x1, float y1, float x2, float y2) noexcept;
    void set_relative_texture_rect(float x, float y, float width, float height) noexcept;

    void resize(std::uint32_t width, std::uint32_t height) noexcept;

    std::uint32_t width() const noexcept
    {
        return m_width;
    }

    std::uint32_t height() const noexcept
    {
        return m_height;
    }

private:
    void init();

private:
    std::uint32_t m_width{};
    std::uint32_t m_height{};
};

using sprite_ptr = std::shared_ptr<sprite>;
using sprite_weak_ptr = std::weak_ptr<sprite>;

template<typename... Args>
sprite_ptr make_sprite(Args&&... args)
{
    return std::make_shared<sprite>(std::forward<Args>(args)...);
}

}

#endif
