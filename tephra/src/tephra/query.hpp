#ifndef TEPHRA_BUFFER_HPP_INCLUDED
#define TEPHRA_BUFFER_HPP_INCLUDED

#include "config.hpp"

#include "vulkan/vulkan.hpp"

#include "enumerations.hpp"

namespace tph
{

class renderer;

enum class query_type : std::uint32_t
{
    occlusion = VK_QUERY_TYPE_OCCLUSION,
    pipeline_statistics = VK_QUERY_TYPE_PIPELINE_STATISTICS,
    timestamp = VK_QUERY_TYPE_TIMESTAMP,
};

enum class query_pipeline_statistic : std::uint32_t
{
    input_assembly_vertices = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT,
    input_assembly_primitives = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT,
    vertex_shader_invocations = VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT,
    geometry_shader_invocations = VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT,
    geometry_shader_primitives = VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT,
    clipping_invocations = VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT,
    clipping_primitives = VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT,
    fragment_shader_invocations = VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT,
    tessellation_control_shader_patches = VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT,
    tessellation_evaluation_shader_invocations = VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT,
    compute_shader_invocation = VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT,
};

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

    void reset(std::uint32_t first, std::uint32_t count);

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

template<> struct tph::enable_enum_operations<tph::query_pipeline_statistic> {static constexpr bool value{true};};

#endif
