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

#ifndef TEPHRA_RENDER_TARGET_HPP_INCLUDED
#define TEPHRA_RENDER_TARGET_HPP_INCLUDED

#include "config.hpp"

#include <vector>
#include <memory>
#include <variant>
#include <cassert>

#include "vulkan/vulkan.hpp"
#include "vulkan/memory.hpp"

#include "enumerations.hpp"
#include "texture.hpp"

namespace tph
{

class renderer;

struct attachment_description
{
    texture_format format{};
    tph::sample_count sample_count{};
    attachment_load_op load_op{};
    attachment_store_op store_op{};
    attachment_load_op stencil_load_op{};
    attachment_store_op stencil_store_op{};
    texture_layout initial_layout{};
    texture_layout final_layout{};
};

static constexpr std::uint32_t unused_attachment{VK_ATTACHMENT_UNUSED};

struct attachment_reference
{
    std::uint32_t attachment{};
    texture_layout layout{};
};

struct subpass_description
{
    std::vector<attachment_reference> input_attachments{};
    std::vector<attachment_reference> color_attachments{};
    std::vector<attachment_reference> resolve_attachments{};
    std::optional<attachment_reference> depth_attachment{};
    std::vector<std::uint32_t> preserve_attachments{};
};

static constexpr std::uint32_t external_subpass{VK_SUBPASS_EXTERNAL};

struct subpass_dependency
{
    std::uint32_t source_subpass{};
    std::uint32_t destination_subpass{};
    pipeline_stage source_stage{};
    pipeline_stage destination_stage{};
    resource_access source_access{};
    resource_access destination_access{};
    tph::dependency_flags dependency_flags{};
};

struct render_pass_info
{
    std::vector<attachment_description> attachments{};
    std::vector<subpass_description> subpasses{};
    std::vector<subpass_dependency> dependencies{};
};

class render_pass;

class TEPHRA_API framebuffer
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr framebuffer() = default;
    explicit framebuffer(renderer& renderer, const render_pass& render_pass, std::span<const std::reference_wrapper<texture_view>> attachments, std::uint32_t width, std::uint32_t height, std::uint32_t layers);

    explicit framebuffer(vulkan::framebuffer framebuffer, std::vector<clear_value_t> clear_values, std::uint32_t width, std::uint32_t height, std::uint32_t layers) noexcept
    :m_framebuffer{std::move(framebuffer)}
    ,m_clear_values{std::move(clear_values)}
    ,m_width{width}
    ,m_height{height}
    ,m_layers{layers}
    {

    }

    ~framebuffer() = default;
    framebuffer(const framebuffer&) = delete;
    framebuffer& operator=(const framebuffer&) = delete;
    framebuffer(framebuffer&& other) noexcept = default;
    framebuffer& operator=(framebuffer&& other) noexcept = default;

    void set_clear_value(std::uint32_t attachment_index, const clear_value_t& value);
    void set_clear_values(std::vector<clear_value_t> clear_values);

    std::span<const clear_value_t> clear_values() const noexcept
    {
        return m_clear_values;
    }

    std::uint32_t width() const noexcept
    {
        return m_width;
    }

    std::uint32_t height() const noexcept
    {
        return m_height;
    }

    std::uint32_t layers() const noexcept
    {
        return m_layers;
    }

private:
    vulkan::framebuffer m_framebuffer{};
    std::vector<clear_value_t> m_clear_values{};
    std::uint32_t m_width{};
    std::uint32_t m_height{};
    std::uint32_t m_layers{};
};

TEPHRA_API void set_object_name(renderer& renderer, const framebuffer& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const framebuffer& framebuffer) noexcept
{
    return framebuffer.m_framebuffer.device();
}

template<>
inline VkFramebuffer underlying_cast(const framebuffer& framebuffer) noexcept
{
    return framebuffer.m_framebuffer;
}

class TEPHRA_API render_pass
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr render_pass() = default;
    explicit render_pass(renderer& renderer, const render_pass_info& info);

    explicit render_pass(vulkan::render_pass render_pass) noexcept
    :m_render_pass{std::move(render_pass)}
    {

    }

    ~render_pass() = default;
    render_pass(const render_pass&) = delete;
    render_pass& operator=(const render_pass&) = delete;
    render_pass(render_pass&& other) noexcept = default;
    render_pass& operator=(render_pass&& other) noexcept = default;

private:
    vulkan::render_pass m_render_pass{};
};

TEPHRA_API void set_object_name(renderer& renderer, const render_pass& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const render_pass& render_pass) noexcept
{
    return render_pass.m_render_pass.device();
}

template<>
inline VkRenderPass underlying_cast(const render_pass& render_pass) noexcept
{
    return render_pass.m_render_pass;
}

}

#endif
