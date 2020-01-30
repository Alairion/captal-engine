#ifndef TEPHRA_DESCRIPTOR_HPP_INCLUDED
#define TEPHRA_DESCRIPTOR_HPP_INCLUDED

#include "config.hpp"

#include <optional>

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

class descriptor_set_layout
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr descriptor_set_layout() = default;
    descriptor_set_layout(renderer& renderer, const std::vector<descriptor_set_layout_binding>& bindings);

    ~descriptor_set_layout() = default;
    descriptor_set_layout(const descriptor_set_layout&) = delete;
    descriptor_set_layout& operator=(const descriptor_set_layout&) = delete;
    descriptor_set_layout(descriptor_set_layout&&) noexcept = default;
    descriptor_set_layout& operator=(descriptor_set_layout&&) noexcept = default;

private:
    vulkan::descriptor_set_layout m_layout{};
};

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

class descriptor_pool
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr descriptor_pool() = default;
    descriptor_pool(renderer& renderer, const std::vector<descriptor_pool_size>& sizes, std::optional<std::uint32_t> max_sets = std::nullopt);

    ~descriptor_pool() = default;
    descriptor_pool(const descriptor_pool&) = delete;
    descriptor_pool& operator=(const descriptor_pool&) = delete;
    descriptor_pool(descriptor_pool&&) noexcept = default;
    descriptor_pool& operator=(descriptor_pool&&) noexcept = default;

private:
    vulkan::descriptor_pool m_descriptor_pool{};
};

template<>
inline VkDescriptorPool underlying_cast(const descriptor_pool& descriptor_pool) noexcept
{
   return descriptor_pool.m_descriptor_pool;
}

class descriptor_set
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr descriptor_set() = default;
    descriptor_set(renderer& renderer, descriptor_pool& pool, descriptor_set_layout& layout);

    ~descriptor_set() = default;
    descriptor_set(const descriptor_set&) = delete;
    descriptor_set& operator=(const descriptor_set&) = delete;
    descriptor_set(descriptor_set&&) noexcept = default;
    descriptor_set& operator=(descriptor_set&&) noexcept = default;

private:
    vulkan::descriptor_set m_descriptor_set{};
};

template<>
inline VkDescriptorSet underlying_cast(const descriptor_set& descriptor_set) noexcept
{
   return descriptor_set.m_descriptor_set;
}

void write_descriptor(renderer& renderer, descriptor_set& descriptor_set, std::uint32_t binding, buffer& buffer, std::uint64_t offset, std::uint64_t size);
void write_descriptor(renderer& renderer, descriptor_set& descriptor_set, std::uint32_t binding, texture& texture);

}

#endif
