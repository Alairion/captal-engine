#ifndef TEPHRA_SHADER_HPP_INCLUDED
#define TEPHRA_SHADER_HPP_INCLUDED

#include "config.hpp"

#include <string_view>

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
    shader(renderer& renderer, shader_stage stage, std::string_view file, load_from_file_t);
    shader(renderer& renderer, shader_stage stage, std::string_view spirv, load_from_memory_t);

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
