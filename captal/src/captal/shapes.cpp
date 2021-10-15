//MIT License
//
//Copyright (c) 2021 Alexy Pellegrini
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "shapes.hpp"

#include <numbers>
#include <cmath>
#include <cassert>

namespace cpt
{

static std::uint32_t compute_circle_point_count(float radius) noexcept
{
    const float circumference{2.0f * std::numbers::pi_v<float> * radius};

    return static_cast<std::uint32_t>(std::ceil(circumference / 8.0f) * 8.0f);
}

std::vector<vec2f> circle(float radius, std::uint32_t point_count)
{
    assert(point_count > 2 && "cpt::circle called with less than 3 points.");

    std::vector<vec2f> points{};
    points.reserve(point_count);

    const float step{(2.0f * std::numbers::pi_v<float>) / static_cast<float>(point_count)};
    for(std::uint32_t i{}; i < point_count; ++i)
    {
        const float angle{step * static_cast<float>(i)};
        points.emplace_back(std::cos(angle) * radius, std::sin(angle) * radius);
    }

    return points;
}

std::vector<vec2f> circle(float radius)
{
    return circle(radius, compute_circle_point_count(radius));
}

static std::uint32_t compute_ellipse_point_count(float width, float height) noexcept
{
    const float circumference{std::numbers::pi_v<float> * std::sqrt(2.0f * (width * width + height * height))};

    return static_cast<std::uint32_t>(std::ceil(circumference / 8.0f) * 8.0f);
}

std::vector<vec2f> ellipse(float width, float height, std::uint32_t point_count)
{
    assert(point_count > 2 && "cpt::circle called with less than 3 points.");

    std::vector<vec2f> points{};
    points.reserve(point_count);

    const float step{(2.0f * std::numbers::pi_v<float>) / static_cast<float>(point_count)};
    for(std::uint32_t i{}; i < point_count; ++i)
    {
        const float angle{step * static_cast<float>(i)};
        points.emplace_back(std::cos(angle) * width, std::sin(angle) * height);
    }

    return points;
}

std::vector<vec2f> ellipse(float width, float height)
{
    return ellipse(width, height, compute_ellipse_point_count(width, height));
}

}
