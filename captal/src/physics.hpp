#ifndef CAPTAL_PHYSICS_HPP_INCLUDED
#define CAPTAL_PHYSICS_HPP_INCLUDED

#include "config.hpp"

#include <glm/vec2.hpp>

#include <memory>
#include <vector>

struct cpSpace;
struct cpBody;
struct cpShape;

namespace cpt
{

class physical_world
{
public:
    physical_world();
    ~physical_world();
    physical_world(const physical_world&) = delete;
    physical_world& operator=(const physical_world&) = delete;
    physical_world(physical_world&&) noexcept = delete;
    physical_world& operator=(physical_world&&) noexcept = delete;

private:
    cpSpace* m_world{};
};

using physical_world_ptr = std::shared_ptr<physical_world>;

template<typename... Args>
physical_world_ptr make_physical_world(Args&&... args)
{
    return std::make_shared<physical_world>(std::forward<Args>(args)...);
}

class physical_body;
using physical_body_ptr = std::shared_ptr<physical_body>;

class physical_shape
{
public:
    physical_shape() = default;
    physical_shape(const physical_body_ptr& body, float radius, const glm::vec2& offset = glm::vec2{});
    physical_shape(const physical_body_ptr& body, const glm::vec2& first, const glm::vec2& second, float thickness = 0.0f);
    physical_shape(const physical_body_ptr& body, const std::vector<glm::vec2>& vertices, float radius = 0.0f);
    physical_shape(const physical_body_ptr& body, float width, float height, float radius = 0.0f);
    ~physical_shape();
    physical_shape(const physical_shape&) = delete;
    physical_shape& operator=(const physical_shape&) = delete;
    physical_shape(physical_shape&&) noexcept = delete;
    physical_shape& operator=(physical_shape&&) noexcept = delete;

private:
    cpShape* m_shape{};
};

enum class physical_body_type
{
    dynamic = 0,
    steady = 1,
    kinematic = 2
};

class physical_body
{
public:
    physical_body() = default;
    physical_body(physical_body_type type, float mass = 1.0f, float inertia = 1.0f);
    ~physical_body();
    physical_body(const physical_body&) = delete;
    physical_body& operator=(const physical_body&) = delete;
    physical_body(physical_body&&) noexcept = delete;
    physical_body& operator=(physical_body&&) noexcept = delete;

    cpBody* handle() const noexcept
    {
        return m_body;
    }

private:
    cpBody* m_body{};
};

}

#endif
