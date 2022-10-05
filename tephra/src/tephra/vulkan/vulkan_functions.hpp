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

#ifndef TEPHRA_VULKAN_FUNCTIONS_INCLUDED
#define TEPHRA_VULKAN_FUNCTIONS_INCLUDED

#include <vulkan/vulkan.h>

#ifndef VK_VERSION_1_1
    #error "Tephra requires a Vulkan SDK that exposes at lease Vulkan Core version 1.1"
#endif

namespace tph::vulkan
{

namespace functions
{

// this function are the same for everyone (i.e. system-level)
#define TEPHRA_EXTERNAL_LEVEL_FUNCTION(function) extern PFN_##function function;
#define TEPHRA_GLOBAL_LEVEL_FUNCTION(function) extern PFN_##function function;

#include "vulkan_functions_list"

#define TEPHRA_INSTANCE_LEVEL_FUNCTION(function) PFN_##function function;

struct instance_level_functions
{
    #include "vulkan_functions_list"
};

#define TEPHRA_DEVICE_LEVEL_FUNCTION(function) PFN_##function function;

struct device_level_functions
{
    #include "vulkan_functions_list"
};

void load_external_level_functions();
void load_global_level_functions();
void load_instance_level_functions(VkInstance instance, instance_level_functions& instance_functions) noexcept;
void load_device_level_functions(VkDevice device, const instance_level_functions& instance_functions, device_level_functions& device_functions) noexcept;

}

}

#endif
