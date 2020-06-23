#include "shapes.hpp"

#include <numbers>
#include <cmath>

namespace cpt
{

static std::uint32_t compute_circle_point_count(float radius) noexcept
{
    const float circumference{2.0f * std::numbers::pi_v<float> * radius};

    return static_cast<std::uint32_t>(std::ceil(circumference / 8.0f) * 8.0f);
}

std::vector<glm::vec2> circle(float radius, std::uint32_t point_count)
{
    assert(point_count > 2 && "cpt::polygon created with less than 3 points.");

    std::vector<glm::vec2> points{};
    points.reserve(point_count);

    const float step{(2.0f * std::numbers::pi_v<float>) / point_count};
    for(std::uint32_t i{}; i < point_count; ++i)
    {
        const float angle{step * i};
        points.emplace_back(std::cos(angle) * radius, std::sin(angle) * radius);
    }

    return points;
}

std::vector<glm::vec2> circle(float radius)
{
    return circle(radius, compute_circle_point_count(radius));
}

static std::uint32_t compute_ellipse_point_count(float width, float height) noexcept
{
    const float circumference{std::numbers::pi_v<float> * std::sqrt(2.0f * (width * width + height * height))};

    return static_cast<std::uint32_t>(std::ceil(circumference / 8.0f) * 8.0f);
}

std::vector<glm::vec2> ellipse(float width, float height, std::uint32_t point_count)
{
    assert(point_count > 2 && "cpt::polygon created with less than 3 points.");

    std::vector<glm::vec2> points{};
    points.reserve(point_count);

    const float step{(2.0f * std::numbers::pi_v<float>) / point_count};
    for(std::uint32_t i{}; i < point_count; ++i)
    {
        const float angle{step * i};
        points.emplace_back(std::cos(angle) * width, std::sin(angle) * height);
    }

    return points;
}

std::vector<glm::vec2> ellipse(float width, float height)
{
    return ellipse(width, height, compute_ellipse_point_count(width, height));
}

}
