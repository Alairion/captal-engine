#include "query.hpp"

#include "renderer.hpp"

namespace tph
{

query_pool::query_pool(renderer& renderer, std::uint32_t count, query_type type, query_pipeline_statistic statistics)
{

}

void query_pool::reset(renderer& renderer)
{
    vkResetQueryPool(underlying_cast<VkDevice>(renderer), underlying_cast<VkQueryPool>(m_query_pool), )
}

}
