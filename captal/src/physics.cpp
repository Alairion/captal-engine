#include "physics.hpp"

#include <stdexcept>

#include <chipmunk/chipmunk.h>

namespace cpt
{

static cpVect to_cp_vect(const glm::vec2& vec)
{
    return cpVect{static_cast<cpFloat>(vec.x), static_cast<cpFloat>(vec.y)};
}

physical_world::physical_world()
:m_world{cpSpaceNew()}
{
    if(!m_world)
        throw std::runtime_error{"Can not allocate physical world."};

    cpSpaceSetUserData(m_world, this);
}

physical_world::~physical_world()
{
    if(m_world)
        cpSpaceFree(m_world);
}

physical_shape::physical_shape(const physical_body_ptr& body, float radius, const glm::vec2& offset)
:m_shape{cpCircleShapeNew(body->handle(), static_cast<cpFloat>(radius), to_cp_vect(offset))}
{
    if(!m_shape)
        throw std::runtime_error{"Can not allocate physical shape."};
}

physical_shape::physical_shape(const physical_body_ptr& body, const glm::vec2& first, const glm::vec2& second, float thickness)
:m_shape{cpSegmentShapeNew(body->handle(), to_cp_vect(first), to_cp_vect(second), static_cast<cpFloat>(thickness))}
{
    if(!m_shape)
        throw std::runtime_error{"Can not allocate physical shape."};
}

physical_shape::physical_shape(const physical_body_ptr& body, const std::vector<glm::vec2>& vertices, float radius)
{
    std::vector<cpVect> native_vertices{};
    native_vertices.reserve(std::size(vertices));
    for(auto&& vertex : vertices)
        native_vertices.push_back(to_cp_vect(vertex));

    m_shape = cpPolyShapeNew(body->handle(), static_cast<int>(std::size(vertices)), std::data(native_vertices), cpTransformIdentity, static_cast<cpFloat>(radius));
    if(!m_shape)
        throw std::runtime_error{"Can not allocate physical shape."};
}

physical_shape::physical_shape(const physical_body_ptr& body, float width, float height, float radius)
:m_shape{cpBoxShapeNew(body->handle(), static_cast<cpFloat>(width), static_cast<cpFloat>(height), static_cast<cpFloat>(radius))}
{
    if(!m_shape)
        throw std::runtime_error{"Can not allocate physical shape."};
}

physical_shape::~physical_shape()
{
    if(m_shape)
        cpShapeFree(m_shape);
}

physical_body::physical_body(physical_body_type type, float mass, float inertia)
{
    if(type == physical_body_type::dynamic)
    {
        m_body = cpBodyNew(static_cast<cpFloat>(mass), static_cast<cpFloat>(inertia));
    }
    else if(type == physical_body_type::steady)
    {
        m_body = cpBodyNewStatic();
    }
    else if(type == physical_body_type::kinematic)
    {
        m_body = cpBodyNewKinematic();
    }

    if(!m_body)
        throw std::runtime_error{"Can not allocate physical body."};

    cpBodySetUserData(m_body, this);
}

physical_body::~physical_body()
{
    if(m_body)
        cpBodyFree(m_body);
}

}
