#include "debug_utils.hpp"

#include <cassert>
#include <iostream>
#include <mutex>
#include <charconv>

#include "vulkan/vulkan_functions.hpp"

#include "application.hpp"
#include "renderer.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

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

            if(labels[i].pLabelName)
            {
                label.name = labels[i].pLabelName;
            }

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

            if(objects[i].pObjectName)
            {
                object.name = objects[i].pObjectName;
            }

            output.emplace_back(object);
        }

        return output;
    };

    const auto queues{convert_labels(callback_data->queueLabelCount, callback_data->pQueueLabels)};
    const auto command_buffer{convert_labels(callback_data->cmdBufLabelCount, callback_data->pCmdBufLabels)};
    const auto objects{convert_objects(callback_data->objectCount, callback_data->pObjects)};

    debug_message_data data{};

    if(callback_data->pMessageIdName)
    {
        data.message_name = callback_data->pMessageIdName;
    }

    data.message_id = callback_data->messageIdNumber;
    data.message = callback_data->pMessage;
    data.queue_labels = std::span<const debug_label>{queues};
    data.command_buffer_labels = std::span<const debug_label>{queues};
    data.objects = std::span<const debug_object>{objects};

    user_callback(static_cast<debug_message_severity>(severity), static_cast<debug_message_type>(type), data);

    return VK_FALSE;
}

void debug_messenger_default_callback(debug_message_severity severity, debug_message_type type, const debug_message_data& data)
{
    const auto format_severity = [](debug_message_severity severity) -> std::string_view
    {
        switch(severity)
        {
            case debug_message_severity::verbose:     return "  Type: diagnostic\n";
            case debug_message_severity::information: return "  Type: information\n";
            case debug_message_severity::warning:     return "  Type: warning\n";
            case debug_message_severity::error:       return "  Type: error\n";
            default: return "  Type: unknown\n";
        }
    };

    const auto format_type = [](debug_message_type type) -> std::string_view
    {
        switch(type)
        {
            case debug_message_type::general:     return "  Category: generic\n";
            case debug_message_type::validation:  return "  Category: validation\n";
            case debug_message_type::performance: return "  Category: performance\n";
            default: return "  Category: unknown\n";
        }
    };

    const auto format_color = [](const std::array<float, 4> color) -> std::string
    {
        std::string output{};

        for(auto value : color)
        {
            const auto ivalue{static_cast<std::uint32_t>(value * 255.0f)};

            std::array<char, 2> hex{};
            auto [ptr, error] = std::to_chars(std::data(hex), std::data(hex) + std::size(hex), ivalue, 16);

            if(error == std::errc{})
            {
                if(ivalue < 10)
                {
                    output.push_back('0');
                    output.push_back(hex[0]);
                }
                else
                {
                    output.push_back(hex[0]);
                    output.push_back(hex[1]);
                }
            }
        }

        return output;
    };

    const auto format_object_type = [](object_type type) -> std::string_view
    {
        switch(type)
        {
            case object_type::unknown:                   return "unknown";
            case object_type::instance:                  return "instance";
            case object_type::physical_device:           return "physical_device";
            case object_type::device:                    return "device";
            case object_type::queue:                     return "queue";
            case object_type::semaphore:                 return "semaphore";
            case object_type::command_buffer:            return "command_buffer";
            case object_type::fence:                     return "fence";
            case object_type::device_memory:             return "device_memory";
            case object_type::buffer:                    return "buffer";
            case object_type::image:                     return "image";
            case object_type::event:                     return "event";
            case object_type::query_pool:                return "query_pool";
            case object_type::buffer_view:               return "buffer_view";
            case object_type::image_view:                return "image_view";
            case object_type::shader_module:             return "shader_module";
            case object_type::pipeline_cache:            return "pipeline_cache";
            case object_type::pipeline_layout:           return "pipeline_layout";
            case object_type::render_pass:               return "render_pass";
            case object_type::pipeline:                  return "pipeline";
            case object_type::descriptor_set_layout:     return "descriptor_set_layout";
            case object_type::sampler:                   return "sampler";
            case object_type::descriptor_pool:           return "descriptor_pool";
            case object_type::descriptor_set:            return "descriptor_set";
            case object_type::framebuffer:               return "framebuffer";
            case object_type::command_pool:              return "command_pool";
            case object_type::sampler_ycbcr_conversion:  return "sampler_ycbcr_conversion";
            case object_type::surface:                   return "surface";
            case object_type::swapchain:                 return "swapchain";
            case object_type::debug_report_callback:     return "debug_report_callback";
            case object_type::debug_messenger:           return "debug_messenger";
            default: return "unknown";
        }
    };

    const auto format_message = [format_color, format_object_type](const debug_message_data& data) -> std::string
    {
        std::string message{};
        message.reserve(1024 * 8);

        message += "  Message name: ";
        message += data.message_name;
        message += '\n';
        message += "  Message ID: ";
        message += std::to_string(data.message_id);
        message += '\n';
        message += "  Message: ";
        message += data.message;
        message += '\n';

        message += "  Active queues:\n";
        for(auto&& queue : data.queue_labels)
        {
            message += "    ";
            message += queue.name;
            message += " (#";
            message += format_color(queue.color);
            message += ")\n";
        }

        message += "  Active command buffers:\n";
        for(auto&& command_buffer : data.command_buffer_labels)
        {
            message += "    ";
            message += command_buffer.name;
            message += " (#";
            message += format_color(command_buffer.color);
            message += ")\n";
        }

        message += "  Related object (from more important to less important)\n";
        for(auto&& object : data.objects)
        {
            message += "    ";
            message += format_object_type(object.type);
            message += " (";
            message += std::to_string(static_cast<std::uint32_t>(object.type));
            message += ") \"";
            message += object.name;
            message += "\" (";
            message += std::to_string(object.handle);
            message += ")\n";
        }

        return message;
    };

    std::string message{};
    message.reserve(1024 * 8);

    message += "Debug messenger message:\n";
    message += format_type(type);
    message += format_severity(severity);
    message += format_message(data);

    static std::mutex mutex{};
    std::lock_guard lock{mutex};

    if(severity == debug_message_severity::error)
    {
        std::cerr << message << std::endl;
        assert(false && "Validation error.");
    }
    else
    {
        std::cout << message << std::endl;
    }
}

debug_messenger::debug_messenger(application& app, callback_type callback, debug_message_severity severities, debug_message_type types)
:m_callback{std::make_unique<callback_type>(std::move(callback))}
,m_debug_messenger{underlying_cast<VkInstance>(app), &debug_messenger_callback, static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(severities), static_cast<VkDebugUtilsMessageTypeFlagBitsEXT>(types), m_callback.get()}
{

}

}
