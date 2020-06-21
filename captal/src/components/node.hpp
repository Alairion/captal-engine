#ifndef CAPTAL_COMPONENTS_NODE_HPP_INCLUDED
#define CAPTAL_COMPONENTS_NODE_HPP_INCLUDED

#include "../config.hpp"

#include <cmath>
#include <numbers>

#include <glm/vec3.hpp>

namespace cpt
{

namespace components
{

class CAPTAL_API node
{
public:
    node() = default;

    explicit node(const glm::vec3& position, const glm::vec3& origin = glm::vec3{0.0f, 0.0f, 0.0f}, float scale = 1.0f, float angle = 0.0f)
    :m_position{position}
    ,m_origin{origin}
    ,m_rotation{std::fmod(angle, std::numbers::pi_v<float> * 2.0f)}
    ,m_scale{scale}
    ,m_updated{true}
    {

    }

    ~node() = default;
    node(const node&) = default;
    node& operator=(const node&) = default;
    node(node&&) noexcept = default;
    node& operator=(node&&) noexcept = default;

    void move_to(const glm::vec3& position) noexcept
    {
        m_position = position;
        m_updated = true;
    }

    void move_to(float x, float y, float z = 0.0f) noexcept
    {
        m_position = glm::vec3{x, y, z};
        m_updated = true;
    }

    void move(const glm::vec3& relative) noexcept
    {
        m_position += relative;
        m_updated = true;
    }

    void move(float x, float y, float z = 0.0f) noexcept
    {
        m_position += glm::vec3{x, y, z};
        m_updated = true;
    }

    void set_origin(const glm::vec3& origin) noexcept
    {
        m_origin = origin;
        m_updated = true;
    }

    void set_origin(float x, float y, float z = 0.0f) noexcept
    {
        m_origin = glm::vec3{x, y, z};
        m_updated = true;
    }

    void move_origin(const glm::vec3& relative) noexcept
    {
        m_origin += relative;
        m_updated = true;
    }

    void move_origin(float x, float y, float z = 0.0f) noexcept
    {
        m_origin += glm::vec3{x, y, z};
        m_updated = true;
    }

    void set_rotation(float angle) noexcept
    {
        m_rotation = std::fmod(angle, std::numbers::pi_v<float> * 2.0f);
        m_updated = true;
    }

    void rotate(float angle) noexcept
    {
        m_rotation = std::fmod(m_rotation + angle, std::numbers::pi_v<float> * 2.0f);
        m_updated = true;
    }

    void set_scale(float scale) noexcept
    {
        m_scale = scale;
        m_updated = true;
    }

    void scale(float scale) noexcept
    {
        m_scale *= scale;
        m_updated = true;
    }

    const glm::vec3& position() const noexcept
    {
        return m_position;
    }

    const glm::vec3& origin() const noexcept
    {
        return m_origin;
    }

    glm::vec3 real_position() const noexcept
    {
        return m_position - m_origin;
    }

    float scale() const noexcept
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
    glm::vec3 m_position{};
    glm::vec3 m_origin{};
    float m_rotation{};
    float m_scale{1.0f};
    bool m_updated{};
};

}

}

#endif
