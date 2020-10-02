#include "query.hpp"

#include "renderer.hpp"

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
