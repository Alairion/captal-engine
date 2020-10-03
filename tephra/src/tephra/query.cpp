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

}
