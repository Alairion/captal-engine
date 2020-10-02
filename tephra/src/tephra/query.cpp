#include "query.hpp"

#include "vulkan/vulkan_functions.hpp"

#include "renderer.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

query_pool::query_pool(renderer& renderer, std::uint32_t count, query_type type, query_pipeline_statistic statistics)
{

}

void query_pool::reset(std::uint32_t first, std::uint32_t count)
{
    vkResetQueryPool(m_query_pool.device(), m_query_pool, first, count);
}

}
