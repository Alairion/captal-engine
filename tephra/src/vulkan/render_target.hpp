#ifndef TEPHRA_VULKAN_RENDER_TARGET_HPP_INCLUDED
#define TEPHRA_VULKAN_RENDER_TARGET_HPP_INCLUDED

#include "../config.hpp"

#include <variant>

#include "vulkan.hpp"

#include "../render_target.hpp"

namespace tph::vulkan
{

class memory_allocator;

struct attachment_description
{
    texture_format format{texture_format::undefined};
    tph::sample_count sample_count{tph::sample_count::msaa_x1};
    attachment_load_op load_op{attachment_load_op::dont_care};
    attachment_load_op store_op{attachment_load_op::dont_care};
    attachment_load_op stencil_load_op{attachment_load_op::dont_care};
    attachment_load_op stencil_store_op{attachment_load_op::dont_care};
    texture_layout initial_layout{texture_layout::undefined};
    texture_layout final_layout{texture_layout::undefined};
};

static constexpr std::uint32_t unused_attachment{VK_ATTACHMENT_UNUSED};

struct attachment_reference
{
    std::uint32_t attachment{unused_attachment};
    texture_layout layout{texture_layout::undefined};
};

struct subpass_description
{
    std::vector<attachment_reference> input_attachments{};
    std::vector<attachment_reference> color_attachments{};
    std::vector<attachment_reference> resolve_attachments{};
    attachment_reference depth_attachment{};
    std::vector<std::uint32_t> preserve_attachments{};
};

static constexpr std::uint32_t external_subpass{VK_SUBPASS_EXTERNAL};

struct subpass_dependency
{
    std::uint32_t source_subpass{external_subpass};
    std::uint32_t destination_subpass{external_subpass};
    pipeline_stage source_stage{};
    pipeline_stage destination_stage{};
    resource_access source_access{};
    resource_access destination_access{};
    dependency_flags dependency_flags{};
};

struct render_target_framebuffer
{
    std::vector<std::variant<std::reference_wrapper<texture>, std::uint32_t>> attachments{};
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint32_t layers{};
};

struct render_pass_info
{
    std::vector<attachment_description> attachments{};
    std::vector<subpass_description> subpasses{};
    std::vector<subpass_dependency> dependencies{};
    std::vector<render_target_framebuffer> framebuffers{};
};

class render_target
{

private:
    VkPhysicalDevice physical_device{};
    VkDevice device{};
    vulkan::memory_allocator* allocator{};
    std::uint32_t graphics_family{};
    VkQueue present_queue{};

};

}

#endif
