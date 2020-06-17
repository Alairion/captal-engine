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
#include "synchronization.hpp"
#include "surface.hpp"
#include "texture.hpp"

namespace tph
{

class renderer;

enum class render_pass_content : std::uint32_t
{
    inlined = VK_SUBPASS_CONTENTS_INLINE,
    recorded = VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS,
};

enum class attachment_load_op : std::uint32_t
{
    load = VK_ATTACHMENT_LOAD_OP_LOAD,
    clear = VK_ATTACHMENT_LOAD_OP_CLEAR,
    dont_care = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
};

enum class attachment_store_op : std::uint32_t
{
    store = VK_ATTACHMENT_STORE_OP_STORE,
    dont_care = VK_ATTACHMENT_STORE_OP_DONT_CARE,
};

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
    dependency_flags dependency_flags{};
};

struct render_pass_info
{
    std::vector<attachment_description> attachments{};
    std::vector<subpass_description> subpasses{};
    std::vector<subpass_dependency> dependencies{};
};

class render_pass;

struct clear_color_value
{
    float red{};
    float green{};
    float blue{};
    float alpha{1.0f};
};

struct clear_depth_stencil_value
{
    float depth{};
    std::uint32_t stencil{};
};

using clear_value_t = std::variant<clear_color_value, clear_depth_stencil_value>;

class framebuffer
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr framebuffer() = default;

    framebuffer(renderer& renderer, const render_pass& render_pass, std::span<const std::reference_wrapper<texture>> attachments, std::uint32_t width, std::uint32_t height, std::uint32_t layers);

    ~framebuffer() = default;
    framebuffer(const framebuffer&) = delete;
    framebuffer& operator=(const framebuffer&) = delete;
    framebuffer(framebuffer&& other) noexcept = default;
    framebuffer& operator=(framebuffer&& other) noexcept = default;

    void set_clear_value(std::uint32_t attachment_index, const clear_color_value& value);
    void set_clear_value(std::uint32_t attachment_index, const clear_depth_stencil_value& value);
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

template<>
inline VkFramebuffer underlying_cast(const framebuffer& framebuffer) noexcept
{
    return framebuffer.m_framebuffer;
}

class render_pass
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr render_pass() = default;

    render_pass(renderer& renderer, const render_pass_info& info);

    ~render_pass() = default;
    render_pass(const render_pass&) = delete;
    render_pass& operator=(const render_pass&) = delete;
    render_pass(render_pass&& other) noexcept = default;
    render_pass& operator=(render_pass&& other) noexcept = default;

private:
    VkDevice m_device{};
    vulkan::render_pass m_render_pass{};
};

template<>
inline VkRenderPass underlying_cast(const render_pass& render_pass) noexcept
{
    return render_pass.m_render_pass;
}

}

//template<> struct tph::enable_enum_operations<tph::render_target_options> {static constexpr bool value{true};};

#endif
