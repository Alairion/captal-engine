#include <iostream>
#include <vector>
#include <array>
#include <thread>
#include <string>
#include <algorithm>
#include <set>
#include <fstream>
#include <random>

#include <SDL.h>
#include <SDL_vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <vulkan/vulkan.h>

static constexpr std::array validation_layers
{
    u8"VK_LAYER_LUNARG_standard_validation"
};

static constexpr std::array device_extensions
{
     VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
    static constexpr bool enable_validation_layers = false;
#else
    static constexpr bool enable_validation_layers = true;
#endif

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback_func(VkDebugUtilsMessageSeverityFlagBitsEXT  severity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void*)
{
    if(severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        std::cerr << "Error:   " << callback_data->pMessage << std::endl;
    else if(severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        std::cerr << "Warning: " << callback_data->pMessage << std::endl;
    else
        std::cout << "Info:    " << callback_data->pMessage << std::endl;

    return VK_FALSE;
}

struct vertex
{
    glm::vec2 pos{};
    glm::vec3 color{};
    glm::vec2 texture_pos{};

    static constexpr VkVertexInputBindingDescription binding_description() noexcept
    {
        return {0, sizeof(vertex), VK_VERTEX_INPUT_RATE_VERTEX};
    }

    static constexpr auto attribute_description() noexcept
    {
        return std::array
        {
            VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex, pos)},
            VkVertexInputAttributeDescription{1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, color)},
            VkVertexInputAttributeDescription{2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex, texture_pos)}
        };
    }
};

struct uniform_buffer_object
{
    glm::mat4 model{};
    glm::mat4 view{};
    glm::mat4 proj{};
};

std::vector<std::uint8_t> read_file(const std::string& filename)
{
    std::ifstream file{filename, std::ios_base::ate | std::ios_base::binary};

    if(!file)
        throw std::runtime_error{"Failed to open file!"};

    std::size_t size = file.tellg();
    file.seekg(0, std::ios_base::beg);

    std::vector<std::uint8_t> data{};
    data.resize(size);
    file.read(reinterpret_cast<char*>(std::data(data)), std::size(data));

    return data;
}

std::uint32_t find_memory_type(VkPhysicalDevice phydev, std::uint32_t type, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties mem_properties{};
    vkGetPhysicalDeviceMemoryProperties(phydev, &mem_properties);

    for(std::uint32_t i{}; i < mem_properties.memoryTypeCount; ++i)
        if(type & (1 << i) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;

    throw std::runtime_error{"Can not find a suitable memory type."};
}

std::size_t required_memory_size(VkDevice dev, VkBuffer buf)
{
    VkMemoryRequirements mem_requirements{};
    vkGetBufferMemoryRequirements(dev, buf, &mem_requirements);
    return mem_requirements.size;
}

std::size_t required_memory_alignement(VkDevice dev, VkBuffer buf)
{
    VkMemoryRequirements mem_requirements{};
    vkGetBufferMemoryRequirements(dev, buf, &mem_requirements);
    return mem_requirements.alignment;
}

std::uint32_t required_memory_type(VkDevice dev, VkBuffer buf)
{
    VkMemoryRequirements mem_requirements{};
    vkGetBufferMemoryRequirements(dev, buf, &mem_requirements);
    return mem_requirements.memoryTypeBits;
}

std::size_t required_memory_size(VkDevice dev, VkImage img)
{
    VkMemoryRequirements mem_requirements{};
    vkGetImageMemoryRequirements(dev, img, &mem_requirements);
    return mem_requirements.size;
}

std::size_t required_memory_alignement(VkDevice dev, VkImage img)
{
    VkMemoryRequirements mem_requirements{};
    vkGetImageMemoryRequirements(dev, img, &mem_requirements);
    return mem_requirements.alignment;
}

std::uint32_t required_memory_type(VkDevice dev, VkImage img)
{
    VkMemoryRequirements mem_requirements{};
    vkGetImageMemoryRequirements(dev, img, &mem_requirements);
    return mem_requirements.memoryTypeBits;
}

std::vector<VkDescriptorSetLayoutBinding> make_descriptor_set_layout_bindings()
{
    return std::vector
    {
        VkDescriptorSetLayoutBinding{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        VkDescriptorSetLayoutBinding{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}
    };
}

class window
{
public:
    window()
    {
        if(SDL_Init(SDL_INIT_VIDEO) != 0)
            throw std::runtime_error{"Can not initialize SDL2: " + std::string{SDL_GetError()}};

        if(m_window = SDL_CreateWindow(u8"<(:3", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480, 480, SDL_WINDOW_VULKAN); !m_window)
        {
            SDL_Quit();
            throw std::runtime_error{"Can not create window: " + std::string{SDL_GetError()}};
        }
    }

    ~window()
    {
        SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    operator SDL_Window*() const noexcept
    {
        return m_window;
    }

private:
    SDL_Window* m_window{};
};

class instance
{
public:
    instance(window& win)
    {
        if constexpr(enable_validation_layers)
        {
            if(!check_validation_layer_support())
                throw std::runtime_error{"Validation layers are not available."};
        }

        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = u8"Ping";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = u8"Oui";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_1;

        auto extensions = get_extensions(win);

        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = std::size(extensions);
        create_info.ppEnabledExtensionNames = std::data(extensions);

        if constexpr(enable_validation_layers)
        {
            create_info.enabledLayerCount = static_cast<uint32_t>(std::size(validation_layers));
            create_info.ppEnabledLayerNames = std::data(validation_layers);
        }

        if(vkCreateInstance(&create_info, nullptr, &m_inst) != VK_SUCCESS)
            throw std::runtime_error{"Failed to create instance."};
    }

    ~instance()
    {
        vkDestroyInstance(m_inst, nullptr);
    }

    instance(const instance&) = delete;
    instance& operator=(const instance&) = delete;
    instance(instance&&) noexcept = delete;
    instance& operator=(instance&&) noexcept = delete;

    bool check_validation_layer_support()
    {
        std::uint32_t layers_count{};
        vkEnumerateInstanceLayerProperties(&layers_count, nullptr);
        std::vector<VkLayerProperties> layers{};
        layers.resize(layers_count);
        vkEnumerateInstanceLayerProperties(&layers_count, std::data(layers));

        for(auto&& layer : validation_layers)
        {
            bool found{};

            for(auto&& properties : layers)
            {
                if(std::string_view{layer} == properties.layerName)
                {
                    found = true;
                    break;
                }
            }

            if(!found)
                return false;
        }

        return true;
    }

    operator VkInstance() const noexcept
    {
        return m_inst;
    }

private:
    std::vector<const char*> get_extensions(SDL_Window* win)
    {
        std::uint32_t extensions_count{};
        SDL_Vulkan_GetInstanceExtensions(win, &extensions_count, nullptr);
        std::vector<const char*> extensions{};
        extensions.resize(extensions_count);
        SDL_Vulkan_GetInstanceExtensions(win, &extensions_count, std::data(extensions));

        if constexpr(enable_validation_layers)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    VkInstance m_inst{};
};

class debug_callback
{
public:
    debug_callback(instance& inst)
    :m_inst{inst}
    {
        VkDebugUtilsMessengerCreateInfoEXT create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.pfnUserCallback = debug_callback_func;

        if(auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(inst, "vkCreateDebugUtilsMessengerEXT")); func)
            func(inst, &create_info, nullptr, &m_cb);
        else
            std::runtime_error{"Can not create debug callback."};
    }

    ~debug_callback()
    {
        if(auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_inst, "vkDestroyDebugUtilsMessengerEXT")); func)
            func(m_inst, m_cb, nullptr);
    }

private:
    VkDebugUtilsMessengerEXT m_cb{};
    VkInstance m_inst{};
};

class surface
{
public:
    surface(instance& inst, window& win)
    :m_inst{inst}
    {
        SDL_Vulkan_CreateSurface(win, inst, &m_surf);
    }

    ~surface()
    {
        vkDestroySurfaceKHR(m_inst, m_surf, nullptr);
    }

    operator VkSurfaceKHR() const noexcept
    {
        return m_surf;
    }

private:
    VkInstance m_inst{};
    VkSurfaceKHR m_surf{};
};

class physical_device
{
public:
    physical_device(instance& inst)
    {
        std::uint32_t count{};
        vkEnumeratePhysicalDevices(inst, &count, nullptr);

        if(count == 0)
            throw std::runtime_error{"No available physical device."};

        std::vector<VkPhysicalDevice> phydevs{};
        phydevs.resize(count);
        vkEnumeratePhysicalDevices(inst, &count, std::data(phydevs));

        auto is_suitable = [](VkPhysicalDevice phydev) noexcept -> bool
        {
            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(phydev, &features);

            return features.geometryShader;
        };

        auto support_extensions = [](VkPhysicalDevice device)
        {
            std::uint32_t count;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
            std::vector<VkExtensionProperties> extensions{};
            extensions.resize(count);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &count, std::data(extensions));

            std::set<std::string> required_extensions(std::begin(device_extensions), std::end(device_extensions));
            for(auto&& extension : extensions)
                required_extensions.erase(extension.extensionName);

            return std::empty(required_extensions);
        };

        for(const auto& phydev : phydevs)
        {
            if(is_suitable(phydev) && support_extensions(phydev))
            {
                m_phydev = phydev;
                break;
            }
        }

        if(!m_phydev)
            throw std::runtime_error{"Can not find any suitable physical device."};
    }

    operator VkPhysicalDevice() const noexcept
    {
        return m_phydev;
    }

private:
    VkPhysicalDevice m_phydev{};
};

struct queue_families
{
    std::uint32_t graphics{std::numeric_limits<std::uint32_t>::max()};
    std::uint32_t transfer{std::numeric_limits<std::uint32_t>::max()};
    std::uint32_t present{std::numeric_limits<std::uint32_t>::max()};
};

class device
{
public:
    device(physical_device& phydev, surface& surf)
    :m_phydev{phydev}
    {
        choose_queue_families(surf);

        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        VkPhysicalDeviceFeatures features{};
        features.wideLines = VK_TRUE;
        features.fillModeNonSolid = VK_TRUE;
        create_info.pEnabledFeatures = &features;

        if constexpr(enable_validation_layers)
        {
            create_info.enabledLayerCount = std::size(validation_layers);
            create_info.ppEnabledLayerNames = std::data(validation_layers);
        }

        create_info.enabledExtensionCount = std::size(device_extensions);
        create_info.ppEnabledExtensionNames = std::data(device_extensions);

        float priority{1.f};
        if(m_qf.graphics == m_qf.transfer && m_qf.graphics == m_qf.present)
        {
            VkDeviceQueueCreateInfo queue_info{};
            queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info.queueFamilyIndex = m_qf.graphics;
            queue_info.queueCount = 1;
            queue_info.pQueuePriorities = &priority;

            create_info.pQueueCreateInfos = &queue_info;
            create_info.queueCreateInfoCount = 1;

            if(vkCreateDevice(phydev, &create_info, nullptr, &m_dev) != VK_SUCCESS)
                throw std::runtime_error{"Can not create logical device."};
        }
        else if(m_qf.graphics == m_qf.present || m_qf.transfer == m_qf.present)
        {
            std::array<VkDeviceQueueCreateInfo, 2> queue_info{};

            queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info[0].queueFamilyIndex = m_qf.graphics;
            queue_info[0].queueCount = 1;
            queue_info[0].pQueuePriorities = &priority;

            queue_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info[1].queueFamilyIndex = m_qf.transfer;
            queue_info[1].queueCount = 1;
            queue_info[1].pQueuePriorities = &priority;

            create_info.pQueueCreateInfos = std::data(queue_info);
            create_info.queueCreateInfoCount = std::size(queue_info);

            if(vkCreateDevice(phydev, &create_info, nullptr, &m_dev) != VK_SUCCESS)
                throw std::runtime_error{"Can not create logical device."};
        }
        else
        {
            std::array<VkDeviceQueueCreateInfo, 3> queue_info{};

            queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info[0].queueFamilyIndex = m_qf.graphics;
            queue_info[0].queueCount = 1;
            queue_info[0].pQueuePriorities = &priority;

            queue_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info[1].queueFamilyIndex = m_qf.transfer;
            queue_info[1].queueCount = 1;
            queue_info[1].pQueuePriorities = &priority;

            queue_info[2].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info[2].queueFamilyIndex = m_qf.present;
            queue_info[2].queueCount = 1;
            queue_info[2].pQueuePriorities = &priority;

            create_info.pQueueCreateInfos = std::data(queue_info);
            create_info.queueCreateInfoCount = std::size(queue_info);

            if(vkCreateDevice(phydev, &create_info, nullptr, &m_dev) != VK_SUCCESS)
                throw std::runtime_error{"Can not create logical device."};
        }

        if(m_qf.graphics == m_qf.transfer && m_qf.graphics == m_qf.present)
        {
            vkGetDeviceQueue(m_dev, m_qf.graphics, 0, &m_graphics);
            vkGetDeviceQueue(m_dev, m_qf.graphics, 0, &m_transfer);
            vkGetDeviceQueue(m_dev, m_qf.graphics, 0, &m_present);
        }
        else if(m_qf.graphics == m_qf.present || m_qf.transfer == m_qf.present)
        {
            vkGetDeviceQueue(m_dev, m_qf.graphics, 0, &m_graphics);
            vkGetDeviceQueue(m_dev, m_qf.transfer, 0, &m_transfer);
            if(m_qf.graphics == m_qf.present)
                vkGetDeviceQueue(m_dev, m_qf.graphics, 0, &m_present);
            else
                vkGetDeviceQueue(m_dev, m_qf.transfer, 0, &m_present);
        }
        else
        {
            vkGetDeviceQueue(m_dev, m_qf.graphics, 0, &m_graphics);
            vkGetDeviceQueue(m_dev, m_qf.transfer, 0, &m_transfer);
            vkGetDeviceQueue(m_dev, m_qf.present, 0, &m_present);
        }
    }

    ~device()
    {
        vkDeviceWaitIdle(m_dev);
        vkDestroyDevice(m_dev, nullptr);
    }

    device(const device&) = delete;
    device& operator=(const device&) = delete;
    device(device&&) noexcept = delete;
    device& operator=(device&&) noexcept = delete;

    VkQueue graphics_queue() const noexcept
    {
        return m_graphics;
    }

    std::uint32_t graphics_family() const noexcept
    {
        return m_qf.graphics;
    }

    VkQueue transfer_queue() const noexcept
    {
        return m_transfer;
    }

    std::uint32_t transfer_family() const noexcept
    {
        return m_qf.transfer;
    }

    VkQueue present_queue() const noexcept
    {
        return m_present;
    }

    std::uint32_t present_family() const noexcept
    {
        return m_qf.present;
    }

    VkPhysicalDevice physical_device() const noexcept
    {
        return m_phydev;
    }

    operator VkDevice() const noexcept
    {
        return m_dev;
    }

private:
    void choose_queue_families(surface& surf)
    {
        std::uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_phydev, &count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(count);
        vkGetPhysicalDeviceQueueFamilyProperties(m_phydev, &count, std::data(queue_families));

        m_qf.graphics = choose_queue_family(queue_families, VK_QUEUE_GRAPHICS_BIT);
        m_qf.transfer = choose_queue_family(queue_families, VK_QUEUE_TRANSFER_BIT);

        VkBool32 support{};
        if(vkGetPhysicalDeviceSurfaceSupportKHR(m_phydev, m_qf.graphics, surf, &support); support == VK_TRUE)
        {
            m_qf.present = m_qf.graphics;
        }
        else if(vkGetPhysicalDeviceSurfaceSupportKHR(m_phydev, m_qf.transfer, surf, &support); support == VK_TRUE)
        {
            m_qf.present = m_qf.transfer;
        }
        else
        {
            for(std::uint32_t i{}; i < count; ++i)
            {
                if(vkGetPhysicalDeviceSurfaceSupportKHR(m_phydev, i, surf, &support); support == VK_TRUE)
                {
                    m_qf.present = i;
                    break;
                }
            }
        }
    }

    std::uint32_t choose_queue_family(const std::vector<VkQueueFamilyProperties>& queue_families, VkQueueFlagBits flags)
    {
        std::uint32_t out{std::numeric_limits<std::uint32_t>::max()};

        std::uint32_t i{};
        for(const auto& family : queue_families) //Search for a specilized queue (which don't have more that we need)
        {
            if(family.queueCount > 0 && family.queueFlags == flags)
            {
                out = i;
                break;
            }

            ++i;
        }

        if(out == std::numeric_limits<std::uint32_t>::max()) //Otherwise use a polyvalant queue family thaht supports what we want.
        {
            std::uint32_t j{};
            for(const auto& family : queue_families)
            {
                if(family.queueCount > 0 && (family.queueFlags & flags) != 0)
                {
                    out = j;
                    break;
                }

                ++j;
            }
        }

        return out;
    }

    VkPhysicalDevice m_phydev{};
    queue_families m_qf{};
    VkDevice m_dev{};
    VkQueue m_graphics{};
    VkQueue m_transfer{};
    VkQueue m_present{};
};

class swap_chain
{
    struct swap_chain_support_details
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR> present_modes{};
    };

public:
    swap_chain(device& dev, physical_device& phydev, window& win, surface& surf, std::uint32_t image_count = 2)
    :m_dev{dev}
    {
        swap_chain_support_details details = query_swap_chain_details(phydev, surf);

        VkSurfaceFormatKHR format = choose_format(details.formats);
        VkPresentModeKHR present_mode = choose_present_mode(details.present_modes);
        VkExtent2D extent = choose_extent(details.capabilities, win);

        image_count = std::clamp(image_count, details.capabilities.minImageCount, details.capabilities.maxImageCount);

        VkSwapchainCreateInfoKHR create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = surf;
        create_info.minImageCount = image_count;
        create_info.imageFormat = format.format;
        create_info.imageColorSpace = format.colorSpace;
        create_info.imageExtent = extent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        create_info.preTransform = details.capabilities.currentTransform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = present_mode;
        create_info.clipped = VK_TRUE;
        create_info.oldSwapchain = VK_NULL_HANDLE;

        std::uint32_t graphics = dev.graphics_family();
        std::uint32_t present = dev.present_family();

        if(graphics == present)
        {
            create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if(vkCreateSwapchainKHR(dev, &create_info, nullptr, &m_swap_chain) != VK_SUCCESS)
                throw std::runtime_error{"failed to create swap chain!"};
        }
        else
        {
            std::array queue_families{graphics, present};

            create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices = std::data(queue_families);

            if(vkCreateSwapchainKHR(dev, &create_info, nullptr, &m_swap_chain) != VK_SUCCESS)
                throw std::runtime_error{"failed to create swap chain!"};
        }

        vkGetSwapchainImagesKHR(dev, m_swap_chain, &image_count, nullptr);
        m_images.resize(image_count);
        vkGetSwapchainImagesKHR(dev, m_swap_chain, &image_count, std::data(m_images));
        m_format = format.format;
        m_extent = extent;

        create_image_views();
    }

    ~swap_chain()
    {
        if(m_swap_chain)
        {
            vkDestroySwapchainKHR(m_dev, m_swap_chain, nullptr);
            for(auto image_view : m_image_views)
                vkDestroyImageView(m_dev, image_view, nullptr);
        }
    }

    swap_chain(const swap_chain&) = delete;
    swap_chain& operator=(const swap_chain&) = delete;

    swap_chain(swap_chain&& other) noexcept
    :m_dev{other.m_dev}
    ,m_swap_chain{std::exchange(other.m_swap_chain, nullptr)}
    ,m_images{std::move(other.m_images)}
    ,m_image_views{std::move(other.m_image_views)}
    ,m_format{other.m_format}
    ,m_extent{other.m_extent}
    {

    }

    swap_chain& operator=(swap_chain&& other) noexcept
    {
        m_dev = other.m_dev;
        m_swap_chain = std::exchange(other.m_swap_chain, nullptr);
        m_images = std::move(other.m_images);
        m_image_views = std::move(other.m_image_views);
        m_format = other.m_format;
        m_extent = other.m_extent;

        return *this;
    }

    operator VkSwapchainKHR() const noexcept
    {
        return m_swap_chain;
    }

    VkFormat format() const noexcept
    {
        return m_format;
    }

    VkExtent2D extent() const noexcept
    {
        return m_extent;
    }

    std::vector<VkImage> images() const noexcept
    {
        return m_images;
    }

    std::vector<VkImageView> image_views() const noexcept
    {
        return m_image_views;
    }

    std::uint32_t image_count() const noexcept
    {
        return std::size(m_images);
    }

private:
    swap_chain_support_details query_swap_chain_details(physical_device& phydev, surface& surf)
    {
        swap_chain_support_details details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phydev, surf, &details.capabilities);

        std::uint32_t format_count{};
        vkGetPhysicalDeviceSurfaceFormatsKHR(phydev, surf, &format_count, nullptr);
        if(format_count)
        {
            details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(phydev, surf, &format_count, std::data(details.formats));
        }

        std::uint32_t present_mode_count{};
        vkGetPhysicalDeviceSurfacePresentModesKHR(phydev, surf, &present_mode_count, nullptr);
        if(present_mode_count)
        {
            details.present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(phydev, surf, &present_mode_count, std::data(details.present_modes));
        }

        return details;
    }

    VkSurfaceFormatKHR choose_format(const std::vector<VkSurfaceFormatKHR>& formats) noexcept
    {
        if(std::size(formats) == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
            return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

        for(auto&& format : formats)
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return format;

        return formats[0];
    }

    VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR>& present_modes [[maybe_unused]]) noexcept
    {
        VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
/*
        for(auto&& present_mode : present_modes)
        {
            if(present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
                return present_mode;

            if(present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
                mode = present_mode;
        }*/

        return mode;
    }

    VkExtent2D choose_extent(const VkSurfaceCapabilitiesKHR& capabilities, window& win)
    {
        if(capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            std::int32_t width{};
            std::int32_t height{};
            SDL_GetWindowSize(win, &width, &height);
            VkExtent2D extent{static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)};

            extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return extent;
        }
    }

    void create_image_views()
    {
        m_image_views.resize(std::size(m_images));

        for(std::size_t i{}; i < std::size(m_images); ++i)
        {
            VkImageViewCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            create_info.image = m_images[i];
            create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            create_info.format = m_format;
            create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            create_info.subresourceRange.baseMipLevel = 0;
            create_info.subresourceRange.levelCount = 1;
            create_info.subresourceRange.baseArrayLayer = 0;
            create_info.subresourceRange.layerCount = 1;

            if(vkCreateImageView(m_dev, &create_info, nullptr, &m_image_views[i]) != VK_SUCCESS)
                throw std::runtime_error{"failed to create image views!"};
        }
    }

    VkDevice m_dev{};
    VkSwapchainKHR m_swap_chain{};
    std::vector<VkImage> m_images{};
    std::vector<VkImageView> m_image_views{};
    VkFormat m_format{};
    VkExtent2D m_extent{};
};

class shader
{
public:
    shader(device& dev, const std::vector<std::uint8_t>& code)
    :m_dev{dev}
    {
        VkShaderModuleCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = std::size(code);
        create_info.pCode = reinterpret_cast<const uint32_t*>(std::data(code));

        if(vkCreateShaderModule(dev, &create_info, nullptr, &m_shader) != VK_SUCCESS)
            throw std::runtime_error{"Failed to create shader module."};
    }

    ~shader()
    {
        if(m_shader)
            vkDestroyShaderModule(m_dev, m_shader, nullptr);
    }

    shader(const shader&) = delete;
    shader& operator=(const shader&) = delete;

    shader(shader&& other) noexcept
    :m_dev{other.m_dev}
    ,m_shader{std::exchange(other.m_shader, nullptr)}
    {

    }

    shader& operator=(shader&& other) noexcept
    {
        m_dev = other.m_dev;
        m_shader = std::exchange(other.m_shader, nullptr);

        return *this;
    }

    operator VkShaderModule() const noexcept
    {
        return m_shader;
    }

private:
    VkDevice m_dev{};
    VkShaderModule m_shader{};
};

class render_pass
{
public:
    render_pass() = default;

    render_pass(device& dev, swap_chain& sc)
    :m_dev{dev}
    {
        VkAttachmentDescription attachment{};
        attachment.format = sc.format();
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference attachment_ref{};
        attachment_ref.attachment = 0;
        attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &attachment_ref;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        create_info.attachmentCount = 1;
        create_info.pAttachments = &attachment;
        create_info.subpassCount = 1;
        create_info.pSubpasses = &subpass;
        create_info.dependencyCount = 1;
        create_info.pDependencies = &dependency;

        if(vkCreateRenderPass(dev, &create_info, nullptr, &m_render_pass) != VK_SUCCESS)
            throw std::runtime_error("failed to create render pass!");

        m_framebuffers.resize(sc.image_count());
        for(std::size_t i{}; i < sc.image_count(); ++i)
        {
            std::array attachments{sc.image_views()[i]};

            VkFramebufferCreateInfo framebuffer_info{};
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.renderPass = m_render_pass;
            framebuffer_info.attachmentCount = std::size(attachments);
            framebuffer_info.pAttachments = std::data(attachments);
            framebuffer_info.width = sc.extent().width;
            framebuffer_info.height = sc.extent().height;
            framebuffer_info.layers = 1;

            if(vkCreateFramebuffer(dev, &framebuffer_info, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
                throw std::runtime_error{"Failed to create framebuffer."};
        }
    }

    ~render_pass()
    {
        if(m_render_pass)
        {
            for(auto framebuffer : m_framebuffers)
                vkDestroyFramebuffer(m_dev, framebuffer, nullptr);

            vkDestroyRenderPass(m_dev, m_render_pass, nullptr);
        }
    }

    render_pass(const render_pass&) = delete;
    render_pass& operator=(const render_pass&) = delete;

    render_pass(render_pass&& other) noexcept
    :m_dev{other.m_dev}
    ,m_render_pass{std::exchange(other.m_render_pass, nullptr)}
    ,m_framebuffers{std::move(other.m_framebuffers)}
    {

    }

    render_pass& operator=(render_pass&& other) noexcept
    {
        m_dev = other.m_dev;
        m_render_pass = std::exchange(other.m_render_pass, nullptr);
        m_framebuffers = std::move(other.m_framebuffers);

        return *this;
    }

    operator VkRenderPass() const noexcept
    {
        return m_render_pass;
    }

    std::vector<VkFramebuffer> framebuffers() const noexcept
    {
        return m_framebuffers;
    }

private:
    VkDevice m_dev{};
    VkRenderPass m_render_pass{};
    std::vector<VkFramebuffer> m_framebuffers{};
};

class command_pool
{
public:
    command_pool(device& dev, std::uint32_t family)
    :m_dev{dev}
    {
        VkCommandPoolCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        create_info.queueFamilyIndex = family;

        if(vkCreateCommandPool(dev, &create_info, nullptr, &m_command_pool) != VK_SUCCESS)
            throw std::runtime_error{"Failed to create command pool."};
    }

    ~command_pool()
    {
        if(m_command_pool)
            vkDestroyCommandPool(m_dev, m_command_pool, nullptr);
    }

    command_pool(const command_pool&) = delete;
    command_pool& operator=(const command_pool&) = delete;

    command_pool(command_pool&& other) noexcept
    :m_dev{other.m_dev}
    ,m_command_pool{std::exchange(other.m_command_pool, nullptr)}
    {

    }

    command_pool& operator=(command_pool&& other) noexcept
    {
        m_dev = other.m_dev;
        m_command_pool = std::exchange(other.m_command_pool, nullptr);

        return *this;
    }

    operator VkCommandPool() const noexcept
    {
        return m_command_pool;
    }

private:
    VkDevice m_dev{};
    VkCommandPool m_command_pool{};
};

class command_buffer
{
public:
    command_buffer(device& dev, command_pool& cp, VkCommandBufferLevel level)
    :m_dev{dev}
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = cp;
        allocInfo.level = level;
        allocInfo.commandBufferCount = 1;

        if(vkAllocateCommandBuffers(dev, &allocInfo, &m_command_buffer) != VK_SUCCESS)
            throw std::runtime_error{"Failed to allocate command buffers."};
    }

    ~command_buffer()
    {

    }

    command_buffer(const command_buffer&) = delete;
    command_buffer& operator=(const command_buffer&) = delete;

    command_buffer(command_buffer&& other) noexcept = default;
    command_buffer& operator=(command_buffer&& other) noexcept = default;

    operator VkCommandBuffer() const noexcept
    {
        return m_command_buffer;
    }

private:
    VkDevice m_dev{};
    VkCommandBuffer m_command_buffer{};
};

class descriptor_set_layout
{
public:
    descriptor_set_layout() = default;

    descriptor_set_layout(device& dev, std::vector<VkDescriptorSetLayoutBinding> bindings)
    :m_dev{dev}
    {
        VkDescriptorSetLayoutCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        create_info.bindingCount = static_cast<std::uint32_t>(std::size(bindings));
        create_info.pBindings = std::data(bindings);

        if(vkCreateDescriptorSetLayout(dev, &create_info, nullptr, &m_layout) != VK_SUCCESS)
            throw std::runtime_error{"Failed to create descriptor set layout."};
    }

    ~descriptor_set_layout()
    {
        if(m_layout)
            vkDestroyDescriptorSetLayout(m_dev, m_layout, nullptr);
    }

    descriptor_set_layout(const descriptor_set_layout&) = delete;
    descriptor_set_layout& operator=(const descriptor_set_layout&) = delete;

    descriptor_set_layout(descriptor_set_layout&& other) noexcept
    :m_dev{other.m_dev}
    ,m_layout{std::exchange(other.m_layout, nullptr)}
    {

    }

    descriptor_set_layout& operator=(descriptor_set_layout&& other) noexcept
    {
        m_dev = other.m_dev;
        m_layout = std::exchange(other.m_layout, nullptr);

        return *this;
    }

    operator VkDescriptorSetLayout() const noexcept
    {
        return m_layout;
    }

private:
    VkDevice m_dev{};
    VkDescriptorSetLayout m_layout{};
};

class pipeline
{
public:
    pipeline() = default;

    pipeline(device& dev, swap_chain& sc, render_pass& rp, descriptor_set_layout& descriptor)
    :m_dev{dev}
    {
        std::array<VkPipelineShaderStageCreateInfo, 2> shader_info{};

        shader vertex{dev, read_file("shaders/vertex.vert.spv")};
        shader_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shader_info[0].module = vertex;
        shader_info[0].pName = "main";

        shader fragment{dev, read_file("shaders/fragment.frag.spv")};
        shader_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shader_info[1].module = fragment;
        shader_info[1].pName = "main";

        auto binding_description = vertex::binding_description();
        auto attribute_descriptions = vertex::attribute_description();

        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = 1;
        vertex_input_info.pVertexBindingDescriptions = &binding_description;
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(std::size(attribute_descriptions));
        vertex_input_info.pVertexAttributeDescriptions = std::data(attribute_descriptions);

        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = sc.extent().width;
        viewport.height = sc.extent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = VkOffset2D{0, 0};
        scissor.extent = sc.extent();

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.pViewports = &viewport;
        viewport_state.scissorCount = 1;
        viewport_state.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState color_blend_attachment{};
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_TRUE;
        color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo color_blending{};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &color_blend_attachment;

        VkDescriptorSetLayout descriptor_set_layout{descriptor};
        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 1;
        pipeline_layout_info.pSetLayouts = &descriptor_set_layout;
        pipeline_layout_info.pushConstantRangeCount = 0; // Optional
        pipeline_layout_info.pPushConstantRanges = nullptr; // Optional

        if(vkCreatePipelineLayout(dev, &pipeline_layout_info, nullptr, &m_pipeline_layout) != VK_SUCCESS)
            throw std::runtime_error{"failed to create pipeline layout."};

        VkGraphicsPipelineCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        create_info.stageCount = 2;
        create_info.pStages = std::data(shader_info);
        create_info.pVertexInputState = &vertex_input_info;
        create_info.pInputAssemblyState = &input_assembly;
        create_info.pViewportState = &viewport_state;
        create_info.pRasterizationState = &rasterizer;
        create_info.pMultisampleState = &multisampling;
        create_info.pDepthStencilState = nullptr;
        create_info.pColorBlendState = &color_blending;
        create_info.pDynamicState = nullptr;
        create_info.layout = m_pipeline_layout;
        create_info.renderPass = rp;
        create_info.subpass = 0;
        create_info.basePipelineHandle = VK_NULL_HANDLE;
        create_info.basePipelineIndex = -1;

        if(vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &create_info, nullptr, &m_pipeline) != VK_SUCCESS)
            throw std::runtime_error{"failed to create graphics pipeline."};
    }

    ~pipeline()
    {
        if(m_pipeline)
        {
            vkDestroyPipelineLayout(m_dev, m_pipeline_layout, nullptr);
            vkDestroyPipeline(m_dev, m_pipeline, nullptr);
        }
    }

    pipeline(const pipeline&) = delete;
    pipeline& operator=(const pipeline&) = delete;

    pipeline(pipeline&& other) noexcept
    :m_dev{other.m_dev}
    ,m_pipeline_layout{other.m_pipeline_layout}
    ,m_pipeline{std::exchange(other.m_pipeline, nullptr)}
    {

    }

    pipeline& operator=(pipeline&& other) noexcept
    {
        m_dev = other.m_dev;
        m_pipeline_layout = other.m_pipeline_layout;
        m_pipeline = std::exchange(other.m_pipeline, nullptr);

        return *this;
    }

    operator VkPipeline() const noexcept
    {
        return m_pipeline;
    }

    VkPipelineLayout layout() const noexcept
    {
        return m_pipeline_layout;
    }

private:
    VkDevice m_dev{};
    VkPipelineLayout m_pipeline_layout{};
    VkPipeline m_pipeline{};
};

class semaphore
{
public:
    semaphore(device& dev)
    :m_dev{dev}
    {
        VkSemaphoreCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if(vkCreateSemaphore(dev, &create_info, nullptr, &m_semaphore) != VK_SUCCESS)
            throw std::runtime_error{"Failed to create semaphore."};
    }

    ~semaphore()
    {
        if(m_semaphore)
            vkDestroySemaphore(m_dev, m_semaphore, nullptr);
    }

    semaphore(const semaphore&) = delete;
    semaphore& operator=(const semaphore&) = delete;

    semaphore(semaphore&& other) noexcept
    :m_dev{other.m_dev}
    ,m_semaphore{std::exchange(other.m_semaphore, nullptr)}
    {

    }

    semaphore& operator=(semaphore&& other) noexcept
    {
        m_dev = other.m_dev;
        m_semaphore = std::exchange(other.m_semaphore, nullptr);

        return *this;
    }

    operator VkSemaphore() const noexcept
    {
        return m_semaphore;
    }

private:
    VkDevice m_dev{};
    VkSemaphore m_semaphore{};
};

class fence
{
public:
    fence(device& dev, bool signaled = true)
    :m_dev{dev}
    {
        VkFenceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        create_info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

        if(vkCreateFence(dev, &create_info, nullptr, &m_fence) != VK_SUCCESS)
            throw std::runtime_error{"Failed to create semaphore."};
    }

    ~fence()
    {
        if(m_fence)
            vkDestroyFence(m_dev, m_fence, nullptr);
    }

    fence(const fence&) = delete;
    fence& operator=(const fence&) = delete;

    fence(fence&& other) noexcept
    :m_dev{other.m_dev}
    ,m_fence{std::exchange(other.m_fence, nullptr)}
    {

    }

    fence& operator=(fence&& other) noexcept
    {
        m_dev = other.m_dev;
        m_fence = std::exchange(other.m_fence, nullptr);

        return *this;
    }

    operator VkFence() const noexcept
    {
        return m_fence;
    }

private:
    VkDevice m_dev{};
    VkFence m_fence{};
};

class buffer
{
public:
    buffer(device& dev, std::size_t size, VkBufferUsageFlags usage, VkSharingMode mode)
    :m_dev{dev}
    {
        VkBufferCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        create_info.size = size;
        create_info.usage = usage;
        create_info.sharingMode = mode;

        if(vkCreateBuffer(dev, &create_info, nullptr, &m_buffer) != VK_SUCCESS)
            throw std::runtime_error{"Failed to create buffer."};
    }

    ~buffer()
    {
        if(m_buffer)
            vkDestroyBuffer(m_dev, m_buffer, nullptr);
    }

    buffer(const buffer&) = delete;
    buffer& operator=(const buffer&) = delete;

    buffer(buffer&& other) noexcept
    :m_dev{other.m_dev}
    ,m_buffer{std::exchange(other.m_buffer, nullptr)}
    {

    }

    buffer& operator=(buffer&& other) noexcept
    {
        m_dev = other.m_dev;
        m_buffer = std::exchange(other.m_buffer, nullptr);

        return *this;
    }

    operator VkBuffer() const noexcept
    {
        return m_buffer;
    }

private:
    VkDevice m_dev{};
    VkBuffer m_buffer{};
};

class image
{
public:
    image() = default;

    image(device& dev, std::size_t width, std::size_t height, VkImageUsageFlags usage, VkFormat format)
    :m_dev{dev}
    {
        VkImageCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        create_info.imageType = VK_IMAGE_TYPE_2D;
        create_info.format = format;
        create_info.extent.width = width;
        create_info.extent.height = height;
        create_info.extent.depth = 1;
        create_info.mipLevels = 1;
        create_info.arrayLayers = 1;
        create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        create_info.usage = usage;
        create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
        create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if(vkCreateImage(dev, &create_info, nullptr, &m_image) != VK_SUCCESS)
            throw std::runtime_error{"Failed to create image."};
    }

    ~image()
    {
        if(m_image)
            vkDestroyImage(m_dev, m_image, nullptr);
    }

    image(const image&) = delete;
    image& operator=(const image&) = delete;

    image(image&& other) noexcept
    :m_dev{other.m_dev}
    ,m_image{std::exchange(other.m_image, nullptr)}
    {

    }

    image& operator=(image&& other) noexcept
    {
        m_dev = other.m_dev;
        m_image = std::exchange(other.m_image, nullptr);

        return *this;
    }

    operator VkImage() const noexcept
    {
        return m_image;
    }

private:
    VkDevice m_dev{};
    VkImage m_image{};
};

class image_view
{
public:
    image_view() = default;

    image_view(device& dev, image& img, VkFormat format)
    :m_dev{dev}
    {
        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = img;
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        if(vkCreateImageView(dev, &create_info, nullptr, &m_view) != VK_SUCCESS)
            throw std::runtime_error{"Failed to create image view."};
    }

    ~image_view()
    {
        if(m_view)
            vkDestroyImageView(m_dev, m_view, nullptr);
    }

    image_view(const image_view&) = delete;
    image_view& operator=(const image_view&) = delete;

    image_view(image_view&& other) noexcept
    :m_dev{other.m_dev}
    ,m_view{std::exchange(other.m_view, nullptr)}
    {

    }

    image_view& operator=(image_view&& other) noexcept
    {
        m_dev = other.m_dev;
        m_view = std::exchange(other.m_view, nullptr);

        return *this;
    }

    operator VkImageView() const noexcept
    {
        return m_view;
    }

private:
    VkDevice m_dev{};
    VkImageView m_view{};
};

class memory
{
public:
    memory() = default;

    memory(device& dev, std::size_t size, std::uint32_t memory_type)
    :m_dev{dev}
    {
        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = size;
        alloc_info.memoryTypeIndex = memory_type;

        if(vkAllocateMemory(dev, &alloc_info, nullptr, &m_memory) != VK_SUCCESS)
            throw std::runtime_error{"Failed to create memory."};
    }

    ~memory()
    {
        if(m_memory)
            vkFreeMemory(m_dev, m_memory, nullptr);
    }

    memory(const memory&) = delete;
    memory& operator=(const memory&) = delete;

    memory(memory&& other) noexcept
    :m_dev{other.m_dev}
    ,m_memory{std::exchange(other.m_memory, nullptr)}
    {

    }

    memory& operator=(memory&& other) noexcept
    {
        m_dev = other.m_dev;
        m_memory = std::exchange(other.m_memory, nullptr);

        return *this;
    }

    operator VkDeviceMemory() const noexcept
    {
        return m_memory;
    }

private:
    VkDevice m_dev{};
    VkDeviceMemory m_memory{};
};

class sampler
{
public:
    sampler() = default;

    sampler(device& dev, VkFilter filter, VkSamplerAddressMode address_mode, std::uint32_t anisotropy_level)
    :m_dev{dev}
    {
        VkSamplerCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        create_info.magFilter = filter;
        create_info.minFilter = filter;
        create_info.addressModeU = address_mode;
        create_info.addressModeV = address_mode;
        create_info.addressModeW = address_mode;
        create_info.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
        create_info.unnormalizedCoordinates = VK_FALSE;
        create_info.compareEnable = VK_FALSE;
        create_info.compareOp = VK_COMPARE_OP_ALWAYS;
        create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        create_info.mipLodBias = 0.0f;
        create_info.minLod = 0.0f;
        create_info.maxLod = 0.0f;

        if(anisotropy_level > 1)
        {
            create_info.anisotropyEnable = VK_TRUE;
            create_info.maxAnisotropy = 1;
        }

        if(vkCreateSampler(dev, &create_info, nullptr, &m_sampler) != VK_SUCCESS)
            throw std::runtime_error{"Failed to create sampler."};
    }


    ~sampler()
    {
        if(m_sampler)
            vkDestroySampler(m_dev, m_sampler, nullptr);
    }

    sampler(const sampler&) = delete;
    sampler& operator=(const sampler&) = delete;

    sampler(sampler&& other) noexcept
    :m_dev{other.m_dev}
    ,m_sampler{std::exchange(other.m_sampler, nullptr)}
    {

    }

    sampler& operator=(sampler&& other) noexcept
    {
        m_dev = other.m_dev;
        m_sampler = std::exchange(other.m_sampler, nullptr);

        return *this;
    }

    operator VkSampler() const noexcept
    {
        return m_sampler;
    }

private:
    VkDevice m_dev{};
    VkSampler m_sampler{};
};

class texture
{
public:
    texture(device& dev, const std::string& str)
    :m_dev{dev}
    {
        int width{};
        int height{};
        int channels{};
        std::uint8_t* pixels = stbi_load(std::data(str), &width, &height, &channels, STBI_rgb_alpha);
        if(!pixels)
            throw std::runtime_error{"Can not load file \"" + str + "\"."};

        const VkDeviceSize byte_size = width * height * 4;

        buffer staging_buffer{dev, byte_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE};
        memory staging_memory{dev, required_memory_size(dev, staging_buffer), find_memory_type(dev.physical_device(), required_memory_type(dev, staging_buffer), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)};
        if(vkBindBufferMemory(dev, staging_buffer, staging_memory, 0) != VK_SUCCESS)
            throw std::runtime_error{"Can not bind memory of staging buffer for file \"" + str + "\"."};

        void* data{};
        vkMapMemory(dev, staging_memory, 0, byte_size, 0, &data);
        memcpy(data, pixels, static_cast<std::size_t>(byte_size));
        vkUnmapMemory(dev, staging_memory);

        stbi_image_free(pixels);

        m_image = image{dev, static_cast<std::size_t>(width), static_cast<std::size_t>(height), VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R8G8B8A8_UNORM};
        m_memory = memory{dev, required_memory_size(dev, m_image), find_memory_type(dev.physical_device(), required_memory_type(dev, m_image), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};
        if(vkBindImageMemory(dev, m_image, m_memory, 0) != VK_SUCCESS)
            throw std::runtime_error{"Can not bind memory for file \"" + str + "\"."};

        command_pool pool{dev, dev.transfer_family()};
        command_buffer command_buf{dev, pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY};

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if(vkBeginCommandBuffer(command_buf, &begin_info) != VK_SUCCESS)
            throw std::runtime_error{"Failed to begin recording command buffer."};

        VkImageMemoryBarrier first_barrier{};
        first_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        first_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        first_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        first_barrier.srcAccessMask = 0;
        first_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        first_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        first_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        first_barrier.image = m_image;
        first_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        first_barrier.subresourceRange.baseMipLevel = 0;
        first_barrier.subresourceRange.levelCount = 1;
        first_barrier.subresourceRange.baseArrayLayer = 0;
        first_barrier.subresourceRange.layerCount = 1;

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset.x = 0;
        region.imageOffset.y = 0;
        region.imageOffset.z = 0;
        region.imageExtent.width = width;
        region.imageExtent.height = height;
        region.imageExtent.depth = 1;

        VkImageMemoryBarrier second_barrier{};
        second_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        second_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        second_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        second_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        second_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        second_barrier.srcQueueFamilyIndex = dev.transfer_family();
        second_barrier.dstQueueFamilyIndex = dev.graphics_family();
        second_barrier.image = m_image;
        second_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        second_barrier.subresourceRange.baseMipLevel = 0;
        second_barrier.subresourceRange.levelCount = 1;
        second_barrier.subresourceRange.baseArrayLayer = 0;
        second_barrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(command_buf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &first_barrier);
        vkCmdCopyBufferToImage(command_buf, staging_buffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        vkCmdPipelineBarrier(command_buf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &second_barrier);

        vkEndCommandBuffer(command_buf);

        fence submit_fence{dev, false};

        VkCommandBuffer native_command_buf{command_buf};
        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &native_command_buf;

        vkQueueSubmit(dev.transfer_queue(), 1, &submit, submit_fence);

        VkFence native_fence{submit_fence};
        vkWaitForFences(dev, 1, &native_fence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());

        m_view = image_view{dev, m_image, VK_FORMAT_R8G8B8A8_UNORM};
        m_sampler = sampler{dev, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, 1};
    }

    ~texture() = default;
    texture(const texture&) = delete;
    texture& operator=(const texture&) = delete;
    texture(texture&& other) noexcept = default;
    texture& operator=(texture&& other) noexcept = default;

    VkImage get_image() const noexcept
    {
        return m_image;
    }

    VkImageView get_image_view() const noexcept
    {
        return m_view;
    }

    VkSampler get_sampler() const noexcept
    {
        return m_sampler;
    }

private:
    VkDevice m_dev{};
    memory m_memory{};
    image m_image{};
    image_view m_view{};
    sampler m_sampler{};
};

class descriptor_pool
{
public:
    descriptor_pool() = default;

    descriptor_pool(device& dev, const std::vector<VkDescriptorPoolSize>& sizes, std::uint32_t sets)
    :m_dev{dev}
    {
        VkDescriptorPoolCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        create_info.poolSizeCount = static_cast<std::uint32_t>(std::size(sizes));
        create_info.pPoolSizes = std::data(sizes);
        create_info.maxSets = sets;

        if(vkCreateDescriptorPool(dev, &create_info, nullptr, &m_descriptor_pool) != VK_SUCCESS)
            throw std::runtime_error{"Failed to create descriptor pool."};
    }


    ~descriptor_pool()
    {
        if(m_descriptor_pool)
            vkDestroyDescriptorPool(m_dev, m_descriptor_pool, nullptr);
    }

    descriptor_pool(const descriptor_pool&) = delete;
    descriptor_pool& operator=(const descriptor_pool&) = delete;

    descriptor_pool(descriptor_pool&& other) noexcept
    :m_dev{other.m_dev}
    ,m_descriptor_pool{std::exchange(other.m_descriptor_pool, nullptr)}
    {

    }

    descriptor_pool& operator=(descriptor_pool&& other) noexcept
    {
        m_dev = other.m_dev;
        m_descriptor_pool = std::exchange(other.m_descriptor_pool, nullptr);

        return *this;
    }

    operator VkDescriptorPool() const noexcept
    {
        return m_descriptor_pool;
    }

private:
    VkDevice m_dev{};
    VkDescriptorPool m_descriptor_pool{};
};

class descriptor_set
{
public:
    descriptor_set() = default;

    descriptor_set(device& dev, descriptor_pool& descriptor_pool, VkDescriptorSetLayout layout)
    :m_dev{dev}
    {
        VkDescriptorSetAllocateInfo allocation_info{};
        allocation_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocation_info.descriptorPool = descriptor_pool;
        allocation_info.descriptorSetCount = 1;
        allocation_info.pSetLayouts = &layout;

        if(vkAllocateDescriptorSets(dev, &allocation_info, &m_descriptor_set) != VK_SUCCESS)
            throw std::runtime_error{"Failed to allocate descriptor set."};
    }

    ~descriptor_set() = default;

    descriptor_set(const descriptor_set&) = delete;
    descriptor_set& operator=(const descriptor_set&) = delete;

    descriptor_set(descriptor_set&& other) noexcept
    :m_dev{other.m_dev}
    ,m_descriptor_set{std::exchange(other.m_descriptor_set, nullptr)}
    {

    }

    descriptor_set& operator=(descriptor_set&& other) noexcept
    {
        m_dev = other.m_dev;
        m_descriptor_set = std::exchange(other.m_descriptor_set, nullptr);

        return *this;
    }

    operator VkDescriptorSet() const noexcept
    {
        return m_descriptor_set;
    }

private:
    VkDevice m_dev{};
    VkDescriptorSet m_descriptor_set{};
};

static std::array vertices
{
    vertex{{-0.5f, -0.5f}, {0.8f, 0.5f, 0.35f}, {1.0f, 0.0f}},
    vertex{{0.5f, -0.5f}, {0.92f, 0.15f, 0.25f}, {0.0f, 0.0f}},
    vertex{{0.5f, 0.5f}, {0.3f, 1.0f, 0.7f}, {0.0f, 1.0f}},
    vertex{{-0.5f, 0.5f}, {0.6f, 0.2f, 0.05f}, {1.0f, 1.0f}},
    vertex{{-1.0f, -1.0f}, {0.8f, 0.5f, 0.35f}, {1.0f, 0.0f}},
    vertex{{0.0f, -1.0f}, {0.92f, 0.15f, 0.25f}, {0.0f, 0.0f}},
    vertex{{0.0f, 0.0f}, {0.3f, 1.0f, 0.7f}, {0.0f, 1.0f}},
    vertex{{-1.0f, 0.0f}, {0.6f, 0.2f, 0.05f}, {1.0f, 1.0f}}
};

static constexpr std::array indexes
{
    0u, 1u, 2u, 2u, 3u, 0u
};

static uniform_buffer_object ubo{};

class application
{
public:
    application()
    {
        vkBindBufferMemory(m_dev, m_vertex_buffer, m_vertex_buffer_memory, 0);
        vkBindBufferMemory(m_dev, m_index_buffer, m_index_buffer_memory, 0);
        vkBindBufferMemory(m_dev, m_ubo_buffer, m_ubo_memory, 0);

        void* data;
        vkMapMemory(m_dev, m_index_buffer_memory, 0, std::size(indexes) * sizeof(std::uint32_t), 0, &data);
        memcpy(data, std::data(indexes), std::size(indexes) * sizeof(std::uint32_t));
        vkUnmapMemory(m_dev, m_index_buffer_memory);

        void* vertex_data;
        vkMapMemory(m_dev, m_vertex_buffer_memory, 0, std::size(vertices) * sizeof(vertex), 0, &vertex_data);
        memcpy(vertex_data, std::data(vertices), std::size(vertices) * sizeof(vertex));
        vkUnmapMemory(m_dev, m_vertex_buffer_memory);

        for(std::size_t i{}; i < m_swap_chain.image_count(); ++i)
        {
            m_image_available.emplace_back(m_dev);
            m_render_finished.emplace_back(m_dev);

            m_fences.emplace_back(m_dev);

            m_descriptor_sets.emplace_back(m_dev, m_descriptor_pool, m_descriptor_set_layout);
            write_descriptor_set(i);

            m_command_buffers.emplace_back(m_dev, m_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
            draw_vertices(i);
        }
    }

    ~application()
    {
        vkDeviceWaitIdle(m_dev);
    }

    application(const application&) = delete;
    application& operator=(const application&) = delete;
    application(application&&) = delete;
    application& operator=(application&&) = delete;

    void run()
    {
        bool running{true};
        while(running)
        {
            SDL_Event event{};
            while(SDL_PollEvent(&event))
            {
                if(event.type == SDL_QUIT)
                    running = false;

                if(event.type == SDL_WINDOWEVENT)
                {
                    if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                        running = false;
                }

                if(event.type == SDL_KEYDOWN)
                {
                    if(event.key.keysym.sym == SDLK_LEFT)  m_pushed_keys[0] = true;
                    if(event.key.keysym.sym == SDLK_UP)    m_pushed_keys[1] = true;
                    if(event.key.keysym.sym == SDLK_RIGHT) m_pushed_keys[2] = true;
                    if(event.key.keysym.sym == SDLK_DOWN)  m_pushed_keys[3] = true;
                }

                if(event.type == SDL_KEYUP)
                {
                    if(event.key.keysym.sym == SDLK_LEFT)  m_pushed_keys[0] = false;
                    if(event.key.keysym.sym == SDLK_UP)    m_pushed_keys[1] = false;
                    if(event.key.keysym.sym == SDLK_RIGHT) m_pushed_keys[2] = false;
                    if(event.key.keysym.sym == SDLK_DOWN)  m_pushed_keys[3] = false;
                }
            }

            if(m_pushed_keys[0]) m_pos += glm::vec3{-0.01f, 0.0f, 0.0f};
            if(m_pushed_keys[1]) m_pos += glm::vec3{0.0f, -0.01f, 0.0f};
            if(m_pushed_keys[2]) m_pos += glm::vec3{0.01f, 0.0f, 0.0f};
            if(m_pushed_keys[3]) m_pos += glm::vec3{0.0f, 0.01f, 0.0f};

            update();
            draw();
        }
    }

private:
    void update()
    {
        //const float elapsed_time = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - last_update).count();
        //last_update = std::chrono::high_resolution_clock::now();

        ubo.model = glm::rotate(glm::mat4{1.0f}, glm::radians(180.0f), glm::vec3{0.0f, 0.0f, 1.0f}); //glm::rotate(glm::mat4{1.0f}, elapsed_time * glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f});
        ubo.view = glm::lookAt(m_pos, m_pos + glm::vec3{0.0f, 0.0f, -1.0f}, glm::vec3{0.0f, -1.0f, 0.0f});
        ubo.proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);

        void* ubo_data;
        vkMapMemory(m_dev, m_ubo_memory, 0, sizeof(uniform_buffer_object), 0, &ubo_data);
        memcpy(ubo_data, &ubo, sizeof(uniform_buffer_object));
        vkUnmapMemory(m_dev, m_ubo_memory);
    }

    void draw()
    {
        VkFence fence{m_fences[m_current_frame]};
        vkWaitForFences(m_dev, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

        std::uint32_t image_index{};
        vkAcquireNextImageKHR(m_dev, m_swap_chain, std::numeric_limits<uint64_t>::max(), m_image_available[m_current_frame], VK_NULL_HANDLE, &image_index);

        VkCommandBuffer command_buffer{m_command_buffers[image_index]};
        VkSemaphore wait_semaphores{m_image_available[m_current_frame]};
        VkPipelineStageFlags wait_stages{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signal_semaphores{m_render_finished[m_current_frame]};

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &wait_semaphores;
        submit_info.pWaitDstStageMask = &wait_stages;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &signal_semaphores;

        vkResetFences(m_dev, 1, &fence);
        if(vkQueueSubmit(m_dev.graphics_queue(), 1, &submit_info, m_fences[m_current_frame]) != VK_SUCCESS)
            throw std::runtime_error{"Failed to submit draw command buffer."};

        VkSwapchainKHR swap_chain{m_swap_chain};

        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &signal_semaphores;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swap_chain;
        present_info.pImageIndices = &image_index;

        vkQueuePresentKHR(m_dev.present_queue(), &present_info);

        m_current_frame = (m_current_frame + 1) % m_swap_chain.image_count();
    }

    void write_descriptor_set(std::size_t index)
    {
        std::array<VkWriteDescriptorSet, 2> writes{};

        VkDescriptorImageInfo descriptor_image{};
        descriptor_image.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptor_image.imageView = m_texture.get_image_view();
        descriptor_image.sampler = m_texture.get_sampler();

        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[0].dstSet = m_descriptor_sets[index];
        writes[0].dstBinding = 0;
        writes[0].dstArrayElement = 0;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[0].descriptorCount = 1;
        writes[0].pImageInfo = &descriptor_image;

        VkDescriptorBufferInfo descriptor_ubo{};
        descriptor_ubo.buffer = m_ubo_buffer;
        descriptor_ubo.offset = 0;
        descriptor_ubo.range = sizeof(uniform_buffer_object);

        writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[1].dstSet = m_descriptor_sets[index];
        writes[1].dstBinding = 1;
        writes[1].dstArrayElement = 0;
        writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[1].descriptorCount = 1;
        writes[1].pBufferInfo = &descriptor_ubo;

        vkUpdateDescriptorSets(m_dev, std::size(writes), std::data(writes), 0, nullptr);
    }

    void draw_vertices(std::uint64_t image_index)
    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if(vkBeginCommandBuffer(m_command_buffers[image_index], &begin_info) != VK_SUCCESS)
            throw std::runtime_error{"Failed to begin recording command buffer."};

        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = m_render_pass;
        render_pass_info.framebuffer = m_render_pass.framebuffers()[image_index];
        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = m_swap_chain.extent();

        VkClearValue clear_color{1.0f, 1.0f, 1.0f, 1.0f};
        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues = &clear_color;

        vkCmdBeginRenderPass(m_command_buffers[image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(m_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

        VkBuffer vertex_buffer{m_vertex_buffer};
        VkDeviceSize offset{};
        vkCmdBindVertexBuffers(m_command_buffers[image_index], 0, 1, &vertex_buffer, &offset);

        VkBuffer index_buffer{m_index_buffer};
        vkCmdBindIndexBuffer(m_command_buffers[image_index], index_buffer, 0, VK_INDEX_TYPE_UINT32);

        VkDescriptorSet descriptor_set{m_descriptor_sets[image_index]};
        vkCmdBindDescriptorSets(m_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.layout(), 0, 1, &descriptor_set, 0, nullptr);
        vkCmdDrawIndexed(m_command_buffers[image_index], std::size(indexes), 1, 0, 0, 0);
        vkCmdDrawIndexed(m_command_buffers[image_index], std::size(indexes), 1, 0, 4, 0);

        vkCmdEndRenderPass(m_command_buffers[image_index]);
        if(vkEndCommandBuffer(m_command_buffers[image_index]) != VK_SUCCESS)
            throw std::runtime_error{"Failed to record command buffer."};
    }

    window m_window{};
    instance m_instance{m_window};
    surface m_surf{m_instance, m_window};
    debug_callback m_db{m_instance};
    physical_device m_phydev{m_instance};
    device m_dev{m_phydev, m_surf};

    //swap chain:
    swap_chain m_swap_chain{m_dev, m_phydev, m_window, m_surf, 2u};
    render_pass m_render_pass{m_dev, m_swap_chain};
    descriptor_set_layout m_descriptor_set_layout{m_dev, make_descriptor_set_layout_bindings()};
    pipeline m_pipeline{m_dev, m_swap_chain, m_render_pass, m_descriptor_set_layout};
    command_pool m_command_pool{m_dev, m_dev.graphics_family()};
    std::vector<command_buffer> m_command_buffers{};

    descriptor_pool m_descriptor_pool{m_dev,
    {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_swap_chain.image_count()},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_swap_chain.image_count()}
    }, m_swap_chain.image_count()};
    std::vector<descriptor_set> m_descriptor_sets{};

    //end of swap chain
    std::vector<semaphore> m_image_available{};
    std::vector<semaphore> m_render_finished{};
    std::vector<fence> m_fences{};
    std::size_t m_current_frame{};

    //End of static part
    buffer m_vertex_buffer
    {
        m_dev,
        std::size(vertices) * sizeof(vertex),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE
    };

    memory m_vertex_buffer_memory
    {
        m_dev,
        required_memory_size(m_dev, m_vertex_buffer),
        find_memory_type(m_phydev, required_memory_type(m_dev, m_vertex_buffer), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };

    buffer m_index_buffer
    {
        m_dev,
        std::size(indexes) * sizeof(std::uint32_t),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE
    };

    memory m_index_buffer_memory
    {
        m_dev,
        required_memory_size(m_dev, m_index_buffer),
        find_memory_type(m_phydev, required_memory_type(m_dev, m_index_buffer), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };

    buffer m_ubo_buffer
    {
        m_dev,
        sizeof(uniform_buffer_object),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE
    };

    memory m_ubo_memory
    {
        m_dev,
        required_memory_size(m_dev, m_ubo_buffer),
        find_memory_type(m_phydev, required_memory_type(m_dev, m_ubo_buffer), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };

    texture m_texture{m_dev, "slight_heart.png"};

    //Logical part
    std::chrono::high_resolution_clock::time_point last_update{std::chrono::high_resolution_clock::now()};
    glm::vec3 m_pos{};
    std::array<bool, 4> m_pushed_keys;
};


int main()
{
    SDL_SetMainReady();

    try
    {
        application{}.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "An error has occured: " << e.what() << std::endl;
    }
}
