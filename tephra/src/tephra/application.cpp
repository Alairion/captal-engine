#include "application.hpp"

#include <algorithm>
#include <iostream>
#include <cassert>

#include "vulkan/vulkan_functions.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

tph::version enumerate_instance_version()
{
    tph::vulkan::functions::load_external_level_functions();
    tph::vulkan::functions::load_global_level_functions();

    std::uint32_t native_version{};

    if(const auto result{vkEnumerateInstanceVersion(&native_version)}; result != VK_SUCCESS)
        throw vulkan::error{result};

    tph::version output{};
    output.major = static_cast<std::uint16_t>(VK_VERSION_MAJOR(native_version));
    output.minor = static_cast<std::uint16_t>(VK_VERSION_MINOR(native_version));
    output.patch = VK_VERSION_PATCH(native_version);

    return output;
}

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

static std::vector<VkExtensionProperties> available_instance_extensions(const std::vector<const char*>& layers)
{
    std::vector<VkExtensionProperties> extensions{};

    std::uint32_t count{};
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    extensions.resize(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, std::data(extensions));

    for(auto layer : layers)
    {
        std::vector<VkExtensionProperties> layer_extensions{};

        vkEnumerateInstanceExtensionProperties(layer, &count, nullptr);
        layer_extensions.resize(count);
        vkEnumerateInstanceExtensionProperties(layer, &count, std::data(layer_extensions));

        extensions.insert(std::cend(extensions), std::cbegin(layer_extensions), std::cend(layer_extensions));
    }

    return extensions;
}

static std::vector<const char*> filter_instance_extensions(std::vector<const char*> extensions, const std::vector<const char*>& layers)
{
    const std::vector<VkExtensionProperties> available{available_instance_extensions(layers)};

    for(auto it{std::cbegin(extensions)}; it != std::cend(extensions); ++it)
    {
        const auto pred = [it](const VkExtensionProperties& ext)
        {
            return std::string_view{ext.extensionName} == *it;
        };

        if(std::find_if(std::begin(available), std::end(available), pred) == std::end(available))
        {
            std::cerr << "Extension \"" << *it << "\" is not available." << std::endl;
            it = extensions.erase(it);
        }
    }

    return extensions;
}

static std::vector<const char*> required_instance_extensions(application_options options, const std::vector<const char*>& layers)
{
    std::vector<const char*> extensions{"VK_KHR_surface"};

    if(static_cast<bool>(options & application_options::enable_validation))
    {
        extensions.emplace_back("VK_EXT_debug_utils");
    }

#ifdef TPH_PLATFORM_ANDROID
    extensions.emplace_back("VK_KHR_android_surface");
#endif
#ifdef TPH_PLATFORM_IOS
    extensions.emplace_back("VK_MVK_ios_surface");
#endif
#ifdef TPH_PLATFORM_WIN32
    extensions.emplace_back("VK_KHR_win32_surface");
#endif
#ifdef TPH_PLATFORM_MACOS
    extensions.emplace_back("VK_MVK_macos_surface");
#endif
#ifdef TPH_PLATFORM_XLIB
    extensions.emplace_back("VK_KHR_xlib_surface");
#endif
#ifdef TPH_PLATFORM_XCB
    extensions.emplace_back("VK_KHR_xcb_surface");
#endif
#ifdef TPH_PLATFORM_WAYLAND
    extensions.emplace_back("VK_KHR_wayland_surface");
#endif

    return filter_instance_extensions(std::move(extensions), layers);
}

static std::vector<VkLayerProperties> available_instance_layers()
{
    std::vector<VkLayerProperties> extensions{};

    std::uint32_t count{};
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    extensions.resize(count);
    vkEnumerateInstanceLayerProperties(&count, std::data(extensions));

    return extensions;
}

static std::vector<const char*> filter_instance_layers(std::vector<const char*> layers)
{
    const std::vector<VkLayerProperties> available{available_instance_layers()};

    for(auto it{std::cbegin(layers)}; it != std::cend(layers); ++it)
    {
        const auto pred = [it](const VkLayerProperties& layer)
        {
            return std::string_view{layer.layerName} == *it;
        };

        if(std::find_if(std::begin(available), std::end(available), pred) == std::end(available))
        {
            std::cerr << "Layer \"" << *it << "\" is not available." << std::endl;
            it = layers.erase(it);
        }
    }

    return layers;
}

static std::vector<const char*> required_instance_layers(application_options options)
{
    std::vector<const char*> layers{};

    if(static_cast<bool>(options & application_options::enable_validation))
    {
       layers.emplace_back("VK_LAYER_LUNARG_standard_validation");
    }

    return filter_instance_layers(std::move(layers));
}

static std::vector<physical_device> make_physical_devices(VkInstance instance)
{
    std::vector<VkPhysicalDevice> vulkan_devices{};

    std::uint32_t count{};
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    vulkan_devices.resize(count);
    vkEnumeratePhysicalDevices(instance, &count, std::data(vulkan_devices));

    std::vector<physical_device> devices{};
    devices.reserve(count);

    for(auto&& vulkan_device : vulkan_devices)
    {
        devices.emplace_back(vulkan::make_physical_device(vulkan_device));
    }

    return devices;
}

application::application(const std::string& application_name, version application_version, application_options options)
:application{application_name, application_version, enumerate_instance_version(), options}
{

}

application::application(const std::string& application_name, version application_version, version api_version, application_options options)
:m_options{options}
,m_version{api_version}
{
    tph::vulkan::functions::load_external_level_functions();
    tph::vulkan::functions::load_global_level_functions();

    const std::vector<const char*> layers{required_instance_layers(m_options)};
    const std::vector<const char*> extensions{required_instance_extensions(m_options, layers)};

    m_instance = vulkan::instance{application_name, application_version, api_version, extensions, layers};

    tph::vulkan::functions::load_instance_level_functions(m_instance);

    if(static_cast<bool>(m_options & application_options::enable_validation) || static_cast<bool>(m_options & application_options::enable_verbose_validation))
    {
        constexpr auto severities{VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};
        constexpr auto verbose_severities{VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        constexpr auto types{VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT};
        constexpr auto verbose_types{VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT};

        if(static_cast<bool>(m_options & application_options::enable_verbose_validation))
        {
            m_debug_messenger = vulkan::debug_messenger{m_instance, debug_messenger_callback, static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(severities | verbose_severities), static_cast<VkDebugUtilsMessageTypeFlagBitsEXT>(types | verbose_types)};
        }
        else
        {
            m_debug_messenger = vulkan::debug_messenger{m_instance, debug_messenger_callback, static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(severities), static_cast<VkDebugUtilsMessageTypeFlagBitsEXT>(types)};
        }
    }

    m_physical_devices = make_physical_devices(m_instance);
}

application::application(vulkan::instance instance, tph::version api_version, vulkan::debug_messenger debug_messenger)
:m_instance{std::move(instance)}
,m_debug_messenger{std::move(debug_messenger)}
,m_version{api_version}
{
    tph::vulkan::functions::load_external_level_functions();
    tph::vulkan::functions::load_global_level_functions();
    tph::vulkan::functions::load_instance_level_functions(m_instance);

    m_physical_devices = make_physical_devices(m_instance);
}

const physical_device& application::select_physical_device(const filter_type& required, const comparator_type& comparator) const
{
    std::vector<std::reference_wrapper<const physical_device>> suitable_devices{};

    for(const auto& device : m_physical_devices)
    {
        if(required(device))
        {
            suitable_devices.emplace_back(device);
        }
    }

    if(std::empty(suitable_devices))
    {
        throw std::runtime_error{"Can not find any suitable device."};
    }

    if(comparator)
    {
        std::sort(std::begin(suitable_devices), std::end(suitable_devices), comparator);
    }

    return suitable_devices[0];
}

const physical_device& application::default_physical_device() const
{
    const auto requirements = [](const physical_device&) -> bool
    {
        return true;
    };

    return select_physical_device(requirements, default_physical_device_comparator);
}

}
