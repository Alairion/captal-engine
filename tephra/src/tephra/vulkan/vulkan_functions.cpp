//MIT License
//
//Copyright (c) 2021 Alexy Pellegrini
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "vulkan_functions.hpp"

#include <mutex>
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
    static std::mutex mutex{};
    std::lock_guard lock{mutex};

    static nes::shared_library vulkan_library{};
    static bool loaded{};

    if(!std::exchange(loaded, true))
    {
        vulkan_library = nes::shared_library{vulkan_path};

        #define TEPHRA_EXTERNAL_LEVEL_FUNCTION(function) function = vulkan_library.load<PFN_##function>(#function);
        #include "vulkan_functions_list"
    }
}

void load_global_level_functions()
{
    static std::mutex mutex{};
    std::lock_guard lock{mutex};

    static bool loaded{};

    if(!std::exchange(loaded, true))
    {
        #define TEPHRA_GLOBAL_LEVEL_FUNCTION(function) function = reinterpret_cast<PFN_##function>(vkGetInstanceProcAddr(nullptr, #function));
        #include "vulkan_functions_list"
    }
}

void load_instance_level_functions(VkInstance instance)
{
    #define TEPHRA_INSTANCE_LEVEL_FUNCTION(function) function = reinterpret_cast<PFN_##function>(vkGetInstanceProcAddr(instance, #function));
    #include "vulkan_functions_list"
}

void load_device_level_functions(VkDevice device)
{
    #define TEPHRA_DEVICE_LEVEL_FUNCTION(function) function = reinterpret_cast<PFN_##function>(vkGetDeviceProcAddr(device, #function));
    #include "vulkan_functions_list"
}

}
