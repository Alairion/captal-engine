#include "vulkan_functions.hpp"

#include <nes/shared_library.hpp>

namespace tph::vulkan::functions
{

#ifdef _WIN32
    static constexpr const char* vulkan_path{"vulkan-1.dll"};
#elif defined(__APPLE__)
    static constexpr const char* vulkan_path{"libvulkan.dylib"};
#else
    static constexpr const char* vulkan_path{"libvulkan.so"};
#endif

#define TEPHRA_EXTERNAL_LEVEL_FUNCTION(function) PFN_##function function = nullptr;
#define TEPHRA_GLOBAL_LEVEL_FUNCTION(function) PFN_##function function = nullptr;
#define TEPHRA_INSTANCE_LEVEL_FUNCTION(function) PFN_##function function = nullptr;
#define TEPHRA_DEVICE_LEVEL_FUNCTION(function) PFN_##function function = nullptr;

#include "vulkan_functions_list"

void load_external_level_functions()
{
    static nes::shared_library vulkan_library{};
    vulkan_library = nes::shared_library{vulkan_path};

    #define TEPHRA_EXTERNAL_LEVEL_FUNCTION(function) function = vulkan_library.load<PFN_##function>(#function);
    #include "vulkan_functions_list"
}

void load_global_level_functions()
{
    #define TEPHRA_GLOBAL_LEVEL_FUNCTION(function) function = reinterpret_cast<PFN_##function>(reinterpret_cast<void*>(vkGetInstanceProcAddr(nullptr, #function)));
    #include "vulkan_functions_list"
}

void load_instance_level_functions(VkInstance instance)
{
    #define TEPHRA_INSTANCE_LEVEL_FUNCTION(function) function = reinterpret_cast<PFN_##function>(reinterpret_cast<void*>(vkGetInstanceProcAddr(instance, #function)));
    #include "vulkan_functions_list"
}

void load_device_level_functions(VkDevice device)
{
    #define TEPHRA_DEVICE_LEVEL_FUNCTION(function) function = reinterpret_cast<PFN_##function>(reinterpret_cast<void*>(vkGetDeviceProcAddr(device, #function)));
    #include "vulkan_functions_list"
}

}
