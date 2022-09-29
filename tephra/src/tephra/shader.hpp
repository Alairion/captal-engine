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

class device;

class TEPHRA_API shader
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr shader() = default;
    explicit shader(device& device, shader_stage stage, const std::filesystem::path& file);
    explicit shader(device& device, shader_stage stage, std::span<const std::uint8_t> data);
    explicit shader(device& device, shader_stage stage, std::span<const std::uint32_t> spirv);
    explicit shader(device& device, shader_stage stage, std::istream& stream);

    explicit shader(vulkan::shader shader, shader_stage stage) noexcept
    :m_shader{std::move(shader)}
    ,m_stage{stage}
    {

    }

    ~shader() = default;
    shader(const shader&) = delete;
    shader& operator=(const shader&) = delete;
    shader(shader&&) noexcept = default;
    shader& operator=(shader&&) noexcept = default;

    vulkan::device_context context() const noexcept
    {
        return m_shader.context();
    }

    shader_stage stage() const noexcept
    {
        return m_stage;
    }

private:
    vulkan::shader m_shader{};
    shader_stage m_stage{};
};

TEPHRA_API void set_object_name(device& device, const shader& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const shader& shader) noexcept
{
    return shader.m_shader.device();
}

template<>
inline VkShaderModule underlying_cast(const shader& shader) noexcept
{
    return shader.m_shader;
}

}

#endif
