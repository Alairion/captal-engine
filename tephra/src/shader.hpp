#ifndef TEPHRA_SHADER_HPP_INCLUDED
#define TEPHRA_SHADER_HPP_INCLUDED

#include "config.hpp"

#include <string_view>
#include <istream>
#include <filesystem>

#include "vulkan/vulkan.hpp"

#include "enumerations.hpp"

namespace tph
{

class renderer;

class shader
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr shader() = default;
    shader(renderer& renderer, shader_stage stage, const std::filesystem::path& file);
    shader(renderer& renderer, shader_stage stage, std::string_view data);
    shader(renderer& renderer, shader_stage stage, std::size_t bytes_size, const std::uint32_t* spirv);
    shader(renderer& renderer, shader_stage stage, std::istream& stream);

    ~shader() = default;

    shader(const shader&) = delete;
    shader& operator=(const shader&) = delete;

    shader(shader&&) noexcept = default;
    shader& operator=(shader&&) noexcept = default;

    shader_stage stage() const noexcept
    {
        return m_stage;
    }

private:
    shader_stage m_stage{};
    vulkan::shader m_shader{};
};

template<>
inline VkShaderModule underlying_cast(const shader& shader) noexcept
{
    return shader.m_shader;
}

}

#endif
