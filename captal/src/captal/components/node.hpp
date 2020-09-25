#ifndef CAPTAL_COMPONENTS_NODE_HPP_INCLUDED
#define CAPTAL_COMPONENTS_NODE_HPP_INCLUDED

#include "../config.hpp"

#include <cmath>
#include <numbers>

#include <captal_foundation/math.hpp>

namespace cpt
{

namespace components
{

class CAPTAL_API node
{
public:
    node() = default;

    explicit node(const vec3f& position, const vec3f& origin = vec3f{0.0f}, const vec3f& scale = vec3f{1.0f}, float angle = 0.0f)
    :m_position{position}
    ,m_origin{origin}
    ,m_scale{scale}
    ,m_rotation{std::fmod(angle, std::numbers::pi_v<float> * 2.0f)}
    {

    }

    ~node() = default;
    node(const node&) = default;
    node& operator=(const node&) = default;
    node(node&&) noexcept = default;
    node& operator=(node&&) noexcept = default;

    void move_to(const vec3f& position) noexcept
    {
        m_position = position;
        m_updated = true;
    }

    void move(const vec3f& relative) noexcept
    {
        m_position += relative;
        m_updated = true;
    }

    void set_origin(const vec3f& origin) noexcept
    {
        m_origin = origin;
        m_updated = true;
    }

    void move_origin(const vec3f& relative) noexcept
    {
        m_origin += relative;
        m_updated = true;
    }

    void set_rotation(float angle) noexcept
    {
        m_rotation = std::fmod(angle, std::numbers::pi_v<float> * 2.0f);
        m_updated = true;
    }

    void set_scale(const vec3f& scale) noexcept
    {
        m_scale = scale;
        m_updated = true;
    }

    void scale(const vec3f& scale) noexcept
    {
        m_scale *= scale;
        m_updated = true;
    }

    void rotate(float angle) noexcept
    {
        m_rotation = std::fmod(m_rotation + angle, std::numbers::pi_v<float> * 2.0f);
        m_updated = true;
    }

    const vec3f& position() const noexcept
    {
        return m_position;
    }

    const vec3f& origin() const noexcept
    {
        return m_origin;
    }

    vec3f real_position() const noexcept
    {
        return m_position - m_origin;
    }

    const vec3f& scale() const noexcept
    {
        return m_scale;
    }

    float rotation() const noexcept
    {
        return m_rotation;
    }

    void update() noexcept
    {
        m_updated = true;
    }

    bool is_updated() const noexcept
    {
        return m_updated;
    }

    void clear() noexcept
    {
        m_updated = false;
    }

private:
    vec3f m_position{};
    vec3f m_origin{};
    vec3f m_scale{1.0f};
    float m_rotation{};
    bool m_updated{true};
};

}

}

#endif
