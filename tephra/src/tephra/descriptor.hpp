#ifndef TEPHRA_DESCRIPTOR_HPP_INCLUDED
#define TEPHRA_DESCRIPTOR_HPP_INCLUDED

#include "config.hpp"

#include <optional>
#include <variant>

#include "vulkan/vulkan.hpp"

#include "enumerations.hpp"

namespace tph
{

class renderer;
class buffer;
class texture;

struct descriptor_set_layout_binding
{
    shader_stage stages{};
    std::uint32_t binding{};
    descriptor_type type{};
    std::uint32_t count{1};
};

class TEPHRA_API descriptor_set_layout
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr descriptor_set_layout() = default;
    explicit descriptor_set_layout(renderer& renderer, std::span<const descriptor_set_layout_binding> bindings);

    explicit descriptor_set_layout(vulkan::descriptor_set_layout layout) noexcept
    :m_layout{std::move(layout)}
    {

    }

    ~descriptor_set_layout() = default;
    descriptor_set_layout(const descriptor_set_layout&) = delete;
    descriptor_set_layout& operator=(const descriptor_set_layout&) = delete;
    descriptor_set_layout(descriptor_set_layout&&) noexcept = default;
    descriptor_set_layout& operator=(descriptor_set_layout&&) noexcept = default;

private:
    vulkan::descriptor_set_layout m_layout{};
};

template<>
inline VkDevice underlying_cast(const descriptor_set_layout& layout) noexcept
{
    return layout.m_layout.device();
}

template<>
inline VkDescriptorSetLayout underlying_cast(const descriptor_set_layout& layout) noexcept
{
   return layout.m_layout;
}

struct descriptor_pool_size
{
    descriptor_type type{};
    std::uint32_t count{1};
};

class TEPHRA_API descriptor_pool
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr descriptor_pool() = default;
    explicit descriptor_pool(renderer& renderer, std::span<const descriptor_pool_size> sizes, std::optional<std::uint32_t> max_sets = std::nullopt);

    explicit descriptor_pool(vulkan::descriptor_pool descriptor_pool) noexcept
    :m_descriptor_pool{std::move(descriptor_pool)}
    {

    }

    ~descriptor_pool() = default;
    descriptor_pool(const descriptor_pool&) = delete;
    descriptor_pool& operator=(const descriptor_pool&) = delete;
    descriptor_pool(descriptor_pool&&) noexcept = default;
    descriptor_pool& operator=(descriptor_pool&&) noexcept = default;

private:
    vulkan::descriptor_pool m_descriptor_pool{};
};

template<>
inline VkDevice underlying_cast(const descriptor_pool& descriptor_pool) noexcept
{
    return descriptor_pool.m_descriptor_pool.device();
}

template<>
inline VkDescriptorPool underlying_cast(const descriptor_pool& descriptor_pool) noexcept
{
   return descriptor_pool.m_descriptor_pool;
}

class TEPHRA_API descriptor_set
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr descriptor_set() = default;
    explicit descriptor_set(renderer& renderer, descriptor_pool& pool, descriptor_set_layout& layout);

    explicit descriptor_set(vulkan::descriptor_set descriptor_set) noexcept
    :m_descriptor_set{std::move(descriptor_set)}
    {

    }

    ~descriptor_set() = default;
    descriptor_set(const descriptor_set&) = delete;
    descriptor_set& operator=(const descriptor_set&) = delete;
    descriptor_set(descriptor_set&&) noexcept = default;
    descriptor_set& operator=(descriptor_set&&) noexcept = default;

private:
    vulkan::descriptor_set m_descriptor_set{};
};

template<>
inline VkDevice underlying_cast(const descriptor_set& descriptor_set) noexcept
{
    return descriptor_set.m_descriptor_set.device();
}

template<>
inline VkDescriptorSet underlying_cast(const descriptor_set& descriptor_set) noexcept
{
   return descriptor_set.m_descriptor_set;
}

struct descriptor_texture_info
{
    texture& texture;
    tph::texture_layout layout{};
};

struct descriptor_buffer_info
{
    buffer& buffer;
    std::uint64_t offset{};
    std::uint64_t size{};
};

struct descriptor_write
{
    descriptor_set& descriptor_set;
    std::uint32_t binding{};
    std::uint32_t array_index{};
    descriptor_type type{};
    std::variant<std::monostate, descriptor_texture_info, descriptor_buffer_info> info{};
};

TEPHRA_API void write_descriptor(renderer& renderer, descriptor_set& descriptor_set, std::uint32_t binding, std::uint32_t array_index, descriptor_type type, buffer& buffer, std::uint64_t offset, std::uint64_t size);
TEPHRA_API void write_descriptor(renderer& renderer, descriptor_set& descriptor_set, std::uint32_t binding, std::uint32_t array_index, descriptor_type type, texture& texture, tph::texture_layout layout);
TEPHRA_API void write_descriptors(renderer& renderer, std::span<const descriptor_write> writes);

}

#endif
