#include "debug_messenger.hpp"

#include "vulkan/vulkan_functions.hpp"

#include "renderer.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

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
}

}
