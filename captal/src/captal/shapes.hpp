#ifndef CAPTAL_SHAPE_HPP_INCLUDED
#define CAPTAL_SHAPE_HPP_INCLUDED

#include "config.hpp"

#include <vector>

#include <captal_foundation/math.hpp>

namespace cpt
{

CAPTAL_API std::vector<vec2f> circle(float radius, std::uint32_t point_count);
CAPTAL_API std::vector<vec2f> circle(float radius);
CAPTAL_API std::vector<vec2f> ellipse(float width, float height, std::uint32_t point_count);
CAPTAL_API std::vector<vec2f> ellipse(float width, float height);

}

#endif // SHAPE_HPP
