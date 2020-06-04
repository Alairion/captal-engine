#include "vulkan.hpp"

#include <algorithm>
#include <stdexcept>
#include <fstream>

#include "vulkan_functions.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

namespace vulkan
{

const char* error::error_name() const noexcept
{
    switch(m_result)
    {
        case VK_NOT_READY: return "VK_NOT_READY";
        case VK_TIMEOUT: return "VK_TIMEOUT";
        case VK_EVENT_SET: return "VK_EVENT_SET";
        case VK_EVENT_RESET: return "VK_EVENT_RESET";
        case VK_INCOMPLETE: return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
        case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
        default: if(m_result < 0) return "Unknown error";
    }

    return "VK_SUCCESS";
}

const char* error::error_description() const noexcept
{
    switch(m_result)
    {
        case VK_NOT_READY: return "A fence or query has not yet completed";
        case VK_TIMEOUT: return "A wait operation has not completed in the specified time";
        case VK_EVENT_SET: return "An event is signaled";
        case VK_EVENT_RESET: return "An event is unsignaled";
        case VK_INCOMPLETE: return "A return array was too small for the result";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "A host memory allocation has failed";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "A device memory allocation has failed";
        case VK_ERROR_INITIALIZATION_FAILED: return "Initialization of an object could not be completed for implementation-specific reasons";
        case VK_ERROR_DEVICE_LOST: return "The logical or physical device has been lost";
        case VK_ERROR_MEMORY_MAP_FAILED: return "Mapping of a memory object has failed";
        case VK_ERROR_LAYER_NOT_PRESENT: return "A requested layer is not present or could not be loaded";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "A requested extension is not supported";
        case VK_ERROR_FEATURE_NOT_PRESENT: return "A requested feature is not supported";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons";
        case VK_ERROR_TOO_MANY_OBJECTS: return "Too many objects of the type have already been created";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return "A requested format is not supported on this device";
        case VK_ERROR_FRAGMENTED_POOL: return "A pool allocation has failed due to fragmentation of the pool’s memory";
        case VK_ERROR_OUT_OF_POOL_MEMORY: return "A pool memory allocation has failed";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "An external handle is not a valid handle of the specified type";
        case VK_ERROR_SURFACE_LOST_KHR: return "A surface is no longer available";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "The requested window is already in use by Vulkan or another API in a manner which prevents it from being used again";
        case VK_SUBOPTIMAL_KHR: return "A swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully";
        case VK_ERROR_OUT_OF_DATE_KHR: return "A surface has changed in such a way that it is no longer compatible with the swapchain, and further presentation requests using the swapchain will fail";
        default: if(m_result < 0) return "Unknown error";
    }

    return "No error";
}

const char* error::what() const noexcept
{
    switch(m_result)
    {
        case VK_NOT_READY: return "VK_NOT_READY: A fence or query has not yet completed";
        case VK_TIMEOUT: return "VK_TIMEOUT: A wait operation has not completed in the specified time";
        case VK_EVENT_SET: return "VK_EVENT_SET: An event is signaled";
        case VK_EVENT_RESET: return "VK_EVENT_RESET: An event is unsignaled";
        case VK_INCOMPLETE: return "VK_INCOMPLETE: A return array was too small for the result";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY: A host memory allocation has failed";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY: A device memory allocation has failed";
        case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED: Initialization of an object could not be completed for implementation-specific reasons";
        case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST: The logical or physical device has been lost";
        case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED: Mapping of a memory object has failed";
        case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT: A requested layer is not present or could not be loaded";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT: A requested extension is not supported";
        case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT: A requested feature is not supported";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER: The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons";
        case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS: Too many objects of the type have already been created";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED: A requested format is not supported on this device";
        case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL: A pool allocation has failed due to fragmentation of the pool’s memory";
        case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY: A pool memory allocation has failed";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE: An external handle is not a valid handle of the specified type";
        case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR: A surface is no longer available";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: The requested window is already in use by Vulkan or another API in a manner which prevents it from being used again";
        case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR: A swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully";
        case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR: A surface has changed in such a way that it is no longer compatible with the swapchain, and further presentation requests using the swapchain will fail";
        default: if(m_result < 0) return "???: Unknown error";
    }

    return "VK_SUCCESS: No error";
}

instance::instance(const std::string& application_name, version version, const std::vector<const char*>& extensions, const std::vector<const char*>& layers)
{
    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = std::data(application_name);
    application_info.applicationVersion = VK_MAKE_VERSION(version.major, version.minor, version.patch);
    application_info.pEngineName = u8"Tephra";
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;
    create_info.enabledExtensionCount = static_cast<std::uint32_t>(std::size(extensions));
    create_info.ppEnabledExtensionNames = std::data(extensions);
    create_info.enabledLayerCount = static_cast<std::uint32_t>(std::size(layers));
    create_info.ppEnabledLayerNames = std::data(layers);

    if(auto result{vkCreateInstance(&create_info, nullptr, &m_instance)}; result != VK_SUCCESS)
        throw error{result};
}

instance::~instance()
{
    if(m_instance)
        vkDestroyInstance(m_instance, nullptr);
}

instance::instance(instance&& other) noexcept
:m_instance{std::exchange(other.m_instance, nullptr)}
{

}

instance& instance::operator=(instance&& other) noexcept
{
    m_instance = std::exchange(other.m_instance, m_instance);

    return *this;
}

/////////////////////////////////////////////////////////////////////

device::device(VkPhysicalDevice physical_device, const std::vector<const char*>& extensions, const std::vector<const char*>& layers, const std::vector<VkDeviceQueueCreateInfo>& queues, const VkPhysicalDeviceFeatures& features)
{
    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.enabledExtensionCount = static_cast<std::uint32_t>(std::size(extensions));
    create_info.ppEnabledExtensionNames = std::data(extensions);
    create_info.enabledLayerCount = static_cast<std::uint32_t>(std::size(layers));
    create_info.ppEnabledLayerNames = std::data(layers);
    create_info.queueCreateInfoCount = static_cast<std::uint32_t>(std::size(queues));
    create_info.pQueueCreateInfos = std::data(queues);
    create_info.pEnabledFeatures = &features;

    if(auto result{vkCreateDevice(physical_device, &create_info, nullptr, &m_device)}; result != VK_SUCCESS)
        throw error{result};
}

device::~device()
{
    if(m_device)
        vkDestroyDevice(m_device, nullptr);
}

device::device(device&& other) noexcept
:m_device{std::exchange(other.m_device, nullptr)}
{

}

device& device::operator=(device&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);

    return *this;
}

/////////////////////////////////////////////////////////////////////

device_memory::device_memory(VkDevice device, std::uint32_t memory_type, std::uint64_t size)
:m_device{device}
{
    VkMemoryAllocateInfo allocation_info{};
    allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocation_info.memoryTypeIndex = memory_type;
    allocation_info.allocationSize = size;

    if(auto result{vkAllocateMemory(m_device, &allocation_info, nullptr, &m_device_memory)}; result != VK_SUCCESS)
        throw error{result};
}

device_memory::~device_memory()
{
    if(m_device_memory)
        vkFreeMemory(m_device, m_device_memory, nullptr);
}

device_memory::device_memory(device_memory&& other) noexcept
:m_device{other.m_device}
,m_device_memory{std::exchange(other.m_device_memory, nullptr)}
{

}

device_memory& device_memory::operator=(device_memory&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_device_memory = std::exchange(other.m_device_memory, m_device_memory);

    return *this;
}

/////////////////////////////////////////////////////////////////////

buffer::buffer(VkDevice device, std::uint64_t size, VkBufferUsageFlags usage)
:m_device{device}
{
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(auto result{vkCreateBuffer(m_device, &create_info, nullptr, &m_buffer)}; result != VK_SUCCESS)
        throw error{result};
}

buffer::~buffer()
{
    if(m_buffer)
        vkDestroyBuffer(m_device, m_buffer, nullptr);
}

buffer::buffer(buffer&& other) noexcept
:m_device{other.m_device}
,m_buffer{std::exchange(other.m_buffer, nullptr)}
{

}

buffer& buffer::operator=(buffer&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_buffer = std::exchange(other.m_buffer, m_buffer);

    return *this;
}

/////////////////////////////////////////////////////////////////////

buffer_view::buffer_view(VkDevice device, VkBuffer buffer, VkFormat format, std::uint64_t offset, std::uint64_t size)
:m_device{device}
{
    VkBufferViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    create_info.buffer = buffer;
    create_info.format = format;
    create_info.offset = offset;
    create_info.range = size;

    if(auto result{vkCreateBufferView(m_device, &create_info, nullptr, &m_buffer_view)}; result != VK_SUCCESS)
        throw error{result};
}

buffer_view::~buffer_view()
{
    if(m_buffer_view)
        vkDestroyBufferView(m_device, m_buffer_view, nullptr);
}

buffer_view::buffer_view(buffer_view&& other) noexcept
:m_device{other.m_device}
,m_buffer_view{std::exchange(other.m_buffer_view, nullptr)}
{

}

buffer_view& buffer_view::operator=(buffer_view&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_buffer_view = std::exchange(other.m_buffer_view, m_buffer_view);

    return *this;
}


/////////////////////////////////////////////////////////////////////

image::image(VkDevice device, VkExtent3D size, VkImageType type, VkFormat format, VkImageUsageFlags usage, VkImageTiling tiling, VkSampleCountFlagBits samples)
:m_device{device}
{
    VkImageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.imageType = type;
    create_info.extent = size;
    create_info.format = format;
    create_info.mipLevels = 1;
    create_info.arrayLayers = 1;
    create_info.samples = samples;
    create_info.tiling = tiling;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if(auto result{vkCreateImage(m_device, &create_info, nullptr, &m_image)}; result != VK_SUCCESS)
        throw error{result};
}

image::image(VkDevice device, VkExtent2D size, VkImageType type, VkFormat format, VkImageUsageFlags usage, VkImageTiling tiling, VkSampleCountFlagBits samples)
:m_device{device}
{
    VkImageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.imageType = type;
    create_info.extent.width = size.width;
    create_info.extent.height = size.height;
    create_info.extent.depth = 1;
    create_info.format = format;
    create_info.mipLevels = 1;
    create_info.arrayLayers = 1;
    create_info.samples = samples;
    create_info.tiling = tiling;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if(auto result{vkCreateImage(m_device, &create_info, nullptr, &m_image)}; result != VK_SUCCESS)
        throw error{result};
}

image::~image()
{
    if(m_device && m_image)
        vkDestroyImage(m_device, m_image, nullptr);
}

image::image(image&& other) noexcept
:m_device{other.m_device}
,m_image{std::exchange(other.m_image, nullptr)}
{

}

image& image::operator=(image&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_image = std::exchange(other.m_image, m_image);

    return *this;
}

/////////////////////////////////////////////////////////////////////

image_view::image_view(VkDevice device, VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspect)
:m_device{device}
{
    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = image;
    create_info.viewType = type;
    create_info.format = format;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask = aspect;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    if(auto result{vkCreateImageView(m_device, &create_info, nullptr, &m_image_view)}; result != VK_SUCCESS)
        throw error{result};
}

image_view::~image_view()
{
    if(m_image_view)
        vkDestroyImageView(m_device, m_image_view, nullptr);
}

image_view::image_view(image_view&& other) noexcept
:m_device{other.m_device}
,m_image_view{std::exchange(other.m_image_view, nullptr)}
{

}

image_view& image_view::operator=(image_view&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_image_view = std::exchange(other.m_image_view, m_image_view);

    return *this;
}

/////////////////////////////////////////////////////////////////////

sampler::sampler(VkDevice device, VkFilter mag_filter, VkFilter min_filter, VkSamplerAddressMode address_mode, VkBool32 compared, VkCompareOp compare_op, VkBool32 unnormalized, float anisotropy)
:m_device{device}
{
    VkSamplerCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    create_info.magFilter = mag_filter;
    create_info.minFilter = min_filter;
    create_info.addressModeU = address_mode;
    create_info.addressModeV = address_mode;
    create_info.addressModeW = address_mode;
    create_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    create_info.anisotropyEnable = anisotropy > 1.0f ? VK_TRUE : VK_FALSE;
    create_info.maxAnisotropy = anisotropy;
    create_info.unnormalizedCoordinates = unnormalized;
    create_info.compareEnable = compared;
    create_info.compareOp = compare_op;
    create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    create_info.mipLodBias = 0.0f;
    create_info.minLod = 0.0f;
    create_info.maxLod = 0.0f;

    if(auto result{vkCreateSampler(m_device, &create_info, nullptr, &m_sampler)}; result != VK_SUCCESS)
        throw error{result};
}

sampler::~sampler()
{
    if(m_sampler)
        vkDestroySampler(m_device, m_sampler, nullptr);
}

sampler::sampler(sampler&& other) noexcept
:m_device{other.m_device}
,m_sampler{std::exchange(other.m_sampler, nullptr)}
{

}

sampler& sampler::operator=(sampler&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_sampler = std::exchange(other.m_sampler, m_sampler);

    return *this;
}

/////////////////////////////////////////////////////////////////////

framebuffer::framebuffer(VkDevice device, VkRenderPass render_pass, const std::vector<VkImageView>& attachments, VkExtent2D size, std::uint32_t layers)
:m_device{device}
{
    VkFramebufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.renderPass = render_pass;
    create_info.attachmentCount = static_cast<std::uint32_t>(std::size(attachments));
    create_info.pAttachments = std::data(attachments);
    create_info.width = size.width;
    create_info.height = size.height;
    create_info.layers = layers;

    if(auto result{vkCreateFramebuffer(m_device, &create_info, nullptr, &m_framebuffer)}; result != VK_SUCCESS)
        throw error{result};
}

framebuffer::~framebuffer()
{
    if(m_framebuffer)
        vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
}

framebuffer::framebuffer(framebuffer&& other) noexcept
:m_device{other.m_device}
,m_framebuffer{std::exchange(other.m_framebuffer, nullptr)}
{

}

framebuffer& framebuffer::operator=(framebuffer&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_framebuffer = std::exchange(other.m_framebuffer, m_framebuffer);

    return *this;
}

/////////////////////////////////////////////////////////////////////

shader::shader(VkDevice device, std::size_t size, const std::uint32_t* code)
:m_device{device}
{
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = static_cast<std::uint32_t>(size);
    create_info.pCode = code;

    if(auto result{vkCreateShaderModule(m_device, &create_info, nullptr, &m_shader)}; result != VK_SUCCESS)
        throw error{result};
}

shader::~shader()
{
    if(m_shader)
        vkDestroyShaderModule(m_device, m_shader, nullptr);
}

shader::shader(shader&& other) noexcept
:m_device{other.m_device}
,m_shader{std::exchange(other.m_shader, nullptr)}
{

}

shader& shader::operator=(shader&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_shader = std::exchange(other.m_shader, m_shader);

    return *this;
}

/////////////////////////////////////////////////////////////////////

semaphore::semaphore(VkDevice device)
:m_device{device}
{
    VkSemaphoreCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if(auto result{vkCreateSemaphore(m_device, &create_info, nullptr, &m_semaphore)}; result != VK_SUCCESS)
        throw error{result};
}

semaphore::~semaphore()
{
    if(m_semaphore)
        vkDestroySemaphore(m_device, m_semaphore, nullptr);
}

semaphore::semaphore(semaphore&& other) noexcept
:m_device{other.m_device}
,m_semaphore{std::exchange(other.m_semaphore, nullptr)}
{

}

semaphore& semaphore::operator=(semaphore&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_semaphore = std::exchange(other.m_semaphore, m_semaphore);

    return *this;
}

/////////////////////////////////////////////////////////////////////

fence::fence(VkDevice device, VkFenceCreateFlags flags)
:m_device{device}
{
    VkFenceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    create_info.flags = flags;

    if(auto result{vkCreateFence(m_device, &create_info, nullptr, &m_fence)}; result != VK_SUCCESS)
        throw error{result};
}

fence::~fence()
{
    if(m_fence)
        vkDestroyFence(m_device, m_fence, nullptr);
}

fence::fence(fence&& other) noexcept
:m_device{other.m_device}
,m_fence{std::exchange(other.m_fence, nullptr)}
{

}

fence& fence::operator=(fence&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_fence = std::exchange(other.m_fence, m_fence);

    return *this;
}

/////////////////////////////////////////////////////////////////////

event::event(VkDevice device)
:m_device{device}
{
    VkEventCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;

    if(auto result{vkCreateEvent(m_device, &create_info, nullptr, &m_event)}; result != VK_SUCCESS)
        throw error{result};
}

event::~event()
{
    if(m_event)
        vkDestroyEvent(m_device, m_event, nullptr);
}

event::event(event&& other) noexcept
:m_device{other.m_device}
,m_event{std::exchange(other.m_event, nullptr)}
{

}

event& event::operator=(event&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_event = std::exchange(other.m_event, m_event);

    return *this;
}

/////////////////////////////////////////////////////////////////////

command_pool::command_pool(VkDevice device, uint32_t queue_family, VkCommandPoolCreateFlags flags)
:m_device{device}
{
    VkCommandPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.flags = flags;
    create_info.queueFamilyIndex = queue_family;

    if(auto result{vkCreateCommandPool(m_device, &create_info, nullptr, &m_command_pool)}; result != VK_SUCCESS)
        throw error{result};
}

command_pool::~command_pool()
{
    if(m_command_pool)
        vkDestroyCommandPool(m_device, m_command_pool, nullptr);
}

command_pool::command_pool(command_pool&& other) noexcept
:m_device{other.m_device}
,m_command_pool{std::exchange(other.m_command_pool, nullptr)}
{

}

command_pool& command_pool::operator=(command_pool&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_command_pool = std::exchange(other.m_command_pool, m_command_pool);

    return *this;
}

/////////////////////////////////////////////////////////////////////

command_buffer::command_buffer(VkDevice device, VkCommandPool command_pool, VkCommandBufferLevel level)
:m_device{device}
,m_command_pool{command_pool}
{
    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = command_pool;
    allocate_info.level = level;
    allocate_info.commandBufferCount = 1;

    if(auto result{vkAllocateCommandBuffers(m_device, &allocate_info, &m_command_buffer)}; result != VK_SUCCESS)
        throw error{result};
}

command_buffer::~command_buffer()
{
    if(m_command_buffer)
        vkFreeCommandBuffers(m_device, m_command_pool, 1, &m_command_buffer);
}

command_buffer::command_buffer(command_buffer&& other) noexcept
:m_device{other.m_device}
,m_command_pool{other.m_command_pool}
,m_command_buffer{std::exchange(other.m_command_buffer, nullptr)}
{

}

command_buffer& command_buffer::operator=(command_buffer&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_command_pool = std::exchange(other.m_command_pool, m_command_pool);
    m_command_buffer = std::exchange(other.m_command_buffer, m_command_buffer);

    return *this;
}

/////////////////////////////////////////////////////////////////////

descriptor_set_layout::descriptor_set_layout(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& bindings)
:m_device{device}
{
    VkDescriptorSetLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.bindingCount = static_cast<std::uint32_t>(std::size(bindings));
    create_info.pBindings = std::data(bindings);

    if(auto result{vkCreateDescriptorSetLayout(m_device, &create_info, nullptr, &m_descriptor_set_layout)}; result != VK_SUCCESS)
        throw error{result};
}

descriptor_set_layout::~descriptor_set_layout()
{
    if(m_descriptor_set_layout)
        vkDestroyDescriptorSetLayout(m_device, m_descriptor_set_layout, nullptr);
}

descriptor_set_layout::descriptor_set_layout(descriptor_set_layout&& other) noexcept
:m_device{other.m_device}
,m_descriptor_set_layout{std::exchange(other.m_descriptor_set_layout, nullptr)}
{

}

descriptor_set_layout& descriptor_set_layout::operator=(descriptor_set_layout&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_descriptor_set_layout = std::exchange(other.m_descriptor_set_layout, m_descriptor_set_layout);

    return *this;
}


/////////////////////////////////////////////////////////////////////

descriptor_pool::descriptor_pool(VkDevice device, const std::vector<VkDescriptorPoolSize>& sizes, std::uint32_t max_sets)
:m_device{device}
{
    VkDescriptorPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.poolSizeCount = static_cast<std::uint32_t>(std::size(sizes));
    create_info.pPoolSizes = std::data(sizes);
    create_info.maxSets = max_sets;

    if(auto result{vkCreateDescriptorPool(m_device, &create_info, nullptr, &m_descriptor_pool)}; result != VK_SUCCESS)
        throw error{result};
}

descriptor_pool::~descriptor_pool()
{
    if(m_descriptor_pool)
        vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);
}

descriptor_pool::descriptor_pool(descriptor_pool&& other) noexcept
:m_device{other.m_device}
,m_descriptor_pool{std::exchange(other.m_descriptor_pool, nullptr)}
{

}

descriptor_pool& descriptor_pool::operator=(descriptor_pool&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_descriptor_pool = std::exchange(other.m_descriptor_pool, m_descriptor_pool);

    return *this;
}

/////////////////////////////////////////////////////////////////////

descriptor_set::descriptor_set(VkDevice device, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout)
:m_device{device}
{
    VkDescriptorSetAllocateInfo allocator_info{};
    allocator_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocator_info.descriptorPool = descriptor_pool;
    allocator_info.descriptorSetCount = 1;
    allocator_info.pSetLayouts = &descriptor_set_layout;

    if(auto result{vkAllocateDescriptorSets(m_device, &allocator_info, &m_descriptor_set)}; result != VK_SUCCESS)
        throw error{result};
}

descriptor_set::descriptor_set(descriptor_set&& other) noexcept
:m_device{other.m_device}
,m_descriptor_set{std::exchange(other.m_descriptor_set, nullptr)}
{

}

descriptor_set& descriptor_set::operator=(descriptor_set&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_descriptor_set = std::exchange(other.m_descriptor_set, m_descriptor_set);

    return *this;
}

/////////////////////////////////////////////////////////////////////

pipeline_layout::pipeline_layout(VkDevice device, const std::vector<VkDescriptorSetLayout>& layouts, const std::vector<VkPushConstantRange>& ranges)
:m_device{device}
{
    VkPipelineLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.setLayoutCount = static_cast<std::uint32_t>(std::size(layouts));
    create_info.pSetLayouts = std::data(layouts);
    create_info.pushConstantRangeCount = static_cast<std::uint32_t>(std::size(ranges));
    create_info.pPushConstantRanges = std::data(ranges);

    if(auto result{vkCreatePipelineLayout(m_device, &create_info, nullptr, &m_pipeline_layout)}; result != VK_SUCCESS)
        throw error{result};
}

pipeline_layout::~pipeline_layout()
{
    if(m_pipeline_layout)
        vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
}

pipeline_layout::pipeline_layout(pipeline_layout&& other) noexcept
:m_device{other.m_device}
,m_pipeline_layout{std::exchange(other.m_pipeline_layout, nullptr)}
{

}

pipeline_layout& pipeline_layout::operator=(pipeline_layout&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_pipeline_layout = std::exchange(other.m_pipeline_layout, m_pipeline_layout);

    return *this;
}

/////////////////////////////////////////////////////////////////////

render_pass::render_pass(VkDevice device, const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& dependencies)
:m_device{device}
{
    VkRenderPassCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = static_cast<std::uint32_t>(std::size(attachments));
    create_info.pAttachments = std::data(attachments);
    create_info.subpassCount = static_cast<std::uint32_t>(std::size(subpasses));
    create_info.pSubpasses = std::data(subpasses);
    create_info.dependencyCount = static_cast<std::uint32_t>(std::size(dependencies));
    create_info.pDependencies = std::data(dependencies);

    if(auto result{vkCreateRenderPass(m_device, &create_info, nullptr, &m_render_pass)}; result != VK_SUCCESS)
        throw error{result};
}

render_pass::~render_pass()
{
    if(m_render_pass)
        vkDestroyRenderPass(m_device, m_render_pass, nullptr);
}

render_pass::render_pass(render_pass&& other) noexcept
:m_device{other.m_device}
,m_render_pass{std::exchange(other.m_render_pass, nullptr)}
{

}

render_pass& render_pass::operator=(render_pass&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_render_pass = std::exchange(other.m_render_pass, m_render_pass);

    return *this;
}

/////////////////////////////////////////////////////////////////////

pipeline::pipeline(VkDevice device, const VkGraphicsPipelineCreateInfo& create_info, VkPipelineCache cache)
:m_device{device}
{
    if(auto result{vkCreateGraphicsPipelines(m_device, cache, 1, &create_info, nullptr, &m_pipeline)}; result != VK_SUCCESS)
        throw error{result};
}

pipeline::pipeline(VkDevice device, const VkComputePipelineCreateInfo& create_info, VkPipelineCache cache)
:m_device{device}
{
    if(auto result{vkCreateComputePipelines(m_device, cache, 1, &create_info, nullptr, &m_pipeline)}; result != VK_SUCCESS)
        throw error{result};
}

pipeline::~pipeline()
{
    if(m_pipeline)
        vkDestroyPipeline(m_device, m_pipeline, nullptr);
}

pipeline::pipeline(pipeline&& other) noexcept
:m_device{other.m_device}
,m_pipeline{std::exchange(other.m_pipeline, nullptr)}
{

}

pipeline& pipeline::operator=(pipeline&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_pipeline = std::exchange(other.m_pipeline, m_pipeline);

    return *this;
}

/////////////////////////////////////////////////////////////////////

pipeline_cache::pipeline_cache(VkDevice device, const void* initial_data, std::size_t size)
:m_device{device}
{
    VkPipelineCacheCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    create_info.pInitialData = initial_data;
    create_info.initialDataSize = size;

    if(auto result{vkCreatePipelineCache(m_device, &create_info, nullptr, &m_pipeline_cache)}; result != VK_SUCCESS)
        throw error{result};
}

pipeline_cache::~pipeline_cache()
{
    if(m_pipeline_cache)
        vkDestroyPipelineCache(m_device, m_pipeline_cache, nullptr);
}

pipeline_cache::pipeline_cache(pipeline_cache&& other) noexcept
:m_device{other.m_device}
,m_pipeline_cache{std::exchange(other.m_pipeline_cache, nullptr)}
{

}

pipeline_cache& pipeline_cache::operator=(pipeline_cache&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_pipeline_cache = std::exchange(other.m_pipeline_cache, m_pipeline_cache);

    return *this;
}

/////////////////////////////////////////////////////////////////////

debug_messenger::debug_messenger(VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT callback, VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagBitsEXT type)
:m_instance{instance}
{
    VkDebugUtilsMessengerCreateInfoEXT create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = severity;
    create_info.messageType = type;
    create_info.pfnUserCallback = callback;

    if(auto result{vkCreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, &m_debug_messenger)}; result != VK_SUCCESS)
        throw error{result};
}

debug_messenger::~debug_messenger()
{
    if(m_debug_messenger)
        vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
}

debug_messenger::debug_messenger(debug_messenger&& other) noexcept
:m_instance{other.m_instance}
,m_debug_messenger{std::exchange(other.m_debug_messenger, nullptr)}
{

}

debug_messenger& debug_messenger::operator=(debug_messenger&& other) noexcept
{
    m_instance = std::exchange(other.m_instance, m_instance);
    m_debug_messenger = std::exchange(other.m_debug_messenger, m_debug_messenger);

    return *this;
}


/////////////////////////////////////////////////////////////////////

#ifdef TPH_PLATFORM_ANDROID
surface::surface(VkInstance instance, const android_surface_info& info)
{
    VkAndroidSurfaceCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    create_info.window = info.window;

    if(auto result{vkCreateAndroidSurfaceKHR(instance, &create_info, nullptr, &m_surface)}; result != VK_SUCCESS)
        throw error{result};
}
#endif

#ifdef TPH_PLATFORM_IOS
surface::surface(VkInstance instance, const ios_surface_info& info)
{
    VkIOSSurfaceCreateInfoMVK create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    create_info.pView = info.view;

    if(auto result{vkCreateIOSSurfaceMVK(instance, &create_info, nullptr, &m_surface)}; result != VK_SUCCESS)
        throw error{result};
}
#endif

#ifdef TPH_PLATFORM_WIN32
surface::surface(VkInstance instance, const win32_surface_info& info)
{
    VkWin32SurfaceCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hinstance = reinterpret_cast<HINSTANCE>(info.instance);
    create_info.hwnd = reinterpret_cast<HWND>(info.window);

    if(auto result{vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &m_surface)}; result != VK_SUCCESS)
        throw error{result};
}
#endif

#ifdef TPH_PLATFORM_MACOS
surface::surface(VkInstance instance, const macos_surface_info& info)
{
    VkMacOSSurfaceCreateInfoMVK create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    create_info.pView = info.view;

    if(auto result{vkCreateMacOSSurfaceMVK(instance, &create_info, nullptr, &m_surface)}; result != VK_SUCCESS)
        throw error{result};
}
#endif

#ifdef TPH_PLATFORM_XLIB
surface::surface(VkInstance instance, const xlib_surface_info& info)
{
    VkXlibSurfaceCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    create_info.dpy = info.display;
    create_info.window = info.window;

    if(auto result{vkCreateXlibSurfaceKHR(instance, &create_info, nullptr, &m_surface)}; result != VK_SUCCESS)
        throw error{result};
}
#endif

#ifdef TPH_PLATFORM_XCB
surface::surface(VkInstance instance, const xcb_surface_info& info)
{
    VkXcbSurfaceCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    create_info.connection = info.connection;
    create_info.window = info.window;

    if(auto result{vkCreateXcbSurfaceKHR(instance, &create_info, nullptr, &m_surface)}; result != VK_SUCCESS)
        throw error{result};
}
#endif

#ifdef TPH_PLATFORM_WAYLAND
surface::surface(VkInstance instance, const wayland_surface_info& info)
{
    VkWaylandSurfaceCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    create_info.display = info.display;
    create_info.window = info.window;

    if(auto result{vkCreateWaylandSurfaceKHR(instance, &create_info, nullptr, &m_surface)}; result != VK_SUCCESS)
        throw error{result};
}
#endif

surface::~surface()
{
    if(m_surface)
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}

surface::surface(surface&& other) noexcept
:m_instance{other.m_instance}
,m_surface{std::exchange(other.m_surface, nullptr)}
{

}

surface& surface::operator=(surface&& other) noexcept
{
    m_instance = std::exchange(other.m_instance, m_instance);
    m_surface = std::exchange(other.m_surface, m_surface);

    return *this;
}

swapchain::swapchain(VkDevice device, VkSurfaceKHR surface, VkExtent2D size, std::uint32_t image_count, VkSurfaceFormatKHR format, VkImageUsageFlags usage, const std::vector<std::uint32_t>& families, VkSurfaceTransformFlagBitsKHR transform, VkCompositeAlphaFlagBitsKHR composite, VkPresentModeKHR present_mode, VkBool32 clipped, VkSwapchainKHR old)
:m_device{device}
{
    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.imageExtent = size;
    create_info.minImageCount = image_count;
    create_info.imageFormat = format.format;
    create_info.imageColorSpace = format.colorSpace;
    create_info.imageUsage = usage;
    create_info.imageArrayLayers = 1;

    if(std::empty(families))
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = static_cast<std::uint32_t>(std::size(families));
        create_info.pQueueFamilyIndices = std::data(families);
    }

    create_info.preTransform = transform;
    create_info.compositeAlpha = composite;
    create_info.presentMode = present_mode;
    create_info.clipped = clipped;
    create_info.oldSwapchain = old;

    if(vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swapchain) != VK_SUCCESS)
        throw std::runtime_error{"Failed to create swapchain."};
}

swapchain::~swapchain()
{
    if(m_swapchain)
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}

swapchain::swapchain(swapchain&& other) noexcept
:m_device{other.m_device}
,m_swapchain{std::exchange(other.m_swapchain, nullptr)}
{

}

swapchain& swapchain::operator=(swapchain&& other) noexcept
{
    m_device = std::exchange(other.m_device, m_device);
    m_swapchain = std::exchange(other.m_swapchain, m_swapchain);

    return *this;
}

}

}

