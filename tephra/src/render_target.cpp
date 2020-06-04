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

static VkExtent2D choose_extent(const VkSurfaceCapabilitiesKHR& capabilities) noexcept
{
    if(capabilities.currentExtent.width == 0xFFFFFFFF || capabilities.currentExtent.height == 0xFFFFFFFF)
        return capabilities.maxImageExtent;

    return capabilities.currentExtent;
}

static VkSurfaceFormatKHR choose_format(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    std::vector<VkSurfaceFormatKHR> formats{};

    std::uint32_t format_count{};
    if(auto result{vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr)}; result != VK_SUCCESS)
        throw vulkan::error{result};

    formats.resize(format_count);
    if(auto result{vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, std::data(formats))}; result != VK_SUCCESS)
        throw vulkan::error{result};

    if(std::size(formats) == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
    {
        return {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    for(auto&& format : formats)
    {
        if(format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }

        if(format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    for(auto&& format : formats)
    {
        if(format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }

        if(format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    return formats[0];
}

static VkSurfaceCapabilitiesKHR get_surface_capabilities(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    VkSurfaceCapabilitiesKHR capabilities{};
    if(auto result{vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities)}; result != VK_SUCCESS)
        throw vulkan::error{result};

    return capabilities;
}

render_target::render_target(renderer& renderer, tph::texture& texture, render_target_options options, tph::sample_count sample_count)
:m_offscreen_target{std::make_unique<offscreen_target>()}
{
    assert(texture.aspect() == texture_aspect::color && "High level tph::render_target contructor called with non color target.");

    m_offscreen_target->physical_device = underlying_cast<VkPhysicalDevice>(renderer);
    m_offscreen_target->device = underlying_cast<VkDevice>(renderer);
    m_offscreen_target->allocator = &renderer.allocator();
    m_offscreen_target->graphics_family = renderer.queue_family_index(queue::graphics);

    m_offscreen_target->options = options;
    m_offscreen_target->sample_count = static_cast<VkSampleCountFlagBits>(sample_count);
    m_offscreen_target->extent = VkExtent2D{texture.width(), texture.height()};
    m_offscreen_target->format = static_cast<VkFormat>(texture.format());
    m_offscreen_target->texture = underlying_cast<VkImage>(texture);
    m_offscreen_target->texture_view = underlying_cast<VkImageView>(texture);
    m_offscreen_target->has_sampling = underlying_cast<VkSampler>(texture);

    build_offscreen_target_depth_images();
    build_offscreen_target_multisampling_images();
    build_offscreen_target_render_pass();
    build_offscreen_target_render_pass_data();

    texture.m_layout = m_offscreen_target->has_sampling ? tph::texture_layout::shader_read_only_optimal :  tph::texture_layout::transfer_source_optimal;
}

void render_target::build_offscreen_target_depth_images()
{
    if(static_cast<bool>(m_offscreen_target->options & render_target_options::depth_buffering))
    {
        m_offscreen_target->depth_format = vulkan::find_format(m_offscreen_target->physical_device, {VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        m_offscreen_target->depth_image = vulkan::image{m_offscreen_target->device, m_offscreen_target->extent, VK_IMAGE_TYPE_2D, m_offscreen_target->depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, m_offscreen_target->sample_count};
        m_offscreen_target->depth_image_memory = m_offscreen_target->allocator->allocate_bound(m_offscreen_target->depth_image, vulkan::memory_resource_type::non_linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        m_offscreen_target->depth_image_view = vulkan::image_view{m_offscreen_target->device, m_offscreen_target->depth_image, VK_IMAGE_VIEW_TYPE_2D, m_offscreen_target->depth_format, VK_IMAGE_ASPECT_DEPTH_BIT};
    }
}

void render_target::build_offscreen_target_multisampling_images()
{
    if(m_offscreen_target->sample_count != VK_SAMPLE_COUNT_1_BIT)
    {
        m_offscreen_target->multisampling_image = vulkan::image{m_offscreen_target->device, m_offscreen_target->extent, VK_IMAGE_TYPE_2D, m_offscreen_target->format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, m_offscreen_target->sample_count};
        m_offscreen_target->multisampling_image_memory = m_offscreen_target->allocator->allocate_bound(m_offscreen_target->multisampling_image, vulkan::memory_resource_type::non_linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        m_offscreen_target->multisampling_image_view = vulkan::image_view{m_offscreen_target->device, m_offscreen_target->multisampling_image, VK_IMAGE_VIEW_TYPE_2D,  VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT};
    }
}

void render_target::build_offscreen_target_render_pass()
{
    std::vector<VkAttachmentDescription> attachments{};
    std::vector<VkAttachmentReference> attachment_references{};

    attachments.emplace_back();
    attachments.back().format = m_offscreen_target->format;
    attachments.back().samples = m_offscreen_target->sample_count;
    attachments.back().loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments.back().storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments.back().stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments.back().stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments.back().initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if(m_offscreen_target->sample_count != VK_SAMPLE_COUNT_1_BIT)
    {
        attachments.back().finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    else
    {
        attachments.back().finalLayout = m_offscreen_target->has_sampling ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    }

    attachment_references.emplace_back();
    attachment_references.back().attachment = static_cast<std::uint32_t>(std::size(attachments) - 1);
    attachment_references.back().layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    if(static_cast<bool>(m_offscreen_target->options & render_target_options::depth_buffering))
    {
        attachments.emplace_back();
        attachments.back().format = m_offscreen_target->depth_format;
        attachments.back().samples = m_offscreen_target->sample_count;
        attachments.back().loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments.back().storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments.back().stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments.back().stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments.back().initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments.back().finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachment_references.emplace_back();
        attachment_references.back().attachment = static_cast<std::uint32_t>(std::size(attachments) - 1);
        attachment_references.back().layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    if(m_offscreen_target->sample_count != VK_SAMPLE_COUNT_1_BIT)
    {
        attachments.emplace_back();
        attachments.back().format = m_offscreen_target->format;
        attachments.back().samples = VK_SAMPLE_COUNT_1_BIT;
        attachments.back().loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments.back().storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments.back().stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments.back().stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments.back().initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments.back().finalLayout = m_offscreen_target->has_sampling ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        attachment_references.emplace_back();
        attachment_references.back().attachment = static_cast<std::uint32_t>(std::size(attachments) - 1);
        attachment_references.back().layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    std::vector<VkSubpassDescription> subpasses{};

    subpasses.emplace_back();
    subpasses.back().pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses.back().colorAttachmentCount = 1;
    subpasses.back().pColorAttachments = &attachment_references[0];

    if(static_cast<bool>(m_offscreen_target->options & render_target_options::depth_buffering))
    {
        subpasses.back().pDepthStencilAttachment = &attachment_references[1];

        if(m_offscreen_target->sample_count != VK_SAMPLE_COUNT_1_BIT)
        {
            subpasses.back().pResolveAttachments = &attachment_references[2];
        }
    }
    else if(m_offscreen_target->sample_count != VK_SAMPLE_COUNT_1_BIT)
    {
        subpasses.back().pResolveAttachments = &attachment_references[1];
    }

    std::vector<VkSubpassDependency> dependencies{};

    dependencies.emplace_back();
    dependencies.back().srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies.back().dstSubpass = 0;
    dependencies.back().srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies.back().dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies.back().srcAccessMask = 0;
    dependencies.back().dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    m_offscreen_target->render_pass = vulkan::render_pass{m_offscreen_target->device, attachments, subpasses, dependencies};
}

void render_target::build_offscreen_target_render_pass_data()
{
    std::vector<VkImageView> attachments{};

    if(m_offscreen_target->sample_count != VK_SAMPLE_COUNT_1_BIT)
    {
        attachments.push_back(m_offscreen_target->multisampling_image_view);

        if(static_cast<bool>(m_offscreen_target->options & render_target_options::depth_buffering))
            attachments.push_back(m_offscreen_target->depth_image_view);

        attachments.push_back(m_offscreen_target->texture_view);
    }
    else
    {
        attachments.push_back(m_offscreen_target->texture_view);

        if(static_cast<bool>(m_offscreen_target->options & render_target_options::depth_buffering))
            attachments.push_back(m_offscreen_target->depth_image_view);
    }

    m_offscreen_target->framebuffer = vulkan::framebuffer{m_offscreen_target->device, m_offscreen_target->render_pass, attachments, m_offscreen_target->extent};
}

render_target::render_target(renderer& renderer, surface& surface, present_mode mode, std::uint32_t image_count, render_target_options options, tph::sample_count sample_count)
:m_surface_target{std::make_unique<surface_target>()}
{
    m_surface_target->physical_device = underlying_cast<VkPhysicalDevice>(renderer);
    m_surface_target->device = underlying_cast<VkDevice>(renderer);
    m_surface_target->surface = underlying_cast<VkSurfaceKHR>(surface);
    m_surface_target->allocator = &renderer.allocator();
    m_surface_target->graphics_family = renderer.queue_family_index(queue::graphics);
    m_surface_target->present_queue = underlying_cast<VkQueue>(renderer, queue::present);

    m_surface_target->options = options;
    m_surface_target->image_count = image_count;
    m_surface_target->present_mode = static_cast<VkPresentModeKHR>(mode);
    m_surface_target->sample_count = static_cast<VkSampleCountFlagBits>(sample_count);

    build_surface_target_swap_chain();
    build_surface_target_depth_images();
    build_surface_target_multisampling_images();
    build_surface_target_render_pass();
    build_surface_target_render_pass_data();
}

void render_target::build_surface_target_swap_chain()
{
    m_surface_target->surface_capabilities = get_surface_capabilities(m_surface_target->physical_device, m_surface_target->surface);
    m_surface_target->swapchain_format = choose_format(m_surface_target->physical_device, m_surface_target->surface);
    m_surface_target->swapchain_extent = choose_extent(m_surface_target->surface_capabilities);

    assert((m_surface_target->swapchain_extent.width != 0 && m_surface_target->swapchain_extent.height != 0) && "Trying to create a swapchain with 0 pixels of width or height.");

    vulkan::swapchain old_swapchain{std::move(m_surface_target->swapchain)};

    m_surface_target->swapchain = vulkan::swapchain
    {
        m_surface_target->device,
        m_surface_target->surface,
        m_surface_target->swapchain_extent,
        m_surface_target->image_count,
        m_surface_target->swapchain_format,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        std::vector<std::uint32_t>{},
        m_surface_target->surface_capabilities.currentTransform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        m_surface_target->present_mode,
        static_cast<VkBool32>(static_cast<bool>(m_surface_target->options & render_target_options::clipping)),
        old_swapchain
    };

    if(auto result{vkGetSwapchainImagesKHR(m_surface_target->device, m_surface_target->swapchain, &m_surface_target->image_count, nullptr)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

void render_target::build_surface_target_depth_images()
{
    if(static_cast<bool>(m_surface_target->options & render_target_options::depth_buffering))
    {
        m_surface_target->depth_format = vulkan::find_format(m_surface_target->physical_device, {VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        m_surface_target->depth_image = vulkan::image{m_surface_target->device, m_surface_target->swapchain_extent, VK_IMAGE_TYPE_2D, m_surface_target->depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, m_surface_target->sample_count};
        m_surface_target->depth_image_memory = m_surface_target->allocator->allocate_bound(m_surface_target->depth_image, vulkan::memory_resource_type::non_linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        m_surface_target->depth_image_view = vulkan::image_view{m_surface_target->device, m_surface_target->depth_image, VK_IMAGE_VIEW_TYPE_2D, m_surface_target->depth_format, VK_IMAGE_ASPECT_DEPTH_BIT};
    }
}

void render_target::build_surface_target_multisampling_images()
{
    if(m_surface_target->sample_count != VK_SAMPLE_COUNT_1_BIT)
    {
        m_surface_target->multisampling_image = vulkan::image{m_surface_target->device, m_surface_target->swapchain_extent, VK_IMAGE_TYPE_2D, m_surface_target->swapchain_format.format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, m_surface_target->sample_count};
        m_surface_target->multisampling_image_memory = m_surface_target->allocator->allocate_bound(m_surface_target->multisampling_image, vulkan::memory_resource_type::non_linear, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        m_surface_target->multisampling_image_view = vulkan::image_view{m_surface_target->device, m_surface_target->multisampling_image, VK_IMAGE_VIEW_TYPE_2D,  m_surface_target->swapchain_format.format, VK_IMAGE_ASPECT_COLOR_BIT};
    }
}

void render_target::build_surface_target_render_pass()
{
    std::vector<VkAttachmentDescription> attachments{};
    std::vector<VkAttachmentReference> attachment_references{};

    attachments.emplace_back();
    attachments.back().format = m_surface_target->swapchain_format.format;
    attachments.back().samples = m_surface_target->sample_count;
    attachments.back().loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments.back().storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments.back().stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments.back().stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments.back().initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments.back().finalLayout = m_surface_target->sample_count == VK_SAMPLE_COUNT_1_BIT ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachment_references.emplace_back();
    attachment_references.back().attachment = static_cast<std::uint32_t>(std::size(attachments) - 1);
    attachment_references.back().layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    if(static_cast<bool>(m_surface_target->options & render_target_options::depth_buffering))
    {
        attachments.emplace_back();
        attachments.back().format = m_surface_target->depth_format;
        attachments.back().samples = m_surface_target->sample_count;
        attachments.back().loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments.back().storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments.back().stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments.back().stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments.back().initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments.back().finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachment_references.emplace_back();
        attachment_references.back().attachment = static_cast<std::uint32_t>(std::size(attachments) - 1);
        attachment_references.back().layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    if(m_surface_target->sample_count != VK_SAMPLE_COUNT_1_BIT)
    {
        attachments.emplace_back();
        attachments.back().format = m_surface_target->swapchain_format.format;
        attachments.back().samples = VK_SAMPLE_COUNT_1_BIT;
        attachments.back().loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments.back().storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments.back().stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments.back().stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments.back().initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments.back().finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        attachment_references.emplace_back();
        attachment_references.back().attachment = static_cast<std::uint32_t>(std::size(attachments) - 1);
        attachment_references.back().layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    std::vector<VkSubpassDescription> subpasses{};

    subpasses.emplace_back();
    subpasses.back().pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses.back().colorAttachmentCount = 1;
    subpasses.back().pColorAttachments = &attachment_references[0];

    if(static_cast<bool>(m_surface_target->options & render_target_options::depth_buffering))
    {
        subpasses.back().pDepthStencilAttachment = &attachment_references[1];

        if(m_surface_target->sample_count != VK_SAMPLE_COUNT_1_BIT)
        {
            subpasses.back().pResolveAttachments = &attachment_references[2];
        }
    }
    else if(m_surface_target->sample_count != VK_SAMPLE_COUNT_1_BIT)
    {
        subpasses.back().pResolveAttachments = &attachment_references[1];
    }

    std::vector<VkSubpassDependency> dependencies{};

    dependencies.emplace_back();
    dependencies.back().srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies.back().dstSubpass = 0;
    dependencies.back().srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies.back().dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies.back().srcAccessMask = 0;
    dependencies.back().dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    m_surface_target->render_pass = vulkan::render_pass{m_surface_target->device, attachments, subpasses, dependencies};
}

void render_target::build_surface_target_render_pass_data()
{
    std::vector<VkImage> images{};
    images.resize(m_surface_target->image_count);

    if(auto result{vkGetSwapchainImagesKHR(m_surface_target->device, m_surface_target->swapchain, &m_surface_target->image_count, std::data(images))}; result != VK_SUCCESS)
        throw vulkan::error{result};

    m_surface_target->render_pass_data.resize(m_surface_target->image_count);
    for(std::uint32_t i{}; i < m_surface_target->image_count; ++i)
    {
        auto& data{m_surface_target->render_pass_data[i]};

        data.swapchain_image = images[i];
        data.swapchain_image_view = vulkan::image_view{m_surface_target->device, images[i], VK_IMAGE_VIEW_TYPE_2D, m_surface_target->swapchain_format.format, VK_IMAGE_ASPECT_COLOR_BIT};

        std::vector<VkImageView> attachments{};

        if(m_surface_target->sample_count != VK_SAMPLE_COUNT_1_BIT)
        {
            attachments.push_back(m_surface_target->multisampling_image_view);

            if(static_cast<bool>(m_surface_target->options & render_target_options::depth_buffering))
                attachments.push_back(m_surface_target->depth_image_view);

            attachments.push_back(data.swapchain_image_view);
        }
        else
        {
            attachments.push_back(data.swapchain_image_view);

            if(static_cast<bool>(m_surface_target->options & render_target_options::depth_buffering))
                attachments.push_back(m_surface_target->depth_image_view);
        }

        data.framebuffer = vulkan::framebuffer{m_surface_target->device, m_surface_target->render_pass, attachments, m_surface_target->swapchain_extent};
    }
}

void render_target::set_clear_color_value(float red, float green, float blue, float alpha) noexcept
{
    assert((m_offscreen_target || m_surface_target) && "tph::render_target::set_clear_color_value called with invalid render_target.");

    if(m_offscreen_target)
    {
        m_offscreen_target->clear_color = {{red, green, blue, alpha}};
    }
    else
    {
        m_surface_target->clear_color = {{red, green, blue, alpha}};
    }
}

void render_target::set_clear_depth_stencil_value(float depth, std::uint32_t stencil) noexcept
{
    assert((m_offscreen_target || m_surface_target) && "tph::render_target::set_clear_depth_stencil_value called with invalid render_target.");

    if(m_offscreen_target)
    {
        m_offscreen_target->clear_depth = {depth, stencil};
    }
    else
    {
        m_surface_target->clear_depth = {depth, stencil};
    }
}

void render_target::begin(command_buffer& buffer, std::uint32_t image_index, render_pass_content content)
{
    assert((m_offscreen_target || m_surface_target) && "tph::render_target::begin called with invalid render_target.");

    if(m_offscreen_target)
    {
        const bool has_depth{static_cast<bool>(m_offscreen_target->options & render_target_options::depth_buffering)};
        const bool has_multisampling{m_offscreen_target->sample_count != VK_SAMPLE_COUNT_1_BIT};

        std::vector<VkClearValue> clear_values{};

        clear_values.emplace_back().color = m_offscreen_target->clear_color;

        if(has_depth)
            clear_values.emplace_back().depthStencil = m_offscreen_target->clear_depth;

        if(has_multisampling)
            clear_values.emplace_back().color = m_offscreen_target->clear_color;

        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = m_offscreen_target->render_pass;
        render_pass_info.framebuffer = m_offscreen_target->framebuffer;
        render_pass_info.renderArea.extent = m_offscreen_target->extent;
        render_pass_info.clearValueCount = static_cast<std::uint32_t>(std::size(clear_values));
        render_pass_info.pClearValues = std::data(clear_values);

        vkCmdBeginRenderPass(underlying_cast<VkCommandBuffer>(buffer), &render_pass_info, static_cast<VkSubpassContents>(content));
    }
    else
    {
        const bool has_depth{static_cast<bool>(m_surface_target->options & render_target_options::depth_buffering)};
        const bool has_multisampling{m_surface_target->sample_count != VK_SAMPLE_COUNT_1_BIT};

        auto& data{m_surface_target->render_pass_data[image_index]};

        std::vector<VkClearValue> clear_values{};

        clear_values.emplace_back().color = m_surface_target->clear_color;

        if(has_depth)
            clear_values.emplace_back().depthStencil = m_surface_target->clear_depth;

        if(has_multisampling)
            clear_values.emplace_back().color = m_surface_target->clear_color;

        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = m_surface_target->render_pass;
        render_pass_info.framebuffer = data.framebuffer;
        render_pass_info.renderArea.extent = m_surface_target->swapchain_extent;
        render_pass_info.clearValueCount = static_cast<std::uint32_t>(std::size(clear_values));
        render_pass_info.pClearValues = std::data(clear_values);

        vkCmdBeginRenderPass(underlying_cast<VkCommandBuffer>(buffer), &render_pass_info, static_cast<VkSubpassContents>(content));
    }
}

std::uint32_t render_target::image_index() const noexcept
{
    assert((m_offscreen_target || m_surface_target) && "tph::render_target::image_index called with invalid render_target.");

    if(m_offscreen_target)
    {
        return 0;
    }
    else
    {
        return m_surface_target->image_index;
    }
}

sample_count render_target::sample_count(std::uint32_t subpass [[maybe_unused]]) const noexcept
{
    assert((m_offscreen_target || m_surface_target) && "tph::render_target::sample_count called with invalid render_target.");

    if(m_offscreen_target)
    {
        return static_cast<tph::sample_count>(m_offscreen_target->sample_count);
    }
    else
    {
        return static_cast<tph::sample_count>(m_surface_target->sample_count);
    }
}

render_target_status render_target::acquire(optional_ref<semaphore> semaphore, optional_ref<fence> fence)
{
    assert(m_surface_target && "tph::render_target::acquire called with invalid render_target.");

    VkSemaphore native_semaphore{semaphore.has_value() ? underlying_cast<VkSemaphore>(*semaphore) : VkSemaphore{}};
    VkFence native_fence{fence.has_value() ? underlying_cast<VkFence>(*fence) : VkFence{}};

    const auto result{vkAcquireNextImageKHR(m_surface_target->device, m_surface_target->swapchain, std::numeric_limits<std::uint64_t>::max(), native_semaphore, native_fence, &m_surface_target->image_index)};

    if(result == VK_SUBOPTIMAL_KHR)
        return render_target_status::suboptimal;

    if(result == VK_ERROR_OUT_OF_DATE_KHR)
        return render_target_status::out_of_date;

    if(result == VK_ERROR_SURFACE_LOST_KHR)
        return render_target_status::surface_lost;

    if(result != VK_SUCCESS)
        throw vulkan::error{result};

    return render_target_status::valid;
}

render_target_status render_target::present(const std::vector<std::reference_wrapper<semaphore>>& wait_semaphores)
{
    assert(m_surface_target && "tph::render_target::present called with invalid render_target.");

    VkSwapchainKHR native_swapchain{m_surface_target->swapchain};

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &native_swapchain;
    present_info.pImageIndices = &m_surface_target->image_index;

    std::vector<VkSemaphore> native_semaphores{};
    native_semaphores.reserve(std::size(wait_semaphores));
    for(semaphore& semaphore : wait_semaphores)
        native_semaphores.push_back(underlying_cast<VkSemaphore>(semaphore));

    present_info.waitSemaphoreCount = static_cast<std::uint32_t>(std::size(native_semaphores));
    present_info.pWaitSemaphores = std::data(native_semaphores);

    const auto result{vkQueuePresentKHR(m_surface_target->present_queue, &present_info)};

    if(result == VK_SUBOPTIMAL_KHR)
        return render_target_status::suboptimal;

    if(result == VK_ERROR_OUT_OF_DATE_KHR)
        return render_target_status::out_of_date;

    if(result == VK_ERROR_SURFACE_LOST_KHR)
        return render_target_status::surface_lost;

    if(result != VK_SUCCESS)
        throw vulkan::error{result};

    return render_target_status::valid;
}

render_target_status render_target::present(semaphore& wait_semaphore)
{
    return present(std::vector<std::reference_wrapper<semaphore>>{wait_semaphore});
}

void render_target::recreate()
{
    assert((m_surface_target || m_offscreen_target) && "tph::render_target::recreate called with invalid render_target.");

    if(m_surface_target)
    {
        const auto old_extent{m_surface_target->swapchain_extent};
        const auto old_format{m_surface_target->swapchain_format};

        build_surface_target_swap_chain();

        const bool extent_changed{old_extent.width != m_surface_target->swapchain_extent.width || old_extent.height != m_surface_target->swapchain_extent.height};
        const bool format_changed{old_format.format != m_surface_target->swapchain_format.format};

        if(extent_changed)
        {
            build_surface_target_depth_images();
        }

        if(extent_changed || format_changed)
        {
            build_surface_target_multisampling_images();
        }

        if(format_changed)
        {
            build_surface_target_render_pass();
        }

        build_surface_target_render_pass_data();
    }
}

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
        if(static_cast<bool>(aspect_from_format(attachment.format()) & texture_aspect::color))
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
