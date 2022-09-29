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

#ifndef TEPHRA_VULKAN_HPP_INCLUDED
#define TEPHRA_VULKAN_HPP_INCLUDED

#include "../config.hpp"

#include <string>
#include <span>
#include <exception>
#include <algorithm>
#include <utility>

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

namespace functions
{

struct instance_level_functions;
struct device_level_functions;

}

class TEPHRA_API error final : public std::exception
{
public:
    error() noexcept = default;

    explicit error(VkResult result) noexcept
    :m_result{result}
    {

    }

    virtual ~error() = default;
    error(const error&) noexcept = default;
    error& operator=(const error&) noexcept = default;
    error(error&& other) noexcept = default;
    error& operator=(error&& other) noexcept = default;

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

inline void check(VkResult result)
{
    if(result != VK_SUCCESS)
    {
        throw error{result};
    }
}

struct instance_context
{
    VkInstance instance{};
    const functions::instance_level_functions* functions{};

    const functions::instance_level_functions* operator->() const noexcept
    {
        return functions;
    }
};

class TEPHRA_API instance
{
public:
    constexpr instance() = default;
    explicit instance(const std::string& application_name, tph::version application_version, tph::version api_version, std::span<const char* const> layers, std::span<const char* const> extensions);
    explicit instance(VkInstance instance) noexcept;

    ~instance();
    instance(const tph::vulkan::instance&) = delete;
    instance& operator=(const tph::vulkan::instance&) = delete;

    instance(instance&& other) noexcept
    :m_context{std::exchange(other.m_context, instance_context{})}
    {

    }

    instance& operator=(instance&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);

        return *this;
    }

    instance_context context() const noexcept
    {
        return m_context;
    }

    instance_context release() noexcept
    {
        return std::exchange(m_context, instance_context{});
    }

    operator VkInstance() const noexcept
    {
        return m_context.instance;
    }

    const functions::instance_level_functions* operator->() const noexcept
    {
        return m_context.functions;
    }

private:
    instance_context m_context{};
};

struct device_context
{
    VkDevice device{};
    const functions::device_level_functions* functions{};

    const functions::device_level_functions* operator->() const noexcept
    {
        return functions;
    }
};

class TEPHRA_API device
{
public:
    constexpr device() = default;
    explicit device(const instance_context& instance, VkPhysicalDevice physical_device, std::span<const char* const> layers, std::span<const char* const> extensions, std::span<const VkDeviceQueueCreateInfo> queues, const VkPhysicalDeviceFeatures& features);
    explicit device(const instance_context& instance, VkDevice device) noexcept;

    ~device();
    device(const device&) = delete;
    device& operator=(const device&) = delete;

    device(device&& other) noexcept
    :m_context{std::exchange(other.m_context, device_context{})}
    {

    }

    device& operator=(device&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    device_context release() noexcept
    {
        return std::exchange(m_context, device_context{});
    }

    operator VkDevice() const noexcept
    {
        return m_context.device;
    }

    const functions::device_level_functions* operator->() const noexcept
    {
        return m_context.functions;
    }

private:
    device_context m_context{};
};

class TEPHRA_API device_memory
{
public:
    constexpr device_memory() = default;
    explicit device_memory(const device_context& context, std::uint32_t memory_type, std::uint64_t size);

    explicit device_memory(const device_context& context, VkDeviceMemory device_memory) noexcept
    :m_context{context}
    ,m_device_memory{device_memory}
    {

    }

    ~device_memory();
    device_memory(const tph::vulkan::device_memory&) = delete;
    device_memory& operator=(const tph::vulkan::device_memory&) = delete;

    device_memory(device_memory&& other) noexcept
    :m_context{other.m_context}
    ,m_device_memory{std::exchange(other.m_device_memory, nullptr)}
    {

    }

    device_memory& operator=(device_memory&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_device_memory = std::exchange(other.m_device_memory, m_device_memory);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    operator VkDeviceMemory() const noexcept
    {
        return m_device_memory;
    }

private:
    device_context m_context{};
    VkDeviceMemory m_device_memory{};
};

class TEPHRA_API buffer
{
public:
    constexpr buffer() = default;
    explicit buffer(const device_context& context, uint64_t size, VkBufferUsageFlags usage);

    explicit buffer(const device_context& context, VkBuffer buffer) noexcept
    :m_context{context}
    ,m_buffer{buffer}
    {

    }

    ~buffer();
    buffer(const buffer&) = delete;
    buffer& operator=(const buffer&) = delete;

    buffer(buffer&& other) noexcept
    :m_context{other.m_context}
    ,m_buffer{std::exchange(other.m_buffer, nullptr)}
    {

    }

    buffer& operator=(buffer&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_buffer = std::exchange(other.m_buffer, m_buffer);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkBuffer() const noexcept
    {
        return m_buffer;
    }

private:
    device_context m_context{};
    VkBuffer m_buffer{};
};

class TEPHRA_API buffer_view
{
public:
    constexpr buffer_view() = default;
    explicit buffer_view(const device_context& context, VkBuffer buffer, VkFormat format, std::uint64_t offset = 0, std::uint64_t size = VK_WHOLE_SIZE);

    explicit buffer_view(const device_context& context, VkBufferView buffer_view) noexcept
    :m_context{context}
    ,m_buffer_view{buffer_view}
    {

    }

    ~buffer_view();
    buffer_view(const buffer_view&) = delete;
    buffer_view& operator=(const buffer_view&) = delete;

    buffer_view(buffer_view&& other) noexcept
    :m_context{other.m_context}
    ,m_buffer_view{std::exchange(other.m_buffer_view, nullptr)}
    {

    }

    buffer_view& operator=(buffer_view&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_buffer_view = std::exchange(other.m_buffer_view, m_buffer_view);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkBufferView() const noexcept
    {
        return m_buffer_view;
    }

private:
    device_context m_context{};
    VkBufferView m_buffer_view{};
};

class TEPHRA_API image
{
public:
    constexpr image() = default;
    explicit image(const device_context& context, const VkImageCreateInfo& info);

    explicit image(const device_context& context, VkImage image) noexcept
    :m_context{context}
    ,m_image{image}
    {

    }

    explicit image(VkImage image) noexcept //Non-owning (useful for swapchain images)
    :m_image{image}
    {

    }

    ~image();
    image(const image&) = delete;
    image& operator=(const image&) = delete;

    image(image&& other) noexcept
    :m_context{other.m_context}
    ,m_image{std::exchange(other.m_image, nullptr)}
    {

    }

    image& operator=(image&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_image = std::exchange(other.m_image, m_image);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkImage() const noexcept
    {
        return m_image;
    }

private:
    device_context m_context{};
    VkImage m_image{};
};

class TEPHRA_API image_view
{
public:
    constexpr image_view() = default;
    explicit image_view(const device_context& context, const VkImageViewCreateInfo& info);

    explicit image_view(const device_context& context, VkImageView image_view) noexcept
    :m_context{context}
    ,m_image_view{image_view}
    {

    }

    ~image_view();
    image_view(const image_view&) = delete;
    image_view& operator=(const image_view&) = delete;

    image_view(image_view&& other) noexcept
    :m_context{other.m_context}
    ,m_image_view{std::exchange(other.m_image_view, nullptr)}
    {

    }

    image_view& operator=(image_view&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_image_view = std::exchange(other.m_image_view, m_image_view);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkImageView() const noexcept
    {
        return m_image_view;
    }

private:
    device_context m_context{};
    VkImageView m_image_view{};
};

class TEPHRA_API sampler
{
public:
    constexpr sampler() = default;
    explicit sampler(const device_context& context, const VkSamplerCreateInfo& info);

    explicit sampler(const device_context& context, VkSampler sampler) noexcept
    :m_context{context}
    ,m_sampler{sampler}
    {

    }

    ~sampler();
    sampler(const sampler&) = delete;
    sampler& operator=(const sampler&) = delete;

    sampler(sampler&& other) noexcept
    :m_context{other.m_context}
    ,m_sampler{std::exchange(other.m_sampler, nullptr)}
    {

    }

    sampler& operator=(sampler&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_sampler = std::exchange(other.m_sampler, m_sampler);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkSampler() const noexcept
    {
        return m_sampler;
    }

private:
    device_context m_context{};
    VkSampler m_sampler{};
};

class TEPHRA_API framebuffer
{
public:
    constexpr framebuffer() = default;
    explicit framebuffer(const device_context& context, VkRenderPass render_pass, std::span<const VkImageView> attachments, VkExtent2D size, std::uint32_t layers = 1);

    explicit framebuffer(const device_context& context, VkFramebuffer framebuffer) noexcept
    :m_context{context}
    ,m_framebuffer{framebuffer}
    {

    }

    ~framebuffer();
    framebuffer(const framebuffer&) = delete;
    framebuffer& operator=(const framebuffer&) = delete;

    framebuffer(framebuffer&& other) noexcept
    :m_context{other.m_context}
    ,m_framebuffer{std::exchange(other.m_framebuffer, nullptr)}
    {

    }

    framebuffer& operator=(framebuffer&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_framebuffer = std::exchange(other.m_framebuffer, m_framebuffer);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkFramebuffer() const noexcept
    {
        return m_framebuffer;
    }

private:
    device_context m_context{};
    VkFramebuffer m_framebuffer{};
};

class TEPHRA_API shader
{
public:
    constexpr shader() = default;
    explicit shader(const device_context& context, std::size_t size, const std::uint32_t* code);

    explicit shader(const device_context& context, VkShaderModule shader) noexcept
    :m_context{context}
    ,m_shader{shader}
    {

    }

    ~shader();
    shader(const shader&) = delete;
    shader& operator=(const shader&) = delete;

    shader(shader&& other) noexcept
    :m_context{other.m_context}
    ,m_shader{std::exchange(other.m_shader, nullptr)}
    {

    }

    shader& operator=(shader&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_shader = std::exchange(other.m_shader, m_shader);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkShaderModule() const noexcept
    {
        return m_shader;
    }

private:
    device_context m_context{};
    VkShaderModule m_shader{};
};

class TEPHRA_API semaphore
{
public:
    constexpr semaphore() = default;
    explicit semaphore(const device_context& context);

    explicit semaphore(const device_context& context, VkSemaphore semaphore) noexcept
    :m_context{context}
    ,m_semaphore{semaphore}
    {

    }

    ~semaphore();
    semaphore(const semaphore&) = delete;
    semaphore& operator=(const semaphore&) = delete;

    semaphore(semaphore&& other) noexcept
    :m_context{other.m_context}
    ,m_semaphore{std::exchange(other.m_semaphore, nullptr)}
    {

    }

    semaphore& operator=(semaphore&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_semaphore = std::exchange(other.m_semaphore, m_semaphore);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkSemaphore() const noexcept
    {
        return m_semaphore;
    }

private:
    device_context m_context{};
    VkSemaphore m_semaphore{};
};

class TEPHRA_API fence
{
public:
    constexpr fence() = default;
    explicit fence(const device_context& context, VkFenceCreateFlags flags = 0);

    explicit fence(const device_context& context, VkFence fence) noexcept
    :m_context{context}
    ,m_fence{fence}
    {

    }

    ~fence();
    fence(const fence&) = delete;
    fence& operator=(const fence&) = delete;

    fence(fence&& other) noexcept
    :m_context{other.m_context}
    ,m_fence{std::exchange(other.m_fence, nullptr)}
    {

    }

    fence& operator=(fence&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_fence = std::exchange(other.m_fence, m_fence);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkFence() const noexcept
    {
        return m_fence;
    }

private:
    device_context m_context{};
    VkFence m_fence{};
};

class TEPHRA_API event
{
public:
    constexpr event() = default;
    explicit event(const device_context& context);

    explicit event(const device_context& context, VkEvent event) noexcept
    :m_context{context}
    ,m_event{event}
    {

    }

    ~event();
    event(const event&) = delete;
    event& operator=(const event&) = delete;

    event(event&& other) noexcept
    :m_context{other.m_context}
    ,m_event{std::exchange(other.m_event, nullptr)}
    {

    }

    event& operator=(event&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_event = std::exchange(other.m_event, m_event);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkEvent() const noexcept
    {
        return m_event;
    }

private:
    device_context m_context{};
    VkEvent m_event{};
};

class TEPHRA_API command_pool
{
public:
    constexpr command_pool() = default;
    explicit command_pool(const device_context& context, std::uint32_t queue_family, VkCommandPoolCreateFlags flags = 0);

    explicit command_pool(const device_context& context, VkCommandPool command_pool) noexcept
    :m_context{context}
    ,m_command_pool{command_pool}
    {

    }

    ~command_pool();
    command_pool(const command_pool&) = delete;
    command_pool& operator=(const command_pool&) = delete;

    command_pool(command_pool&& other) noexcept
    :m_context{other.m_context}
    ,m_command_pool{std::exchange(other.m_command_pool, nullptr)}
    {

    }

    command_pool& operator=(command_pool&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_command_pool = std::exchange(other.m_command_pool, m_command_pool);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkCommandPool() const noexcept
    {
        return m_command_pool;
    }

private:
    device_context m_context{};
    VkCommandPool m_command_pool{};
};

class TEPHRA_API command_buffer
{
public:
    constexpr command_buffer() = default;
    explicit command_buffer(const device_context& context, VkCommandPool command_pool, VkCommandBufferLevel level);

    explicit command_buffer(const device_context& context, VkCommandPool command_pool, VkCommandBuffer command_buffer) noexcept
    :m_context{context}
    ,m_command_pool{command_pool}
    ,m_command_buffer{command_buffer}
    {

    }

    ~command_buffer();
    command_buffer(const command_buffer&) = delete;
    command_buffer& operator=(const command_buffer&) = delete;

    command_buffer(command_buffer&& other) noexcept
    :m_context{other.m_context}
    ,m_command_pool{other.m_command_pool}
    ,m_command_buffer{std::exchange(other.m_command_buffer, nullptr)}
    {

    }

    command_buffer& operator=(command_buffer&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_command_pool = std::exchange(other.m_command_pool, m_command_pool);
        m_command_buffer = std::exchange(other.m_command_buffer, m_command_buffer);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    VkCommandPool command_pool() const noexcept
    {
        return m_command_pool;
    }

    operator VkCommandBuffer() const noexcept
    {
        return m_command_buffer;
    }

private:
    device_context m_context{};
    VkCommandPool m_command_pool{};
    VkCommandBuffer m_command_buffer{};
};

class TEPHRA_API descriptor_set_layout
{
public:
    constexpr descriptor_set_layout() = default;
    explicit descriptor_set_layout(const device_context& context, std::span<const VkDescriptorSetLayoutBinding> bindings);

    explicit descriptor_set_layout(const device_context& context, VkDescriptorSetLayout descriptor_set_layout) noexcept
    :m_context{context}
    ,m_descriptor_set_layout{descriptor_set_layout}
    {

    }

    ~descriptor_set_layout();
    descriptor_set_layout(const descriptor_set_layout&) = delete;
    descriptor_set_layout& operator=(const descriptor_set_layout&) = delete;

    descriptor_set_layout(descriptor_set_layout&& other) noexcept
    :m_context{other.m_context}
    ,m_descriptor_set_layout{std::exchange(other.m_descriptor_set_layout, nullptr)}
    {

    }

    descriptor_set_layout& operator=(descriptor_set_layout&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_descriptor_set_layout = std::exchange(other.m_descriptor_set_layout, m_descriptor_set_layout);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkDescriptorSetLayout() const noexcept
    {
        return m_descriptor_set_layout;
    }

private:
    device_context m_context{};
    VkDescriptorSetLayout m_descriptor_set_layout{};
};

class TEPHRA_API descriptor_pool
{
public:
    constexpr descriptor_pool() = default;
    explicit descriptor_pool(const device_context& context, std::span<const VkDescriptorPoolSize> sizes, std::uint32_t max_sets);

    explicit descriptor_pool(const device_context& context, VkDescriptorPool descriptor_pool) noexcept
    :m_context{context}
    ,m_descriptor_pool{descriptor_pool}
    {

    }

    ~descriptor_pool();
    descriptor_pool(const descriptor_pool&) = delete;
    descriptor_pool& operator=(const descriptor_pool&) = delete;

    descriptor_pool(descriptor_pool&& other) noexcept
    :m_context{other.m_context}
    ,m_descriptor_pool{std::exchange(other.m_descriptor_pool, nullptr)}
    {

    }

    descriptor_pool& operator=(descriptor_pool&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_descriptor_pool = std::exchange(other.m_descriptor_pool, m_descriptor_pool);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkDescriptorPool() const noexcept
    {
        return m_descriptor_pool;
    }

private:
    device_context m_context{};
    VkDescriptorPool m_descriptor_pool{};
};

class TEPHRA_API descriptor_set
{
public:
    constexpr descriptor_set() = default;
    explicit descriptor_set(const device_context& context, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout);

    explicit descriptor_set(const device_context& context, VkDescriptorSet descriptor_set) noexcept
    :m_context{context}
    ,m_descriptor_set{descriptor_set}
    {

    }

    ~descriptor_set() = default;
    descriptor_set(const descriptor_set&) = delete;
    descriptor_set& operator=(const descriptor_set&) = delete;

    descriptor_set(descriptor_set&& other) noexcept
    :m_context{other.m_context}
    ,m_descriptor_set{std::exchange(other.m_descriptor_set, nullptr)}
    {

    }

    descriptor_set& operator=(descriptor_set&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_descriptor_set = std::exchange(other.m_descriptor_set, m_descriptor_set);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkDescriptorSet() const noexcept
    {
        return m_descriptor_set;
    }

private:
    device_context m_context{};
    VkDescriptorSet m_descriptor_set{};
};

class TEPHRA_API pipeline_layout
{
public:
    constexpr pipeline_layout() = default;
    explicit pipeline_layout(const device_context& context, std::span<const VkDescriptorSetLayout> layouts, std::span<const VkPushConstantRange> ranges);

    explicit pipeline_layout(const device_context& context, VkPipelineLayout pipeline_layout) noexcept
    :m_context{context}
    ,m_pipeline_layout{pipeline_layout}
    {

    }

    ~pipeline_layout();
    pipeline_layout(const pipeline_layout&) = delete;
    pipeline_layout& operator=(const pipeline_layout&) = delete;

    pipeline_layout(pipeline_layout&& other) noexcept
    :m_context{other.m_context}
    ,m_pipeline_layout{std::exchange(other.m_pipeline_layout, nullptr)}
    {

    }

    pipeline_layout& operator=(pipeline_layout&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_pipeline_layout = std::exchange(other.m_pipeline_layout, m_pipeline_layout);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkPipelineLayout() const noexcept
    {
        return m_pipeline_layout;
    }

private:
    device_context m_context{};
    VkPipelineLayout m_pipeline_layout{};
};

class TEPHRA_API render_pass
{
public:
    constexpr render_pass() = default;
    explicit render_pass(const device_context& context, std::span<const VkAttachmentDescription> attachments, std::span<const VkSubpassDescription> subpasses, std::span<const VkSubpassDependency> dependencies);

    explicit render_pass(const device_context& context, VkRenderPass render_pass) noexcept
    :m_context{context}
    ,m_render_pass{render_pass}
    {

    }

    ~render_pass();
    render_pass(const render_pass&) = delete;
    render_pass& operator=(const render_pass&) = delete;

    render_pass(render_pass&& other) noexcept
    :m_context{other.m_context}
    ,m_render_pass{std::exchange(other.m_render_pass, nullptr)}
    {

    }

    render_pass& operator=(render_pass&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_render_pass = std::exchange(other.m_render_pass, m_render_pass);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkRenderPass() const noexcept
    {
        return m_render_pass;
    }

private:
    device_context m_context{};
    VkRenderPass m_render_pass{};
};

class TEPHRA_API pipeline
{
public:
    constexpr pipeline() = default;
    explicit pipeline(const device_context& context, const VkGraphicsPipelineCreateInfo& create_info, VkPipelineCache cache);
    explicit pipeline(const device_context& context, const VkComputePipelineCreateInfo& create_info, VkPipelineCache cache);

    explicit pipeline(const device_context& context, VkPipeline pipeline) noexcept
    :m_context{context}
    ,m_pipeline{pipeline}
    {

    }

    ~pipeline();
    pipeline(const pipeline&) = delete;
    pipeline& operator=(const pipeline&) = delete;

    pipeline(pipeline&& other) noexcept
    :m_context{other.m_context}
    ,m_pipeline{std::exchange(other.m_pipeline, nullptr)}
    {

    }

    pipeline& operator=(pipeline&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_pipeline = std::exchange(other.m_pipeline, m_pipeline);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkPipeline() const noexcept
    {
        return m_pipeline;
    }

private:
    device_context m_context{};
    VkPipeline m_pipeline{};
};

class TEPHRA_API pipeline_cache
{
public:
    constexpr pipeline_cache() = default;
    explicit pipeline_cache(const device_context& context, const void* initial_data = nullptr, std::size_t size = 0);

    explicit pipeline_cache(const device_context& context, VkPipelineCache pipeline_cache) noexcept
    :m_context{context}
    ,m_pipeline_cache{pipeline_cache}
    {

    }

    ~pipeline_cache();
    pipeline_cache(const pipeline_cache&) = delete;
    pipeline_cache& operator=(const pipeline_cache&) = delete;

    pipeline_cache(pipeline_cache&& other) noexcept
    :m_context{other.m_context}
    ,m_pipeline_cache{std::exchange(other.m_pipeline_cache, nullptr)}
    {

    }

    pipeline_cache& operator=(pipeline_cache&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_pipeline_cache = std::exchange(other.m_pipeline_cache, m_pipeline_cache);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkPipelineCache() const noexcept
    {
        return m_pipeline_cache;
    }

private:
    device_context m_context{};
    VkPipelineCache m_pipeline_cache{};
};

class TEPHRA_API query_pool
{
public:
    constexpr query_pool() = default;
    explicit query_pool(const device_context& context, VkQueryType type, std::uint32_t count, VkQueryPipelineStatisticFlags statistics);

    explicit query_pool(const device_context& context, VkQueryPool query_pool) noexcept
    :m_context{context}
    ,m_query_pool{query_pool}
    {

    }

    ~query_pool();
    query_pool(const query_pool&) = delete;
    query_pool& operator=(const query_pool&) = delete;

    query_pool(query_pool&& other) noexcept
    :m_context{other.m_context}
    ,m_query_pool{std::exchange(other.m_query_pool, nullptr)}
    {

    }

    query_pool& operator=(query_pool&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_query_pool = std::exchange(other.m_query_pool, m_query_pool);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkQueryPool() const noexcept
    {
        return m_query_pool;
    }

private:
    device_context m_context{};
    VkQueryPool m_query_pool{};
};

class TEPHRA_API debug_messenger
{
public:
    constexpr debug_messenger() = default;
    explicit debug_messenger(const instance_context& instance, PFN_vkDebugUtilsMessengerCallbackEXT callback, VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagBitsEXT type, void* userdata);

    explicit debug_messenger(const instance_context& instance, VkDebugUtilsMessengerEXT debug_messenger) noexcept
    :m_context{instance}
    ,m_debug_messenger{debug_messenger}
    {

    }

    ~debug_messenger();
    debug_messenger(const debug_messenger&) = delete;
    debug_messenger& operator=(const debug_messenger&) = delete;

    debug_messenger(debug_messenger&& other) noexcept
    :m_context{other.m_context}
    ,m_debug_messenger{std::exchange(other.m_debug_messenger, nullptr)}
    {

    }

    debug_messenger& operator=(debug_messenger&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_debug_messenger = std::exchange(other.m_debug_messenger, m_debug_messenger);

        return *this;
    }

    instance_context context() const noexcept
    {
        return m_context;
    }

    VkInstance instance() const noexcept
    {
        return m_context.instance;
    }

    operator VkDebugUtilsMessengerEXT() const noexcept
    {
        return m_debug_messenger;
    }

private:
    instance_context m_context{};
    VkDebugUtilsMessengerEXT m_debug_messenger{};
};

class TEPHRA_API surface
{
public:
    constexpr surface() = default;

#ifdef TPH_PLATFORM_ANDROID
    explicit surface(const instance_context& instance, const android_surface_info& info);
#endif

#ifdef TPH_PLATFORM_IOS
    explicit surface(const instance_context& instance, const ios_surface_info& info);
#endif

#ifdef TPH_PLATFORM_WIN32
    explicit surface(const instance_context& instance, const win32_surface_info& info);
#endif

#ifdef TPH_PLATFORM_MACOS
    explicit surface(const instance_context& instance, const macos_surface_info& info);
#endif

#ifdef TPH_PLATFORM_XLIB
    explicit surface(const instance_context& instance, const xlib_surface_info& info);
#endif

#ifdef TPH_PLATFORM_XCB
    explicit surface(const instance_context& instance, const xcb_surface_info& info);
#endif

#ifdef TPH_PLATFORM_WAYLAND
    explicit surface(const instance_context& instance, const wayland_surface_info& info);
#endif

    explicit surface(const instance_context& instance, VkSurfaceKHR surface) noexcept
    :m_context{instance}
    ,m_surface{surface}
    {

    }

    ~surface();
    surface(const surface&) = delete;
    surface& operator=(const surface&) = delete;

    surface(surface&& other) noexcept
    :m_context{other.m_context}
    ,m_surface{std::exchange(other.m_surface, nullptr)}
    {

    }

    surface& operator=(surface&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_surface = std::exchange(other.m_surface, m_surface);

        return *this;
    }

    instance_context context() const noexcept
    {
        return m_context;
    }

    VkInstance instance() const noexcept
    {
        return m_context.instance;
    }

    operator VkSurfaceKHR() const noexcept
    {
        return m_surface;
    }

private:
    instance_context m_context{};
    VkSurfaceKHR m_surface{};
};

class TEPHRA_API swapchain
{
public:
    constexpr swapchain() = default;
    explicit swapchain(const device_context& context, VkSurfaceKHR surface, VkExtent2D size, std::uint32_t image_count, VkSurfaceFormatKHR format, VkImageUsageFlags usage, std::span<const std::uint32_t> families, VkSurfaceTransformFlagBitsKHR transform, VkCompositeAlphaFlagBitsKHR composite, VkPresentModeKHR present_mode, VkBool32 clipped, VkSwapchainKHR old = nullptr);

    explicit swapchain(const device_context& context, VkSwapchainKHR swapchain) noexcept
    :m_context{context}
    ,m_swapchain{swapchain}
    {

    }

    ~swapchain();
    swapchain(const swapchain&) = delete;
    swapchain& operator=(const swapchain&) = delete;

    swapchain(swapchain&& other) noexcept
    :m_context{other.m_context}
    ,m_swapchain{std::exchange(other.m_swapchain, nullptr)}
    {

    }

    swapchain& operator=(swapchain&& other) noexcept
    {
        m_context = std::exchange(other.m_context, m_context);
        m_swapchain = std::exchange(other.m_swapchain, m_swapchain);

        return *this;
    }

    device_context context() const noexcept
    {
        return m_context;
    }

    VkDevice device() const noexcept
    {
        return m_context.device;
    }

    operator VkSwapchainKHR() const noexcept
    {
        return m_swapchain;
    }

private:
    device_context m_context{};
    VkSwapchainKHR m_swapchain{};
};

}

#endif
