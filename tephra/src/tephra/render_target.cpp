#include "render_target.hpp"

#include <cassert>

#include <captal_foundation/stack_allocator.hpp>

#include "vulkan/vulkan_functions.hpp"
#include "vulkan/helper.hpp"

#include "renderer.hpp"
#include "commands.hpp"
#include "synchronization.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

static vulkan::framebuffer make_framebuffer(renderer& renderer, const render_pass& render_pass, std::span<const std::reference_wrapper<texture>> attachments, std::uint32_t width, std::uint32_t height, std::uint32_t layers)
{
    stack_memory_pool<512> pool{};

    auto native_attachments{make_stack_vector<VkImageView>(pool)};
    native_attachments.reserve(std::size(attachments));
    for(const tph::texture& attachment : attachments)
    {
        native_attachments.emplace_back(underlying_cast<VkImageView>(attachment));
    }

    return vulkan::framebuffer{underlying_cast<VkDevice>(renderer), underlying_cast<VkRenderPass>(render_pass), native_attachments, VkExtent2D{width, height}, layers};
}

static std::vector<clear_value_t> make_clear_values(std::span<const std::reference_wrapper<texture>> attachments)
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

framebuffer::framebuffer(renderer& renderer, const render_pass& render_pass, std::span<const std::reference_wrapper<texture>> attachments, std::uint32_t width, std::uint32_t height, std::uint32_t layers)
:m_framebuffer{make_framebuffer(renderer, render_pass, attachments, width, height, layers)}
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

//Don't know if I should keep this lul
static constexpr std::size_t stack_size{1024 * 4};
using memory_pool_t = stack_memory_pool<stack_size>;
template<typename T>
using stack_vector_t = stack_vector<T, stack_size>;

static stack_vector_t<VkAttachmentDescription> make_attachments(memory_pool_t& pool, std::span<const attachment_description> attachments)
{
    auto output{make_stack_vector<VkAttachmentDescription>(pool)};
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

static stack_vector_t<VkAttachmentReference> convert_attachments(memory_pool_t& pool, std::span<const attachment_reference> attachments)
{
    auto output{make_stack_vector<VkAttachmentReference>(pool)};
    output.reserve(std::size(attachments));

    for(auto&& attachment : attachments)
    {
        output.emplace_back(convert_attachment(attachment));
    }

    return output;
}

struct subpass_data
{
    stack_vector_t<VkAttachmentReference> input_attachments{};
    stack_vector_t<VkAttachmentReference> color_attachments{};
    stack_vector_t<VkAttachmentReference> resolve_attachments{};
    std::optional<VkAttachmentReference> depth_attachment{};
    const std::vector<std::uint32_t>* preserve_attachments{};
};

static stack_vector_t<subpass_data> make_subpasses_data(memory_pool_t& pool, std::span<const subpass_description> subpasses)
{
    auto output{make_stack_vector<subpass_data>(pool)};
    output.reserve(std::size(subpasses));

    for(auto&& subpass : subpasses)
    {
        auto& native_subpass{output.emplace_back()};

        native_subpass.input_attachments = convert_attachments(pool, subpass.input_attachments);
        native_subpass.color_attachments = convert_attachments(pool, subpass.color_attachments);
        native_subpass.resolve_attachments = convert_attachments(pool, subpass.resolve_attachments);

        if(subpass.depth_attachment.has_value())
        {
            native_subpass.depth_attachment = convert_attachment(subpass.depth_attachment.value());
        }

        native_subpass.preserve_attachments = &subpass.preserve_attachments;
    }

    return output;
}

static stack_vector_t<VkSubpassDescription> make_subpasses(memory_pool_t& pool, std::span<const subpass_data> subpasses_data)
{
    auto output{make_stack_vector<VkSubpassDescription>(pool)};
    output.reserve(std::size(subpasses_data));

    for(auto&& data : subpasses_data)
    {
        auto& native_subpass{output.emplace_back()};

        native_subpass.inputAttachmentCount = static_cast<std::uint32_t>(std::size(data.input_attachments));
        native_subpass.pInputAttachments = std::data(data.input_attachments);
        native_subpass.colorAttachmentCount = static_cast<std::uint32_t>(std::size(data.color_attachments));
        native_subpass.pColorAttachments = std::data(data.color_attachments);
        native_subpass.pResolveAttachments = std::data(data.resolve_attachments);
        native_subpass.preserveAttachmentCount = static_cast<std::uint32_t>(std::size(*data.preserve_attachments));
        native_subpass.pPreserveAttachments = std::data(*data.preserve_attachments);

        if(data.depth_attachment.has_value())
        {
            native_subpass.pDepthStencilAttachment = &data.depth_attachment.value();
        }
    }

    return output;
}

static stack_vector_t<VkSubpassDependency> make_dependencies(memory_pool_t& pool, std::span<const subpass_dependency> dependencies)
{
    auto output{make_stack_vector<VkSubpassDependency>(pool)};
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
{
    memory_pool_t pool{};

    const auto attachments{make_attachments(pool, info.attachments)};
    const auto subpasses_data{make_subpasses_data(pool, info.subpasses)};
    const auto subpass{make_subpasses(pool, subpasses_data)};
    const auto dependencies{make_dependencies(pool, info.dependencies)};

    m_render_pass = vulkan::render_pass{underlying_cast<VkDevice>(renderer), attachments, subpass, dependencies};
}

}
