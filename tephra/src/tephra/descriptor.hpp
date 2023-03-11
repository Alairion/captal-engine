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

#ifndef TEPHRA_DESCRIPTOR_HPP_INCLUDED
#define TEPHRA_DESCRIPTOR_HPP_INCLUDED

#include "config.hpp"

#include <optional>
#include <variant>

#include "vulkan/vulkan.hpp"

#include "enumerations.hpp"

namespace tph
{

class device;
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
    explicit descriptor_set_layout(device& dev, std::span<const descriptor_set_layout_binding> bindings);

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

TEPHRA_API void set_object_name(device& dev, const descriptor_set_layout& object, const std::string& name);

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
    explicit descriptor_pool(device& dev, std::span<const descriptor_pool_size> sizes, std::optional<std::uint32_t> max_sets = std::nullopt);

    explicit descriptor_pool(vulkan::descriptor_pool descriptor_pool) noexcept
    :m_descriptor_pool{std::move(descriptor_pool)}
    {

    }

    ~descriptor_pool() = default;
    descriptor_pool(const descriptor_pool&) = delete;
    descriptor_pool& operator=(const descriptor_pool&) = delete;
    descriptor_pool(descriptor_pool&&) noexcept = default;
    descriptor_pool& operator=(descriptor_pool&&) noexcept = default;

    vulkan::device_context context() const noexcept
    {
        return m_descriptor_pool.context();
    }

private:
    vulkan::descriptor_pool m_descriptor_pool{};
};

TEPHRA_API void set_object_name(device& dev, const descriptor_pool& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const descriptor_pool& desc_pool) noexcept
{
    return desc_pool.m_descriptor_pool.device();
}

template<>
inline VkDescriptorPool underlying_cast(const descriptor_pool& desc_pool) noexcept
{
   return desc_pool.m_descriptor_pool;
}

class TEPHRA_API descriptor_set
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr descriptor_set() = default;
    explicit descriptor_set(device& dev, descriptor_pool& pool, descriptor_set_layout& layout);

    explicit descriptor_set(vulkan::descriptor_set desc_set) noexcept
    :m_descriptor_set{std::move(desc_set)}
    {

    }

    ~descriptor_set() = default;
    descriptor_set(const descriptor_set&) = delete;
    descriptor_set& operator=(const descriptor_set&) = delete;
    descriptor_set(descriptor_set&&) noexcept = default;
    descriptor_set& operator=(descriptor_set&&) noexcept = default;

    vulkan::device_context context() const noexcept
    {
        return m_descriptor_set.context();
    }

private:
    vulkan::descriptor_set m_descriptor_set{};
};

TEPHRA_API void set_object_name(device& dev, const descriptor_set& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const descriptor_set& desc_set) noexcept
{
    return desc_set.m_descriptor_set.device();
}

template<>
inline VkDescriptorSet underlying_cast(const descriptor_set& desc_set) noexcept
{
   return desc_set.m_descriptor_set;
}

struct descriptor_texture_info
{
    tph::sampler* sampler{};
    tph::texture_view* texture_view{};
    tph::texture_layout layout{};
};

struct descriptor_buffer_info
{
    std::reference_wrapper<tph::buffer> buffer;
    std::uint64_t offset{};
    std::uint64_t size{};
};

using descriptor_info = std::variant<std::monostate, descriptor_texture_info, descriptor_buffer_info>;

struct descriptor_write
{
    std::reference_wrapper<tph::descriptor_set> descriptor_set;
    std::uint32_t binding{};
    std::uint32_t array_index{};
    descriptor_type type{};
    descriptor_info info{};
};

struct descriptor_copy
{
    std::reference_wrapper<tph::descriptor_set> source_set;
    std::uint32_t source_binding{};
    std::uint32_t source_array_index{};
    std::reference_wrapper<tph::descriptor_set> dest_set;
    std::uint32_t dest_binding{};
    std::uint32_t dest_array_index{};
    std::uint32_t count{1};
};

TEPHRA_API void write_descriptors(device& dev, std::span<const descriptor_write> writes);
TEPHRA_API void copy_descriptors(device& dev, std::span<const descriptor_copy> copies);
TEPHRA_API void update_descriptors(device& dev, std::span<const descriptor_write> writes, std::span<const descriptor_copy> copies);

}

#endif
