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

#include "query.hpp"

#include "vulkan/vulkan_functions.hpp"

#include "renderer.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

query_pool::query_pool(renderer& renderer, std::uint32_t count, query_type type, query_pipeline_statistic statistics)
:m_query_pool{underlying_cast<VkDevice>(renderer), static_cast<VkQueryType>(type), count, static_cast<VkQueryPipelineStatisticFlagBits>(statistics)}
{

}

void query_pool::reset(std::uint32_t first, std::uint32_t count) noexcept
{
    vkResetQueryPool(m_query_pool.device(), m_query_pool, first, count);
}

bool query_pool::results(std::uint32_t first, std::uint32_t count, std::size_t buffer_size, void* buffer, std::uint64_t stride, query_results flags)
{
    const auto result{vkGetQueryPoolResults(m_query_pool.device(), m_query_pool, first, count, buffer_size, buffer, stride, static_cast<VkQueryResultFlags>(flags))};

    if(result == VK_NOT_READY)
    {
        return false;
    }
    else if(result != VK_SUCCESS)
    {
        throw vulkan::error{result};
    }

    return true;
}

void set_object_name(renderer& renderer, const query_pool& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_QUERY_POOL;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkQueryPool>(object));
    info.pObjectName = std::data(name);

    if(auto result{vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(renderer), &info)}; result != VK_SUCCESS)
        throw vulkan::error{result};
}

}
