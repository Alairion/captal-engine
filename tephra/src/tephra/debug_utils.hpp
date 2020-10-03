#ifndef TEPHRA_DEBUG_UTILS_HPP_INCLUDED
#define TEPHRA_DEBUG_UTILS_HPP_INCLUDED

#include "config.hpp"

#include <functional>
#include <string_view>
#include <span>

#include "vulkan/vulkan.hpp"

#include "enumerations.hpp"

namespace tph
{

class renderer;

enum class debug_message_severity : std::uint32_t
{
    verbose = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
    information = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
    warning = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
    error = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
};

enum class debug_message_type : std::uint32_t
{
    general = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
    validation = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
    performace = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
};

enum class object_type : std::uint32_t
{
    unknown = VK_OBJECT_TYPE_UNKNOWN,
    instance = VK_OBJECT_TYPE_INSTANCE,
    physical_device = VK_OBJECT_TYPE_PHYSICAL_DEVICE,
    device = VK_OBJECT_TYPE_DEVICE,
    queue = VK_OBJECT_TYPE_QUEUE,
    semaphore = VK_OBJECT_TYPE_SEMAPHORE,
    command_buffer = VK_OBJECT_TYPE_COMMAND_BUFFER,
    fence = VK_OBJECT_TYPE_FENCE,
    device_memory = VK_OBJECT_TYPE_DEVICE_MEMORY,
    buffer = VK_OBJECT_TYPE_BUFFER,
    image = VK_OBJECT_TYPE_IMAGE,
    event = VK_OBJECT_TYPE_EVENT,
    query_pool = VK_OBJECT_TYPE_QUERY_POOL,
    buffer_view = VK_OBJECT_TYPE_BUFFER_VIEW,
    image_view = VK_OBJECT_TYPE_IMAGE_VIEW,
    shader_module = VK_OBJECT_TYPE_SHADER_MODULE,
    pipeline_cache = VK_OBJECT_TYPE_PIPELINE_CACHE,
    pipeline_layout = VK_OBJECT_TYPE_PIPELINE_LAYOUT,
    render_pass = VK_OBJECT_TYPE_RENDER_PASS,
    pipeline = VK_OBJECT_TYPE_PIPELINE,
    descriptor_set_layout = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
    sampler = VK_OBJECT_TYPE_SAMPLER,
    descriptor_pool = VK_OBJECT_TYPE_DESCRIPTOR_POOL,
    descriptor_set = VK_OBJECT_TYPE_DESCRIPTOR_SET,
    framebuffer = VK_OBJECT_TYPE_FRAMEBUFFER,
    command_pool = VK_OBJECT_TYPE_COMMAND_POOL,
    sampler_ycbcr_conversion = VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION,
    surface = VK_OBJECT_TYPE_SURFACE_KHR,
    swapchain = VK_OBJECT_TYPE_SWAPCHAIN_KHR,
    debug_report_callback = VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT,
    debug_messenger = VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT,
};

struct debug_label
{
    std::string_view name{};
    std::array<float, 4> color{};
};

struct debug_object
{
    object_type type{};
    std::uint64_t handle{};
    std::string_view name{};
};

struct debug_message_data
{
    std::string_view message_name{};
    std::int32_t message_id{};
    std::string_view message{};
    std::span<debug_label> queue_labels{};
    std::span<debug_label> command_buffer_labels{};
    std::span<debug_object> objects{};
};

class TEPHRA_API debug_messenger
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    using callback_type = std::function<void(debug_message_severity severity, debug_message_type type, const debug_message_data& data)>;

public:
    constexpr debug_messenger() = default;
    explicit debug_messenger(renderer& renderer, callback_type callback);

    explicit debug_messenger(vulkan::debug_messenger debug_messenger) noexcept
    :m_debug_messenger{std::move(debug_messenger)}
    {

    }

    ~debug_messenger() = default;
    debug_messenger(const debug_messenger&) = delete;
    debug_messenger& operator=(const debug_messenger&) = delete;
    debug_messenger(debug_messenger&& other) noexcept = default;
    debug_messenger& operator=(debug_messenger&& other) noexcept = default;

private:
    vulkan::debug_messenger m_debug_messenger{};
};

TEPHRA_API void set_object_name(renderer& renderer, const debug_messenger& messenger, const std::string& name);
TEPHRA_API void set_object_tag(renderer& renderer, const debug_messenger& messenger, std::uint64_t id, std::span<std::uint8_t> tag);

template<>
inline VkInstance underlying_cast(const debug_messenger& debug_messenger) noexcept
{
    return debug_messenger.m_debug_messenger.instance();
}

template<>
inline VkDebugUtilsMessengerEXT underlying_cast(const debug_messenger& debug_messenger) noexcept
{
    return debug_messenger.m_debug_messenger;
}

}

#endif
