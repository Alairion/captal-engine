#ifndef CAPTAL_RENDER_TECHNIQUE_HPP_INCLUDED
#define CAPTAL_RENDER_TECHNIQUE_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <mutex>

#include <tephra/shader.hpp>
#include <tephra/pipeline.hpp>
#include <tephra/descriptor.hpp>

#include "asynchronous_resource.hpp"
#include "render_target.hpp"

namespace cpt
{

//reserved bindings:
//0: view uniform
//1: model uniform
//2: texture sampler

class render_target;
class descriptor_pool;
class render_technique;

class CAPTAL_API descriptor_set : public asynchronous_resource
{
public:
    descriptor_set() = default;
    descriptor_set(descriptor_pool& parent, tph::descriptor_set set) noexcept;
    ~descriptor_set() = default;
    descriptor_set(const descriptor_set&) = delete;
    descriptor_set& operator=(const descriptor_set&) = delete;
    descriptor_set(descriptor_set&& other) noexcept = default;
    descriptor_set& operator=(descriptor_set&& other) noexcept = default;

    descriptor_pool& pool() const noexcept
    {
        return *m_parent;
    }

    tph::descriptor_set& set() noexcept
    {
        return m_set;
    }

    const tph::descriptor_set& set() const noexcept
    {
        return m_set;
    }

private:
    descriptor_pool* m_parent{};
    tph::descriptor_set m_set{};
};

using descriptor_set_ptr = std::shared_ptr<descriptor_set>;
using descriptor_set_weak_ptr = std::weak_ptr<descriptor_set>;

class CAPTAL_API descriptor_pool
{
public:
    constexpr static std::size_t pool_size{16};

public:
    descriptor_pool() = default;
    descriptor_pool(render_technique& parent, tph::descriptor_pool pool);
    ~descriptor_pool() = default;
    descriptor_pool(const descriptor_pool&) = delete;
    descriptor_pool& operator=(const descriptor_pool&) = delete;
    descriptor_pool(descriptor_pool&&) noexcept = delete;
    descriptor_pool& operator=(descriptor_pool&&) noexcept = delete;

    std::optional<descriptor_set_ptr> allocate();
    bool unused() const noexcept;

    render_technique& technique() noexcept
    {
        return *m_parent;
    }

    const render_technique& technique() const noexcept
    {
        return *m_parent;
    }

    tph::descriptor_pool& pool() noexcept
    {
        return m_pool;
    }

    const tph::descriptor_pool& pool() const noexcept
    {
        return m_pool;
    }

private:
    render_technique* m_parent{};
    tph::descriptor_pool m_pool{};
    std::array<descriptor_set_ptr, pool_size> m_sets{};
};

struct render_technique_info
{
    std::vector<tph::pipeline_shader_stage> stages{};
    std::vector<tph::descriptor_set_layout_binding> stages_bindings{};
    std::vector<tph::push_constant_range> push_constant_ranges{};
    tph::pipeline_tessellation tesselation{};
    tph::pipeline_rasterization rasterization{};
    tph::pipeline_multisample multisample{};
    tph::pipeline_depth_stencil depth_stencil{};
    tph::pipeline_color_blend color_blend{};
};

class CAPTAL_API render_technique : public asynchronous_resource
{
public:
    render_technique() = default;
    render_technique(const render_target_ptr& target, const render_technique_info& info);
    ~render_technique() = default;
    render_technique(const render_technique&) = delete;
    render_technique& operator=(const render_technique&) = delete;
    render_technique(render_technique&&) noexcept = delete;
    render_technique& operator=(render_technique&&) noexcept = delete;

    descriptor_set_ptr make_set();

    template<typename T>
    T& get_push_constant(std::size_t index) noexcept
    {
        assert(m_ranges[index].size == sizeof(T) && "Size of T does not match range size.");

        return *reinterpret_cast<T*>(std::data(m_push_constant_buffer) + m_ranges[index].offset);
    }

    template<typename T>
    const T& get_push_constant(std::size_t index) const noexcept
    {
        assert(m_ranges[index].size == sizeof(T) && "Size of T does not match range size.");

        return *reinterpret_cast<const T*>(std::data(m_push_constant_buffer) + m_ranges[index].offset);
    }

    template<typename T>
    void set_push_constant(std::size_t index, T&& value) noexcept
    {
        assert(m_ranges[index].size == sizeof(T) && "Size of T does not match range size.");

        get_push_constant<T>(index) = std::forward<T>(value);
    }

    const std::vector<tph::descriptor_set_layout_binding>& bindings() const noexcept
    {
        return m_bindings;
    }

    const std::vector<tph::push_constant_range>& ranges() const noexcept
    {
        return m_ranges;
    }

    tph::descriptor_set_layout& descriptor_set_layout() noexcept
    {
        return m_descriptor_set_layout;
    }

    const tph::descriptor_set_layout& descriptor_set_layout() const noexcept
    {
        return m_descriptor_set_layout;
    }

    tph::pipeline_layout& pipeline_layout() noexcept
    {
        return m_layout;
    }

    const tph::pipeline_layout& pipeline_layout() const noexcept
    {
        return m_layout;
    }

    tph::pipeline& pipeline() noexcept
    {
        return m_pipeline;
    }

    const tph::pipeline& pipeline() const noexcept
    {
        return m_pipeline;
    }

    const std::vector<std::uint8_t>& push_constant_buffer() const noexcept
    {
        return m_push_constant_buffer;
    }

private:
    std::vector<tph::descriptor_set_layout_binding> m_bindings{};
    std::vector<tph::push_constant_range> m_ranges{};
    std::vector<tph::descriptor_pool_size> m_sizes{};
    tph::descriptor_set_layout m_descriptor_set_layout{};
    tph::pipeline_layout m_layout{};
    tph::pipeline m_pipeline{};
    std::mutex m_mutex{};
    std::vector<std::unique_ptr<descriptor_pool>> m_pools{};
    std::vector<std::uint8_t> m_push_constant_buffer{};
};

using render_technique_ptr = std::shared_ptr<render_technique>;
using render_technique_weak_ptr = std::weak_ptr<render_technique>;

template<typename... Args>
render_technique_ptr make_render_technique(Args&&... args)
{
    return std::make_shared<render_technique>(std::forward<Args>(args)...);
}

}

#endif
