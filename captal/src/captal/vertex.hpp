#ifndef CAPTAL_VERTEX_HPP_INCLUDED
#define CAPTAL_VERTEX_HPP_INCLUDED

#include "config.hpp"

#include <captal_foundation/math.hpp>

namespace cpt
{

struct vertex
{
    vec3f position{};
    vec4f color{};
    vec2f texture_coord{};
};

}

#endif
