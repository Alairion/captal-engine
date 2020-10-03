#ifndef TEPHRA_QUERY_HPP_INCLUDED
#define TEPHRA_QUERY_HPP_INCLUDED

#include "config.hpp"

#include "vulkan/vulkan.hpp"

#include "enumerations.hpp"

namespace tph
{

class renderer;

// From https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/chap18.html#vkGetQueryPoolResults
// Occlusion queries write one integer value - the number of samples passed.
// Pipeline statistics queries write one integer value for each bit that is enabled in the pipelineStatistics when the pool is created,
//   and the statistics values are written in bit order starting from the least significant bit.
// Timestamp queries write one integer value.
// Transform feedback queries write two integers; the first integer is the number of primitives successfully written to the corresponding
//   transform feedback buffer and the second is the number of primitives output to the vertex stream, regardless of whether they were
//   successfully captured or not. In other words, if the transform feedback buffer was sized too small for the number of primitives
//   output by the vertex stream, the first integer represents the number of primitives actually written and the second is the number
//   that would have been written if all the transform feedback buffers associated with that vertex stream were large enough.

class TEPHRA_API query_pool
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr query_pool() = default;
    explicit query_pool(renderer& renderer, std::uint32_t count, query_type type, query_pipeline_statistic statistics = query_pipeline_statistic{});

    explicit query_pool(vulkan::query_pool query_pool) noexcept
    :m_query_pool{std::move(query_pool)}
    {

    }

    ~query_pool() = default;
    query_pool(const query_pool&) = delete;
    query_pool& operator=(const query_pool&) = delete;
    query_pool(query_pool&& other) noexcept = default;
    query_pool& operator=(query_pool&& other) noexcept = default;

    void reset(std::uint32_t first, std::uint32_t count) noexcept;
    bool results(std::uint32_t first, std::uint32_t count, std::size_t buffer_size, void* buffer, std::uint64_t stride, query_results flags);

private:
    vulkan::query_pool m_query_pool{};
};

template<>
inline VkDevice underlying_cast(const query_pool& query_pool) noexcept
{
    return query_pool.m_query_pool.device();
}

template<>
inline VkQueryPool underlying_cast(const query_pool& query_pool) noexcept
{
    return query_pool.m_query_pool;
}

}

#endif
