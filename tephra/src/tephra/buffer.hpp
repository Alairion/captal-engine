#ifndef TEPHRA_BUFFER_HPP_INCLUDED
#define TEPHRA_BUFFER_HPP_INCLUDED

#include "config.hpp"

#include "vulkan/vulkan.hpp"
#include "vulkan/memory.hpp"

#include "enumerations.hpp"

namespace tph
{

class renderer;

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
    buffer(renderer& renderer, std::uint64_t size, buffer_usage usage);
    ~buffer() = default;
    buffer(const buffer&) = delete;
    buffer& operator=(const buffer&) = delete;
    buffer(buffer&& other) noexcept = default;
    buffer& operator=(buffer&& other) noexcept = default;

    std::uint8_t* map();
    const std::uint8_t* map() const;
    void unmap();

    std::uint64_t size() const noexcept
    {
        return m_size;
    }

private:
    vulkan::buffer m_buffer{};
    vulkan::memory_heap_chunk m_memory{};
    std::uint64_t m_size{};
};

template<>
inline VkBuffer underlying_cast(const buffer& buffer) noexcept
{
    return buffer.m_buffer;
}

}

template<> struct tph::enable_enum_operations<tph::buffer_usage> {static constexpr bool value{true};};

#endif
