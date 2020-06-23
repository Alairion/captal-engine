#ifndef CAPTAL_VERTEX_HPP_INCLUDED
#define CAPTAL_VERTEX_HPP_INCLUDED

#include "config.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace cpt
{

struct vertex
{
    glm::vec3 position{};
    glm::vec4 color{};
    glm::vec2 texture_coord{};
};

}

#endif
