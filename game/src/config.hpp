#ifndef MY_PROJECT_CONFIG_HPP_INCLUDED
#define MY_PROJECT_CONFIG_HPP_INCLUDED

#include <captal/config.hpp>

#include <array>

#include <glm/vec4.hpp>

namespace mpr
{

using namespace cpt::foundation::enum_operations;

inline constexpr cpt::version game_version{MY_PROJECT_MAJOR_VERSION, MY_PROJECT_MINOR_VERSION, MY_PROJECT_PATCH_VERSION};

inline constexpr std::uint32_t normal_map_binding{3};
inline constexpr std::uint32_t height_map_binding{4};
inline constexpr std::uint32_t specular_map_binding{5};
inline constexpr std::uint32_t emission_map_binding{6};
inline constexpr std::uint32_t directional_light_binding{7};

inline constexpr std::array<std::uint8_t, 4> dummy_normal_map_data{128, 128, 255, 255};
inline constexpr std::array<std::uint8_t, 4> dummy_height_map_data{0, 0, 0, 0};
inline constexpr std::array<std::uint8_t, 4> dummy_specular_map_data{0, 0, 0, 0};
inline constexpr std::array<std::uint8_t, 4> dummy_emission_map_data{0, 0, 0, 0};

//For texture pools
inline constexpr std::string_view dummy_normal_map_name = "VIRTUAL/normal_map";
inline constexpr std::string_view dummy_height_map_name = "VIRTUAL/height_map";
inline constexpr std::string_view dummy_specular_map_name = "VIRTUAL/specular_map";
inline constexpr std::string_view dummy_emission_map_name = "VIRTUAL/emission_map";

struct directional_light
{
    glm::vec4 direction{};
    glm::vec4 ambient{};
    glm::vec4 diffuse{};
    glm::vec4 specular{};
};

struct point_light
{
    glm::vec4 position{};
    glm::vec4 ambient{};
    glm::vec4 diffuse{};
    glm::vec4 specular{};
    float constant{};
    float linear{};
    float quadratic{};
    float radius{};
};

struct uniform_lights
{
    std::uint32_t directional_light_count{};
    std::array<directional_light, 2> directional_lights{};
    std::uint32_t point_light_count{};
    std::array<point_light, 32> point_lights{};
};

}

#endif
