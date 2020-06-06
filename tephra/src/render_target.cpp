#include "render_target.hpp"

#include <cassert>

#include "vulkan/vulkan_functions.hpp"

#include "vulkan/helper.hpp"
#include "renderer.hpp"
#include "commands.hpp"
#include "synchronization.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

static std::vector<VkImageView> make_image_views(const std::vector<std::reference_wrapper<texture>>& attachments)
{
    std::vector<VkImageView> output{};
    output.reserve(std::size(attachments));

    for(const tph::texture& attachment : attachments)
    {
        output.emplace_back(underlying_cast<VkImageView>(attachment));
    }

    return output;
}

static std::vector<clear_value_t> make_clear_values(const std::vector<std::reference_wrapper<texture>>& attachments)
{
    std::vector<clear_value_t> output{};
    output.reserve(std::size(attachments));

    for(const tph::texture& attachment : attachments)
    {
        if(static_cast<bool>(attachment.aspect() & texture_aspect::color))
        {
            output.emplace_back(clear_color_value{});
        }
        else
        {
            output.emplace_back(clear_depth_stencil_value{});
        }
    }

    return output;
}

framebuffer::framebuffer(renderer& renderer, const render_pass& render_pass, const std::vector<std::reference_wrapper<texture>>& attachments, std::uint32_t width, std::uint32_t height, std::uint32_t layers)
:m_framebuffer{underlying_cast<VkDevice>(renderer), underlying_cast<VkRenderPass>(render_pass), make_image_views(attachments), VkExtent2D{width, height}, layers}
,m_clear_values{make_clear_values(attachments)}
,m_width{width}
,m_height{height}
,m_layers{layers}
{

}

void framebuffer::set_clear_value(std::uint32_t attachment_index, const clear_color_value& value)
{
    assert(std::holds_alternative<clear_color_value>(m_clear_values[attachment_index]) && "Clear value type does not correspond to attachment's aspect.");

    m_clear_values[attachment_index] = value;
}

void framebuffer::set_clear_value(std::uint32_t attachment_index, const clear_depth_stencil_value& value)
{
    assert(std::holds_alternative<clear_depth_stencil_value>(m_clear_values[attachment_index]) && "Clear value type does not correspond to attachment's aspect.");

    m_clear_values[attachment_index] = value;
}

void framebuffer::set_clear_values(std::vector<clear_value_t> clear_values)
{
    m_clear_values = std::move(clear_values);
}

static std::vector<VkAttachmentDescription> make_attachments(const std::vector<attachment_description>& attachments)
{
    std::vector<VkAttachmentDescription> output{};
    output.reserve(std::size(attachments));

    for(auto&& attachment : attachments)
    {
        auto& native_attachment{output.emplace_back()};

        native_attachment.format = static_cast<VkFormat>(attachment.format);
        native_attachment.samples = static_cast<VkSampleCountFlagBits>(attachment.sample_count);
        native_attachment.loadOp = static_cast<VkAttachmentLoadOp>(attachment.load_op);
        native_attachment.storeOp = static_cast<VkAttachmentStoreOp>(attachment.store_op);
        native_attachment.stencilLoadOp = static_cast<VkAttachmentLoadOp>(attachment.stencil_load_op);
        native_attachment.stencilStoreOp = static_cast<VkAttachmentStoreOp>(attachment.stencil_store_op);
        native_attachment.initialLayout = static_cast<VkImageLayout>(attachment.initial_layout);
        native_attachment.finalLayout = static_cast<VkImageLayout>(attachment.final_layout);
    }

    return output;
}

static VkAttachmentReference convert_attachment(const attachment_reference& attachment)
{
    VkAttachmentReference output{};

    output.layout = static_cast<VkImageLayout>(attachment.layout);
    output.attachment = attachment.attachment;

    return output;
}

static std::vector<VkAttachmentReference> convert_attachments(const std::vector<attachment_reference>& attachments)
{
    std::vector<VkAttachmentReference> output{};

    output.reserve(std::size(attachments));
    for(auto&& attachment : attachments)
    {
        output.emplace_back(convert_attachment(attachment));
    }

    return output;
}

struct subpass_data
{
    std::vector<VkAttachmentReference> input_attachments{};
    std::vector<VkAttachmentReference> color_attachments{};
    std::vector<VkAttachmentReference> resolve_attachments{};
    std::unique_ptr<VkAttachmentReference> depth_attachment{};
    std::vector<std::uint32_t> preserve_attachments{};
};

static std::vector<subpass_data> make_subpasses_data(const std::vector<subpass_description>& subpasses)
{
    std::vector<subpass_data> output{};
    output.reserve(std::size(subpasses));

    for(auto&& subpass : subpasses)
    {
        auto& native_subpass{output.emplace_back()};

        native_subpass.input_attachments = convert_attachments(subpass.input_attachments);
        native_subpass.color_attachments = convert_attachments(subpass.color_attachments);
        native_subpass.resolve_attachments = convert_attachments(subpass.resolve_attachments);

        if(subpass.depth_attachment)
        {
            native_subpass.depth_attachment = std::make_unique<VkAttachmentReference>(convert_attachment(subpass.depth_attachment.value()));
        }

        native_subpass.preserve_attachments = subpass.preserve_attachments;
    }

    return output;
}

static std::vector<VkSubpassDescription> make_subpasses(const std::vector<subpass_data>& subpasses_data)
{
    std::vector<VkSubpassDescription> output{};
    output.reserve(std::size(subpasses_data));

    for(auto&& data : subpasses_data)
    {
        auto& native_subpass{output.emplace_back()};

        native_subpass.inputAttachmentCount = static_cast<std::uint32_t>(std::size(data.input_attachments));
        native_subpass.pInputAttachments = std::data(data.input_attachments);
        native_subpass.colorAttachmentCount = static_cast<std::uint32_t>(std::size(data.color_attachments));
        native_subpass.pColorAttachments = std::data(data.color_attachments);
        native_subpass.pResolveAttachments = std::data(data.resolve_attachments);
        native_subpass.preserveAttachmentCount = static_cast<std::uint32_t>(std::size(data.preserve_attachments));
        native_subpass.pPreserveAttachments = std::data(data.preserve_attachments);
        native_subpass.pDepthStencilAttachment = data.depth_attachment.get();
    }

    return output;
}

static std::vector<VkSubpassDependency> make_dependencies(const std::vector<subpass_dependency>& dependencies)
{
    std::vector<VkSubpassDependency> output{};
    output.reserve(std::size(dependencies));

    for(auto&& dependency : dependencies)
    {
        auto& native_dependency{output.emplace_back()};

        native_dependency.srcSubpass = dependency.source_subpass;
        native_dependency.dstSubpass = dependency.destination_subpass;
        native_dependency.srcStageMask = static_cast<VkPipelineStageFlags>(dependency.source_stage);
        native_dependency.dstStageMask = static_cast<VkPipelineStageFlags>(dependency.destination_stage);
        native_dependency.srcAccessMask = static_cast<VkAccessFlags>(dependency.source_access);
        native_dependency.dstAccessMask = static_cast<VkAccessFlags>(dependency.destination_access);
        native_dependency.dependencyFlags = static_cast<VkDependencyFlags>(dependency.dependency_flags);
    }

    return output;
}

render_pass::render_pass(renderer& renderer, const render_pass_info& info)
:m_device{underlying_cast<VkDevice>(renderer)}
{
    const auto attachments{make_attachments(info.attachments)};
    const auto subpasses_data{make_subpasses_data(info.subpasses)};
    const auto subpass{make_subpasses(subpasses_data)};
    const auto dependencies{make_dependencies(info.dependencies)};

    m_render_pass = vulkan::render_pass{underlying_cast<VkDevice>(renderer), attachments, subpass, dependencies};
}

}
