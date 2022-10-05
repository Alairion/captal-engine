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

#ifndef TEPHRA_QUERY_HPP_INCLUDED
#define TEPHRA_QUERY_HPP_INCLUDED

#include "config.hpp"

#include "vulkan/vulkan.hpp"

#include "enumerations.hpp"

namespace tph
{

class device;

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
    explicit query_pool(device& dev, std::uint32_t count, query_type type, query_pipeline_statistic statistics = query_pipeline_statistic{});

    explicit query_pool(vulkan::query_pool pool) noexcept
    :m_query_pool{std::move(pool)}
    {

    }

    ~query_pool() = default;
    query_pool(const query_pool&) = delete;
    query_pool& operator=(const query_pool&) = delete;
    query_pool(query_pool&& other) noexcept = default;
    query_pool& operator=(query_pool&& other) noexcept = default;

    vulkan::device_context context() const noexcept
    {
        return m_query_pool.context();
    }

    void reset(std::uint32_t first, std::uint32_t count) noexcept;
    bool results(std::uint32_t first, std::uint32_t count, std::size_t buffer_size, void* buffer, std::uint64_t stride, query_results flags);

private:
    vulkan::query_pool m_query_pool{};
};

TEPHRA_API void set_object_name(device& dev, const query_pool& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const query_pool& pool) noexcept
{
    return pool.m_query_pool.device();
}

template<>
inline VkQueryPool underlying_cast(const query_pool& pool) noexcept
{
    return pool.m_query_pool;
}

}

#endif
