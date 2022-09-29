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

#ifndef TEPHRA_BUFFER_HPP_INCLUDED
#define TEPHRA_BUFFER_HPP_INCLUDED

#include "config.hpp"

#include "vulkan/vulkan.hpp"
#include "vulkan/memory.hpp"

#include "enumerations.hpp"

namespace tph
{

class device;
class command_buffer;

enum class buffer_usage : std::uint32_t
{
    transfer_source = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    transfer_destination = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    uniform_texel = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
    storage_texel = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
    uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    storage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    index = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    vertex = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    indirect = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
    device_only = 0x10000000,
    staging = 0x20000000,
};

class TEPHRA_API buffer
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

    friend class image;

public:
    constexpr buffer() = default;
    explicit buffer(device& device, std::uint64_t size, buffer_usage usage);

    explicit buffer(vulkan::buffer buffer, vulkan::memory_heap_chunk memory, std::uint64_t size) noexcept
    :m_buffer{std::move(buffer)}
    ,m_memory{std::move(memory)}
    ,m_size{size}
    {

    }

    ~buffer() = default;
    buffer(const buffer&) = delete;
    buffer& operator=(const buffer&) = delete;
    buffer(buffer&& other) noexcept = default;
    buffer& operator=(buffer&& other) noexcept = default;

    std::uint8_t* map();
    const std::uint8_t* map() const;
    void unmap() noexcept;

    vulkan::device_context context() const noexcept
    {
        return m_buffer.context();
    }

    std::uint64_t size() const noexcept
    {
        return m_size;
    }

private:
    vulkan::buffer m_buffer{};
    vulkan::memory_heap_chunk m_memory{};
    std::uint64_t m_size{};
};

TEPHRA_API void set_object_name(device& device, const buffer& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const buffer& buffer) noexcept
{
    return buffer.m_buffer.device();
}

template<>
inline VkBuffer underlying_cast(const buffer& buffer) noexcept
{
    return buffer.m_buffer;
}

}

template<> struct tph::enable_enum_operations<tph::buffer_usage> {static constexpr bool value{true};};

#endif
