#ifndef TEPHRA_VULKAN_FUNCTIONS_INCLUDED
#define TEPHRA_VULKAN_FUNCTIONS_INCLUDED

#include <vulkan/vulkan.h>

namespace tph::vulkan::functions
{

#define TEPHRA_EXTERNAL_LEVEL_FUNCTION(function) extern PFN_##function function;
#define TEPHRA_GLOBAL_LEVEL_FUNCTION(function) extern PFN_##function function;
#define TEPHRA_INSTANCE_LEVEL_FUNCTION(function) extern PFN_##function function;
#define TEPHRA_DEVICE_LEVEL_FUNCTION(function) extern PFN_##function function;

#include "vulkan_functions_list"

void load_external_level_functions();
void load_global_level_functions();
void load_instance_level_functions(VkInstance instance);
void load_device_level_functions(VkDevice device);

}

#endif
