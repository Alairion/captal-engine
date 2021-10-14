#ifndef TEPHRA_DEBUG_UTILS_HPP_INCLUDED
#define TEPHRA_DEBUG_UTILS_HPP_INCLUDED

#include "config.hpp"

#include <array>
#include <functional>
#include <string>
#include <string_view>
#include <span>
#include <memory>

#include "vulkan/vulkan.hpp"

#include "enumerations.hpp"

namespace tph
{

class application;
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
    performance = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
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
    std::span<const debug_label> queue_labels{};
    std::span<const debug_label> command_buffer_labels{};
    std::span<const debug_object> objects{};
};

TEPHRA_API void debug_messenger_default_callback(debug_message_severity severity, debug_message_type type, const debug_message_data& data);

class TEPHRA_API debug_messenger
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    using callback_type = std::function<void(debug_message_severity severity, debug_message_type type, const debug_message_data& data)>;

public:
    constexpr debug_messenger() = default;
    explicit debug_messenger(application& app, callback_type callback, debug_message_severity severities, debug_message_type types);

    explicit debug_messenger(vulkan::debug_messenger debug_messenger) noexcept
    :m_debug_messenger{std::move(debug_messenger)}
    {

    }

    ~debug_messenger() = default;
    debug_messenger(const debug_messenger&) = delete;
    debug_messenger& operator=(const debug_messenger&) = delete;
    debug_messenger(debug_messenger&& other) noexcept = default;
    debug_messenger& operator=(debug_messenger&& other) noexcept = default;

    const callback_type& callback() const noexcept
    {
        return *m_callback;
    }

private:
    std::unique_ptr<callback_type> m_callback{};
    vulkan::debug_messenger m_debug_messenger{};
};

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

template<> struct tph::enable_enum_operations<tph::debug_message_severity> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::debug_message_type> {static constexpr bool value{true};};

#endif
