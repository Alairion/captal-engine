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
class texture_view;
class sampler;

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

TEPHRA_API void set_object_name(renderer& renderer, const descriptor_set_layout& object, const std::string& name);

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

TEPHRA_API void set_object_name(renderer& renderer, const descriptor_pool& object, const std::string& name);

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

TEPHRA_API void set_object_name(renderer& renderer, const descriptor_set& object, const std::string& name);

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
    sampler* sampler{};
    texture_view* texture_view{};
    tph::texture_layout layout{};
};

struct descriptor_buffer_info
{
    std::reference_wrapper<buffer> buffer;
    std::uint64_t offset{};
    std::uint64_t size{};
};

using descriptor_info = std::variant<std::monostate, descriptor_texture_info, descriptor_buffer_info>;

struct descriptor_write
{
    std::reference_wrapper<descriptor_set> descriptor_set;
    std::uint32_t binding{};
    std::uint32_t array_index{};
    descriptor_type type{};
    descriptor_info info{};
};

struct descriptor_copy
{
    std::reference_wrapper<descriptor_set> source_set;
    std::uint32_t source_binding{};
    std::uint32_t source_array_index{};
    std::reference_wrapper<descriptor_set> destination_set;
    std::uint32_t destination_binding{};
    std::uint32_t destination_array_index{};
    std::uint32_t count{1};
};

TEPHRA_API void write_descriptors(renderer& renderer, std::span<const descriptor_write> writes);
TEPHRA_API void copy_descriptors(renderer& renderer, std::span<const descriptor_copy> copies);
TEPHRA_API void update_descriptors(renderer& renderer, std::span<const descriptor_write> writes, std::span<const descriptor_copy> copies);

}

#endif
