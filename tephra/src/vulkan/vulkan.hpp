#ifndef TEPHRA_VULKAN_HPP_INCLUDED
#define TEPHRA_VULKAN_HPP_INCLUDED

#include "../config.hpp"

#include <string>
#include <vector>
#include <exception>

#include <vulkan/vulkan_core.h>

#ifdef TPH_PLATFORM_ANDROID
    struct ANativeWindow;

    namespace tph::vulkan
    {
        struct android_surface_info
        {
            ANativeWindow* window{};
        };
    }
#endif

#ifdef TPH_PLATFORM_IOS
    namespace tph::vulkan
    {
        struct ios_surface_info
        {
            void* view{};
        };
    }
#endif

#ifdef TPH_PLATFORM_WIN32
    namespace tph::vulkan
    {
        struct win32_surface_info
        {
            void* instance{};
            void* window{};
        };
    }
#endif

#ifdef TPH_PLATFORM_MACOS
    namespace tph::vulkan
    {
        struct macos_surface_info
        {
            void* view{};
        };
    }
#endif

#ifdef TPH_PLATFORM_XLIB
    #include <X11/Xlib.h>

    namespace tph::vulkan
    {
        struct xlib_surface_info
        {
            Display* display{};
            Window window{};
        };
    }
#endif

#ifdef TPH_PLATFORM_XCB
    #include <xcb/xcb.h>

    namespace tph::vulkan
    {
        struct xcb_surface_info
        {
            xcb_connection_t* connection{};
            xcb_window_t window{};
        };
    }
#endif

#ifdef TPH_PLATFORM_WAYLAND
    struct wl_display;
    struct wl_surface;

    namespace tph::vulkan
    {
        struct wayland_surface_info
        {
            wl_display* display{};
            wl_surface* surface{};
        };
    }
#endif

namespace tph::vulkan
{

class error : public std::exception
{
public:
    error() noexcept = default;
    virtual ~error() = default;
    error(const error&) noexcept = default;
    error& operator=(const error&) noexcept = default;
    error(error&& other) noexcept = default;
    error& operator=(error&& other) noexcept = default;

    error(VkResult result) noexcept
    :m_result{result}
    {

    }

    VkResult error_code() const noexcept
    {
        return m_result;
    }

    const char* error_name() const noexcept;
    const char* error_description() const noexcept;
    const char* what() const noexcept override;

private:
    VkResult m_result{};
};

class instance
{
public:
    constexpr instance() = default;
    explicit instance(const std::string& application_name, tph::version version, const std::vector<const char*>& extensions, const std::vector<const char*>& layers);
    ~instance();

    instance(const instance&) = delete;
    instance& operator=(const instance&) = delete;

    instance(instance&& other) noexcept;
    instance& operator=(instance&& other) noexcept;

    explicit instance(VkInstance instance) noexcept
    :m_instance{instance}
    {

    }

    operator VkInstance() const noexcept
    {
        return m_instance;
    }

private:
    VkInstance m_instance{};
};

class device
{
public:
    constexpr device() = default;
    explicit device(VkPhysicalDevice physical_device, const std::vector<const char*>& extensions, const std::vector<const char*>& layers, const std::vector<VkDeviceQueueCreateInfo>& queues, const VkPhysicalDeviceFeatures& features);
    ~device();

    device(const device&) = delete;
    device& operator=(const device&) = delete;

    device(device&& other) noexcept;
    device& operator=(device&& other) noexcept;

    explicit device(VkDevice device) noexcept
    :m_device{device}
    {

    }

    operator VkDevice() const noexcept
    {
        return m_device;
    }

private:
    VkDevice m_device{};
};

class device_memory
{
public:
    constexpr device_memory() = default;
    device_memory(VkDevice device, std::uint32_t memory_type, std::uint64_t size);
    ~device_memory();

    device_memory(const device_memory&) = delete;
    device_memory& operator=(const device_memory&) = delete;

    device_memory(device_memory&& other) noexcept;
    device_memory& operator=(device_memory&& other) noexcept;

    explicit device_memory(VkDevice device, VkDeviceMemory device_memory) noexcept
    :m_device{device}
    ,m_device_memory{device_memory}
    {

    }

    operator VkDeviceMemory() const noexcept
    {
        return m_device_memory;
    }

private:
    VkDevice m_device{};
    VkDeviceMemory m_device_memory{};
};

class buffer
{
public:
    constexpr buffer() = default;
    buffer(VkDevice device, uint64_t size, VkBufferUsageFlags usage);
    ~buffer();

    buffer(const buffer&) = delete;
    buffer& operator=(const buffer&) = delete;

    buffer(buffer&& other) noexcept;
    buffer& operator=(buffer&& other) noexcept;

    explicit buffer(VkDevice device, VkBuffer buffer) noexcept
    :m_device{device}
    ,m_buffer{buffer}
    {

    }

    operator VkBuffer() const noexcept
    {
        return m_buffer;
    }

private:
    VkDevice m_device{};
    VkBuffer m_buffer{};
};

class buffer_view
{
public:
    constexpr buffer_view() = default;
    buffer_view(VkDevice device, VkBuffer buffer, VkFormat format, std::uint64_t offset = 0, std::uint64_t size = VK_WHOLE_SIZE);
    ~buffer_view();

    buffer_view(const buffer_view&) = delete;
    buffer_view& operator=(const buffer_view&) = delete;

    buffer_view(buffer_view&& other) noexcept;
    buffer_view& operator=(buffer_view&& other) noexcept;

    explicit buffer_view(VkDevice device, VkBufferView buffer_view) noexcept
    :m_device{device}
    ,m_buffer_view{buffer_view}
    {

    }

    operator VkBufferView() const noexcept
    {
        return m_buffer_view;
    }

private:
    VkDevice m_device{};
    VkBufferView m_buffer_view{};
};

class image
{
public:
    constexpr image() = default;
    image(VkDevice device, VkExtent3D size, VkImageType type, VkFormat format, VkImageUsageFlags usage, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
    image(VkDevice device, VkExtent2D size, VkImageType type, VkFormat format, VkImageUsageFlags usage, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
    ~image();

    image(const image&) = delete;
    image& operator=(const image&) = delete;

    image(image&& other) noexcept;
    image& operator=(image&& other) noexcept;

    explicit image(VkDevice device, VkImage image) noexcept
    :m_device{device}
    ,m_image{image}
    {

    }

    explicit image(VkImage image) noexcept //Non-owning (useful for swapchain images)
    :m_image{image}
    {

    }

    operator VkImage() const noexcept
    {
        return m_image;
    }

private:
    VkDevice m_device{};
    VkImage m_image{};
};

class image_view
{
public:
    constexpr image_view() = default;
    image_view(VkDevice device, VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspect);
    ~image_view();

    image_view(const image_view&) = delete;
    image_view& operator=(const image_view&) = delete;

    image_view(image_view&& other) noexcept;
    image_view& operator=(image_view&& other) noexcept;

    explicit image_view(VkDevice device, VkImageView image_view) noexcept
    :m_device{device}
    ,m_image_view{image_view}
    {

    }

    operator VkImageView() const noexcept
    {
        return m_image_view;
    }

private:
    VkDevice m_device{};
    VkImageView m_image_view{};
};

class sampler
{
public:
    constexpr sampler() = default;
    sampler(VkDevice device, VkFilter mag_filter, VkFilter min_filter, VkSamplerAddressMode address_mode, VkBool32 compared, VkCompareOp compare_op, VkBool32 unnormalized, float anisotropy = 1.0f);
    ~sampler();

    sampler(const sampler&) = delete;
    sampler& operator=(const sampler&) = delete;

    sampler(sampler&& other) noexcept;
    sampler& operator=(sampler&& other) noexcept;

    explicit sampler(VkDevice device, VkSampler sampler) noexcept
    :m_device{device}
    ,m_sampler{sampler}
    {

    }

    operator VkSampler() const noexcept
    {
        return m_sampler;
    }

private:
    VkDevice m_device{};
    VkSampler m_sampler{};
};

class framebuffer
{
public:
    constexpr framebuffer() = default;
    framebuffer(VkDevice device, VkRenderPass render_pass, const std::vector<VkImageView>& attachments, VkExtent2D size, std::uint32_t layers = 1);
    ~framebuffer();

    framebuffer(const framebuffer&) = delete;
    framebuffer& operator=(const framebuffer&) = delete;

    framebuffer(framebuffer&& other) noexcept;
    framebuffer& operator=(framebuffer&& other) noexcept;

    explicit framebuffer(VkDevice device, VkFramebuffer framebuffer) noexcept
    :m_device{device}
    ,m_framebuffer{framebuffer}
    {

    }

    operator VkFramebuffer() const noexcept
    {
        return m_framebuffer;
    }

private:
    VkDevice m_device{};
    VkFramebuffer m_framebuffer{};
};

class shader
{
public:
    constexpr shader() = default;
    shader(VkDevice device, std::size_t size, const std::uint32_t* code);
    ~shader();

    shader(const shader&) = delete;
    shader& operator=(const shader&) = delete;

    shader(shader&& other) noexcept;
    shader& operator=(shader&& other) noexcept;

    explicit shader(VkDevice device, VkShaderModule shader) noexcept
    :m_device{device}
    ,m_shader{shader}
    {

    }

    operator VkShaderModule() const noexcept
    {
        return m_shader;
    }

private:
    VkDevice m_device{};
    VkShaderModule m_shader{};
};

class semaphore
{
public:
    constexpr semaphore() = default;
    semaphore(VkDevice device);
    ~semaphore();

    semaphore(const semaphore&) = delete;
    semaphore& operator=(const semaphore&) = delete;

    semaphore(semaphore&& other) noexcept;
    semaphore& operator=(semaphore&& other) noexcept;

    explicit semaphore(VkDevice device, VkSemaphore semaphore) noexcept
    :m_device{device}
    ,m_semaphore{semaphore}
    {

    }

    operator VkSemaphore() const noexcept
    {
        return m_semaphore;
    }

private:
    VkDevice m_device{};
    VkSemaphore m_semaphore{};
};

class fence
{
public:
    constexpr fence() = default;
    fence(VkDevice device, VkFenceCreateFlags flags = 0);
    ~fence();

    fence(const fence&) = delete;
    fence& operator=(const fence&) = delete;

    fence(fence&& other) noexcept;
    fence& operator=(fence&& other) noexcept;

    explicit fence(VkDevice device, VkFence fence) noexcept
    :m_device{device}
    ,m_fence{fence}
    {

    }

    operator VkFence() const noexcept
    {
        return m_fence;
    }

private:
    VkDevice m_device{};
    VkFence m_fence{};
};

class event
{
public:
    constexpr event() = default;
    event(VkDevice device);
    ~event();

    event(const event&) = delete;
    event& operator=(const event&) = delete;

    event(event&& other) noexcept;
    event& operator=(event&& other) noexcept;

    explicit event(VkDevice device, VkEvent event) noexcept
    :m_device{device}
    ,m_event{event}
    {

    }

    operator VkEvent() const noexcept
    {
        return m_event;
    }

private:
    VkDevice m_device{};
    VkEvent m_event{};
};

class command_pool
{
public:
    constexpr command_pool() = default;
    command_pool(VkDevice device, std::uint32_t queue_family, VkCommandPoolCreateFlags flags = 0);
    ~command_pool();

    command_pool(const command_pool&) = delete;
    command_pool& operator=(const command_pool&) = delete;

    command_pool(command_pool&& other) noexcept;
    command_pool& operator=(command_pool&& other) noexcept;

    explicit command_pool(VkDevice device, VkCommandPool command_pool) noexcept
    :m_device{device}
    ,m_command_pool{command_pool}
    {

    }

    operator VkCommandPool() const noexcept
    {
        return m_command_pool;
    }

private:
    VkDevice m_device{};
    VkCommandPool m_command_pool{};
};

class command_buffer
{
public:
    constexpr command_buffer() = default;
    command_buffer(VkDevice device, VkCommandPool command_pool, VkCommandBufferLevel level);
    ~command_buffer();

    command_buffer(const command_buffer&) = delete;
    command_buffer& operator=(const command_buffer&) = delete;

    command_buffer(command_buffer&& other) noexcept;
    command_buffer& operator=(command_buffer&& other) noexcept;

    explicit command_buffer(VkDevice device, VkCommandPool command_pool, VkCommandBuffer command_buffer) noexcept
    :m_device{device}
    ,m_command_pool{command_pool}
    ,m_command_buffer{command_buffer}
    {

    }

    operator VkCommandBuffer() const noexcept
    {
        return m_command_buffer;
    }

private:
    VkDevice m_device{};
    VkCommandPool m_command_pool{};
    VkCommandBuffer m_command_buffer{};
};

class descriptor_set_layout
{
public:
    constexpr descriptor_set_layout() = default;
    descriptor_set_layout(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
    ~descriptor_set_layout();

    descriptor_set_layout(const descriptor_set_layout&) = delete;
    descriptor_set_layout& operator=(const descriptor_set_layout&) = delete;

    descriptor_set_layout(descriptor_set_layout&& other) noexcept;
    descriptor_set_layout& operator=(descriptor_set_layout&& other) noexcept;

    explicit descriptor_set_layout(VkDevice device, VkDescriptorSetLayout descriptor_set_layout) noexcept
    :m_device{device}
    ,m_descriptor_set_layout{descriptor_set_layout}
    {

    }

    operator VkDescriptorSetLayout() const noexcept
    {
        return m_descriptor_set_layout;
    }

private:
    VkDevice m_device{};
    VkDescriptorSetLayout m_descriptor_set_layout{};
};

class descriptor_pool
{
public:
    constexpr descriptor_pool() = default;
    descriptor_pool(VkDevice device, const std::vector<VkDescriptorPoolSize>& sizes, std::uint32_t max_sets);
    ~descriptor_pool();

    descriptor_pool(const descriptor_pool&) = delete;
    descriptor_pool& operator=(const descriptor_pool&) = delete;

    descriptor_pool(descriptor_pool&& other) noexcept;
    descriptor_pool& operator=(descriptor_pool&& other) noexcept;

    explicit descriptor_pool(VkDevice device, VkDescriptorPool descriptor_pool) noexcept
    :m_device{device}
    ,m_descriptor_pool{descriptor_pool}
    {

    }

    operator VkDescriptorPool() const noexcept
    {
        return m_descriptor_pool;
    }

private:
    VkDevice m_device{};
    VkDescriptorPool m_descriptor_pool{};
};

class descriptor_set
{
public:
    constexpr descriptor_set() = default;
    descriptor_set(VkDevice device, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout);
    ~descriptor_set() = default;

    descriptor_set(const descriptor_set&) = delete;
    descriptor_set& operator=(const descriptor_set&) = delete;

    descriptor_set(descriptor_set&& other) noexcept;
    descriptor_set& operator=(descriptor_set&& other) noexcept;

    explicit descriptor_set(VkDevice device, VkDescriptorSet descriptor_set) noexcept
    :m_device{device}
    ,m_descriptor_set{descriptor_set}
    {

    }

    operator VkDescriptorSet() const noexcept
    {
        return m_descriptor_set;
    }

private:
    VkDevice m_device{};
    VkDescriptorSet m_descriptor_set{};
};

class render_pass
{
public:
    constexpr render_pass() = default;
    render_pass(VkDevice device, const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& dependencies);
    ~render_pass();

    render_pass(const render_pass&) = delete;
    render_pass& operator=(const render_pass&) = delete;

    render_pass(render_pass&& other) noexcept;
    render_pass& operator=(render_pass&& other) noexcept;

    explicit render_pass(VkDevice device, VkRenderPass render_pass) noexcept
    :m_device{device}
    ,m_render_pass{render_pass}
    {

    }

    operator VkRenderPass() const noexcept
    {
        return m_render_pass;
    }

private:
    VkDevice m_device{};
    VkRenderPass m_render_pass{};
};

class pipeline_layout
{
public:
    constexpr pipeline_layout() = default;
    pipeline_layout(VkDevice device, const std::vector<VkDescriptorSetLayout>& layouts, const std::vector<VkPushConstantRange>& ranges);
    ~pipeline_layout();

    pipeline_layout(const pipeline_layout&) = delete;
    pipeline_layout& operator=(const pipeline_layout&) = delete;

    pipeline_layout(pipeline_layout&& other) noexcept;
    pipeline_layout& operator=(pipeline_layout&& other) noexcept;

    explicit pipeline_layout(VkDevice device, VkPipelineLayout pipeline_layout) noexcept
    :m_device{device}
    ,m_pipeline_layout{pipeline_layout}
    {

    }

    operator VkPipelineLayout() const noexcept
    {
        return m_pipeline_layout;
    }

private:
    VkDevice m_device{};
    VkPipelineLayout m_pipeline_layout{};
};

class pipeline
{
public:
    constexpr pipeline() = default;
    pipeline(VkDevice device, const VkGraphicsPipelineCreateInfo& create_info, VkPipelineCache cache);
    pipeline(VkDevice device, const VkComputePipelineCreateInfo& create_info, VkPipelineCache cache);
    ~pipeline();

    pipeline(const pipeline&) = delete;
    pipeline& operator=(const pipeline&) = delete;

    pipeline(pipeline&& other) noexcept;
    pipeline& operator=(pipeline&& other) noexcept;

    explicit pipeline(VkDevice device, VkPipeline pipeline) noexcept
    :m_device{device}
    ,m_pipeline{pipeline}
    {

    }

    operator VkPipeline() const noexcept
    {
        return m_pipeline;
    }

private:
    VkDevice m_device{};
    VkPipeline m_pipeline{};
};

class pipeline_cache
{
public:
    constexpr pipeline_cache() = default;
    pipeline_cache(VkDevice device, const void* initial_data = nullptr, std::size_t size = 0);
    ~pipeline_cache();

    pipeline_cache(const pipeline_cache&) = delete;
    pipeline_cache& operator=(const pipeline_cache&) = delete;

    pipeline_cache(pipeline_cache&& other) noexcept;
    pipeline_cache& operator=(pipeline_cache&& other) noexcept;

    explicit pipeline_cache(VkDevice device, VkPipelineCache pipeline_cache) noexcept
    :m_device{device}
    ,m_pipeline_cache{pipeline_cache}
    {

    }

    operator VkPipelineCache() const noexcept
    {
        return m_pipeline_cache;
    }

private:
    VkDevice m_device{};
    VkPipelineCache m_pipeline_cache{};
};

class debug_messenger
{
public:
    constexpr debug_messenger() = default;
    debug_messenger(VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT callback, VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagBitsEXT type);
    ~debug_messenger();

    debug_messenger(const debug_messenger&) = delete;
    debug_messenger& operator=(const debug_messenger&) = delete;

    debug_messenger(debug_messenger&& other) noexcept;
    debug_messenger& operator=(debug_messenger&& other) noexcept;

    explicit debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger) noexcept
    :m_instance{instance}
    ,m_debug_messenger{debug_messenger}
    {

    }

    operator VkDebugUtilsMessengerEXT() const noexcept
    {
        return m_debug_messenger;
    }

private:
    VkInstance m_instance{};
    VkDebugUtilsMessengerEXT m_debug_messenger{};
};

class surface
{
public:
    constexpr surface() = default;

#ifdef TPH_PLATFORM_ANDROID
    surface(VkInstance instance, const android_surface_info& info);
#endif

#ifdef TPH_PLATFORM_IOS
    surface(VkInstance instance, const ios_surface_info& info);
#endif

#ifdef TPH_PLATFORM_WIN32
    surface(VkInstance instance, const win32_surface_info& info);
#endif

#ifdef TPH_PLATFORM_MACOS
    surface(VkInstance instance, const macos_surface_info& info);
#endif

#ifdef TPH_PLATFORM_XLIB
    surface(VkInstance instance, const xlib_surface_info& info);
#endif

#ifdef TPH_PLATFORM_XCB
    surface(VkInstance instance, const xcb_surface_info& info);
#endif

#ifdef TPH_PLATFORM_WAYLAND
    surface(VkInstance instance, const wayland_surface_info& info);
#endif

    ~surface();

    surface(const surface&) = delete;
    surface& operator=(const surface&) = delete;

    surface(surface&& other) noexcept;
    surface& operator=(surface&& other) noexcept;

    explicit surface(VkInstance instance, VkSurfaceKHR surface) noexcept
    :m_instance{instance}
    ,m_surface{surface}
    {

    }

    operator VkSurfaceKHR() const noexcept
    {
        return m_surface;
    }

private:
    VkInstance m_instance{};
    VkSurfaceKHR m_surface{};
};

class swapchain
{
public:
    constexpr swapchain() = default;
    swapchain(VkDevice device, VkSurfaceKHR surface, VkExtent2D size, std::uint32_t image_count, VkSurfaceFormatKHR format, VkImageUsageFlags usage, const std::vector<std::uint32_t>& families, VkSurfaceTransformFlagBitsKHR transform, VkCompositeAlphaFlagBitsKHR composite, VkPresentModeKHR present_mode, VkBool32 clipped, VkSwapchainKHR old = nullptr);
    ~swapchain();

    swapchain(const swapchain&) = delete;
    swapchain& operator=(const swapchain&) = delete;

    swapchain(swapchain&& other) noexcept;
    swapchain& operator=(swapchain&& other) noexcept;

    explicit swapchain(VkDevice device, VkSwapchainKHR swapchain) noexcept
    :m_device{device}
    ,m_swapchain{swapchain}
    {

    }

    operator VkSwapchainKHR() const noexcept
    {
        return m_swapchain;
    }

private:
    VkDevice m_device{};
    VkSwapchainKHR m_swapchain{};
};

}

#endif
