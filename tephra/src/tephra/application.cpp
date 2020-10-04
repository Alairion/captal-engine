#include "application.hpp"

#include <algorithm>
#include <iostream>
#include <cassert>

#include "vulkan/vulkan_functions.hpp"

#include "renderer.hpp"

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

static std::vector<VkLayerProperties> available_instance_layers()
{
    std::vector<VkLayerProperties> layers{};

    std::uint32_t count{};
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    layers.resize(count);
    vkEnumerateInstanceLayerProperties(&count, std::data(layers));

    return layers;
}

static application_layer layer_from_name(std::string_view name) noexcept
{
    if(name == "VK_LAYER_KHRONOS_validation")
    {
        return application_layer::validation;
    }

    return application_layer::none;
}

static std::vector<const char*> filter_instance_layers(std::vector<const char*> layers, application_layer& layer_bits)
{
    const std::vector<VkLayerProperties> available{available_instance_layers()};

    for(auto it{std::cbegin(layers)}; it != std::cend(layers);)
    {
        const auto pred = [it](const VkLayerProperties& layer)
        {
            return std::string_view{layer.layerName} == *it;
        };

        if(std::find_if(std::begin(available), std::end(available), pred) == std::end(available))
        {
            #ifndef NDEBUG
            std::cerr << "Instance layer \"" << *it << "\" is not available." << std::endl;
            #endif

            layer_bits &= ~layer_from_name(*it);
            it = layers.erase(it);
        }
        else
        {
            ++it;
        }
    }

    return layers;
}

static std::vector<const char*> required_instance_layers(application_layer& layers)
{
    std::vector<const char*> output{};

    //output indices must correspond to application_layer bit indices
    if(static_cast<bool>(layers & application_layer::validation))
    {
        output.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    return filter_instance_layers(std::move(output), layers);
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

static application_extension extension_from_name(std::string_view name) noexcept
{
    if(name == "VK_KHR_surface")
    {
        return application_extension::surface;
    }
    else if(name == "VK_EXT_debug_utils")
    {
        return application_extension::debug_utils;
    }

    return application_extension::none;
}

static std::vector<const char*> filter_instance_extensions(const std::vector<const char*>& layers, std::vector<const char*> extensions, application_extension& extension_bits)
{
    const std::vector<VkExtensionProperties> available{available_instance_extensions(layers)};

    for(auto it{std::cbegin(extensions)}; it != std::cend(extensions);)
    {
        const auto pred = [it](const VkExtensionProperties& ext)
        {
            return std::string_view{ext.extensionName} == *it;
        };

        if(std::find_if(std::begin(available), std::end(available), pred) == std::end(available))
        {
            #ifndef NDEBUG
            std::cerr << "Instance extension \"" << *it << "\" is not available." << std::endl;
            #endif

            extension_bits &= ~extension_from_name(*it);
            it = extensions.erase(it);
        }
        else
        {
            ++it;
        }
    }

    return extensions;
}

static std::vector<const char*> required_instance_extensions(const std::vector<const char*>& layers, application_extension& extensions)
{
    std::vector<const char*> output{};

    if(static_cast<bool>(extensions & application_extension::debug_utils))
    {
        output.emplace_back("VK_EXT_debug_utils");
    }

    if(static_cast<bool>(extensions & application_extension::surface))
    {
        output.emplace_back("VK_KHR_surface");

        #ifdef TPH_PLATFORM_ANDROID
        output.emplace_back("VK_KHR_android_surface");
        #endif

        #ifdef TPH_PLATFORM_IOS
        output.emplace_back("VK_MVK_ios_surface");
        #endif

        #ifdef TPH_PLATFORM_WIN32
        output.emplace_back("VK_KHR_win32_surface");
        #endif

        #ifdef TPH_PLATFORM_MACOS
        output.emplace_back("VK_MVK_macos_surface");
        #endif

        #ifdef TPH_PLATFORM_XLIB
        output.emplace_back("VK_KHR_xlib_surface");
        #endif

        #ifdef TPH_PLATFORM_XCB
        output.emplace_back("VK_KHR_xcb_surface");
        #endif

        #ifdef TPH_PLATFORM_WAYLAND
        output.emplace_back("VK_KHR_wayland_surface");
        #endif
    }

    return filter_instance_extensions(layers, std::move(output), extensions);
}

static std::vector<physical_device> make_physical_devices(VkInstance instance, tph::version version)
{
    std::vector<VkPhysicalDevice> native_devices{};

    std::uint32_t count{};
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    native_devices.resize(count);
    vkEnumeratePhysicalDevices(instance, &count, std::data(native_devices));

    std::vector<physical_device> devices{};
    devices.reserve(count);

    for(auto&& native_device : native_devices)
    {
        devices.emplace_back(vulkan::make_physical_device(native_device, version));
    }

    return devices;
}

application::application(const std::string& application_name, version application_version, application_layer layers, application_extension extensions)
:application{application_name, application_version, enumerate_instance_version(), layers, extensions}
{

}

application::application(const std::string& application_name, version application_version, version api_version, application_layer layers, application_extension extensions)
:m_version{api_version}
{
    tph::vulkan::functions::load_external_level_functions();
    tph::vulkan::functions::load_global_level_functions();

    const std::vector<const char*> layer_names{required_instance_layers(layers)};
    const std::vector<const char*> extension_names{required_instance_extensions(layer_names, extensions)};

    m_instance = vulkan::instance{application_name, application_version, api_version, layer_names, extension_names};
    m_layers = layers;
    m_extensions = extensions;

    tph::vulkan::functions::load_instance_level_functions(m_instance);

    m_physical_devices = make_physical_devices(m_instance, m_version);
}

application::application(vulkan::instance instance, tph::version api_version, application_layer layers, application_extension extensions)
:m_instance{std::move(instance)}
,m_version{api_version}
,m_layers{layers}
,m_extensions{extensions}
{
    tph::vulkan::functions::load_external_level_functions();
    tph::vulkan::functions::load_global_level_functions();
    tph::vulkan::functions::load_instance_level_functions(m_instance);

    m_physical_devices = make_physical_devices(m_instance, m_version);
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
