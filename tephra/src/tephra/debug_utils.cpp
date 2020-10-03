#include "debug_utils.hpp"

#include "vulkan/vulkan_functions.hpp"

#include "application.hpp"
#include "renderer.hpp"

using namespace tph::vulkan::functions;

namespace tph
{
/*
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void*)
{
    const auto format_type = [](VkDebugUtilsMessageTypeFlagsEXT type) -> std::string
    {
        switch(type)
        {
            case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: return "  Category: generic\n";
            case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: return "  Category: validation\n";
            case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: return "  Category: performance\n";
            default: return "  Category: generic\n";
        }
    };

    const auto format_severity = [](VkDebugUtilsMessageSeverityFlagBitsEXT severity) -> std::string
    {
        switch(severity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: return "  Type: diagnostic\n";
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: return "  Type: information\n";
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: return "  Type: warning\n";
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: return "  Type: error\n";
            default: return "  Type: unknown\n";
        }
    };

    const auto format_message = [](const VkDebugUtilsMessengerCallbackDataEXT& data) -> std::string
    {
        std::string message{};

        message += "  Message: " + std::string{data.pMessage} + "\n";

        return message;
    };

    std::string message{"Debug messenger message:\n"};
    message += format_type(type);
    message += format_severity(severity);
    message += format_message(*callback_data);

    if(severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        std::cerr << message << std::endl;
        assert(false && "Validation error.");
    }
    else
    {
        std::cout << message << std::endl;
    }

    return VK_FALSE;
}*/

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* userdata)
{
    const auto& user_callback{*reinterpret_cast<debug_messenger::callback_type*>(userdata)};

    const auto convert_labels = [](std::uint32_t count, const VkDebugUtilsLabelEXT* labels)
    {
        std::vector<debug_label> output{};
        output.reserve(static_cast<std::size_t>(count));

        for(std::uint32_t i{}; i < count; ++i)
        {
            debug_label label{};
            label.name = labels[i].pLabelName;
            std::copy(std::begin(labels[i].color), std::end(labels[i].color), std::begin(label.color));

            output.emplace_back(label);
        }

        return output;
    };

    const auto convert_objects = [](std::uint32_t count, const VkDebugUtilsObjectNameInfoEXT* objects)
    {
        std::vector<debug_object> output{};
        output.reserve(static_cast<std::size_t>(count));

        for(std::uint32_t i{}; i < count; ++i)
        {
            debug_object object{};
            object.type = static_cast<object_type>(objects[i].objectType);
            object.handle = objects[i].objectHandle;
            object.name = objects[i].pObjectName;

            output.emplace_back(object);
        }

        return output;
    };

    const auto queues{convert_labels(callback_data->queueLabelCount, callback_data->pQueueLabels)};
    const auto command_buffer{convert_labels(callback_data->cmdBufLabelCount, callback_data->pCmdBufLabels)};
    const auto objects{convert_objects(callback_data->objectCount, callback_data->pObjects)};

    debug_message_data data{};
    data.message_name = callback_data->pMessageIdName;
    data.message_id = callback_data->messageIdNumber;
    data.message = callback_data->pMessage;
    data.queue_labels = std::span<const debug_label>{queues};
    data.command_buffer_labels = std::span<const debug_label>{queues};
    data.objects = std::span<const debug_object>{objects};

    user_callback(static_cast<debug_message_severity>(severity), static_cast<debug_message_type>(type), data);

    return VK_FALSE;
}

debug_messenger::debug_messenger(application& app, callback_type callback, debug_message_severity severities, debug_message_type types)
:m_callback{std::make_unique<callback_type>(std::move(callback))}
,m_debug_messenger{underlying_cast<VkInstance>(app), &debug_messenger_callback, static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(severities), static_cast<VkDebugUtilsMessageTypeFlagBitsEXT>(types), m_callback.get()}
{

}

}
