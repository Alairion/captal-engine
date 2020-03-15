#ifndef TEPHRA_RENDER_TARGET_HPP_INCLUDED
#define TEPHRA_RENDER_TARGET_HPP_INCLUDED

#include "config.hpp"

#include <vector>
#include <memory>
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
class command_buffer;

enum class render_target_options : std::uint32_t
{
    none = 0x00,
    clipping = 0x01,
    depth_buffering = 0x02,
    all = 0xFFFFFFFF
};

template<> struct enable_enum_operations<render_target_options> {static constexpr bool value{true};};

enum class render_pass_content : std::uint32_t
{
    inlined = VK_SUBPASS_CONTENTS_INLINE,
    recorded = VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS,
};

enum class render_target_status : std::uint32_t
{
    valid = 0,
    suboptimal = 1,
    out_of_date = 2,
    surface_lost = 3,
};

class render_target
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

private:
    struct offscreen_target
    {
        //Links
        VkPhysicalDevice physical_device{};
        VkDevice device{};
        vulkan::memory_allocator* allocator{};
        std::uint32_t graphics_family{};

        //User parameters
        render_target_options options{};
        VkSampleCountFlagBits sample_count{};
        VkExtent2D extent{};
        VkImage texture{};
        VkImageView texture_view{};
        bool has_sampling{};

        //Depth buffering
        VkFormat depth_format{};
        vulkan::image depth_image{};
        vulkan::memory_heap_chunk depth_image_memory{};
        vulkan::image_view depth_image_view{};

        //Multisampling
        vulkan::image multisampling_image{};
        vulkan::memory_heap_chunk multisampling_image_memory{};
        vulkan::image_view multisampling_image_view{};

        //Renderpass
        vulkan::render_pass render_pass{};
        vulkan::framebuffer framebuffer{};

        //Dynamic user parameters
        VkClearColorValue clear_color{};
        VkClearDepthStencilValue clear_depth{};
    };

    struct surface_target
    {
        struct render_pass_data
        {
            //Presentation
            VkImage swapchain_image{}; //use plain vulkan object because it is owned by the swapchain
            vulkan::image_view swapchain_image_view{};
            vulkan::framebuffer framebuffer{};
        };

        //Links
        VkPhysicalDevice physical_device{};
        VkDevice device{};
        VkSurfaceKHR surface{};
        vulkan::memory_allocator* allocator{};
        std::uint32_t graphics_family{};
        VkQueue present_queue{};

        //User parameters
        render_target_options options{};
        std::uint32_t image_count{};
        VkPresentModeKHR present_mode{};
        VkSampleCountFlagBits sample_count{};

        //Swapchain
        VkSurfaceCapabilitiesKHR surface_capabilities{};
        VkSurfaceFormatKHR swapchain_format{};
        VkExtent2D swapchain_extent{};
        vulkan::swapchain swapchain{};

        //Depth buffering
        VkFormat depth_format{};
        vulkan::image depth_image{};
        vulkan::memory_heap_chunk depth_image_memory{};
        vulkan::image_view depth_image_view{};

        //Multisampling
        vulkan::image multisampling_image{};
        vulkan::memory_heap_chunk multisampling_image_memory{};
        vulkan::image_view multisampling_image_view{};

        //Renderpass
        vulkan::render_pass render_pass{};
        std::vector<render_pass_data> render_pass_data{};

        //Runtime
        std::uint32_t image_index{};

        //Dynamic user parameters
        VkClearColorValue clear_color{};
        VkClearDepthStencilValue clear_depth{};
    };

public:
    constexpr render_target() = default;
    render_target(renderer& renderer, texture& texture, render_target_options options = render_target_options::none, tph::sample_count sample_count = sample_count::msaa_x1);
    render_target(renderer& renderer, surface& surface, present_mode mode, std::uint32_t image_count, render_target_options options = render_target_options::none, tph::sample_count sample_count = sample_count::msaa_x1);

    ~render_target() = default;
    render_target(const render_target&) = delete;
    render_target& operator=(const render_target&) = delete;
    render_target(render_target&& other) noexcept = default;
    render_target& operator=(render_target&& other) noexcept = default;

public: //Common interface:
    void set_clear_color_value(float red, float green, float blue, float alpha = 1.0f) noexcept;
    void set_clear_depth_stencil_value(float depth, std::uint32_t stencil) noexcept;

    void begin(command_buffer& buffer, std::uint32_t image_index, render_pass_content content = render_pass_content::inlined);

    std::uint32_t image_index() const noexcept;
    sample_count sample_count(std::uint32_t subpass = 0) const noexcept;
    void recreate();

public: //Surface target interface:
    render_target_status acquire(optional_ref<semaphore> semaphore, optional_ref<fence> fence);
    render_target_status present(const std::vector<std::reference_wrapper<semaphore>>& wait_semaphores);
    render_target_status present(semaphore& wait_semaphore);

private:
    void build_offscreen_target_depth_images();
    void build_offscreen_target_multisampling_images();
    void build_offscreen_target_render_pass();
    void build_offscreen_target_render_pass_data();

private:
    void build_surface_target_swap_chain();
    void build_surface_target_depth_images();
    void build_surface_target_multisampling_images();
    void build_surface_target_render_pass();
    void build_surface_target_render_pass_data();

private:
    std::unique_ptr<offscreen_target> m_offscreen_target{};
    std::unique_ptr<surface_target> m_surface_target{};
};

template<>
inline VkRenderPass underlying_cast(const render_target& render_target) noexcept
{
    assert((render_target.m_offscreen_target || render_target.m_surface_target) && "tph::underlying_cast<VkRenderPass>(const tph::render_target&) called with invalid render_target.");

    if(render_target.m_offscreen_target)
        return render_target.m_offscreen_target->render_pass;

    return render_target.m_surface_target->render_pass;
}

template<>
inline VkSwapchainKHR underlying_cast(const render_target& render_target) noexcept
{
    assert(render_target.m_surface_target && "tph::underlying_cast<VkSwapchainKHR>(const tph::render_target&) called with offscreen or invalid render_target.");

    return render_target.m_surface_target->swapchain;
}

template<>
inline VkFramebuffer underlying_cast(const render_target& render_target, const std::size_t& image_index) noexcept
{
    assert((render_target.m_offscreen_target || render_target.m_surface_target) && "tph::underlying_cast<VkRenderPass>(const tph::render_target&) called with invalid render_target.");

    if(render_target.m_offscreen_target)
        return render_target.m_offscreen_target->framebuffer;

    return render_target.m_surface_target->render_pass_data[image_index].framebuffer;
}

}

#endif
