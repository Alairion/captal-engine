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
#include "signal.hpp"
#include "binding.hpp"

namespace cpt
{

class render_target;
class descriptor_pool;
class render_layout;

class CAPTAL_API descriptor_set : public asynchronous_resource
{
public:
    descriptor_set() = default;
    explicit descriptor_set(descriptor_pool& parent, tph::descriptor_set set) noexcept;

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
    static constexpr std::uint32_t pool_size{32};

public:
    explicit descriptor_pool(render_layout& parent, tph::descriptor_set_layout& layout, tph::descriptor_pool pool);
    ~descriptor_pool() = default;
    descriptor_pool(const descriptor_pool&) = delete;
    descriptor_pool& operator=(const descriptor_pool&) = delete;
    descriptor_pool(descriptor_pool&&) noexcept = delete;
    descriptor_pool& operator=(descriptor_pool&&) noexcept = delete;

    descriptor_set_ptr allocate() noexcept;
    bool unused() const noexcept;

    render_layout& layout() noexcept
    {
        return *m_parent;
    }

    const render_layout& layout() const noexcept
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

#ifdef CAPTAL_DEBUG
    void set_name(std::string_view name);
#else
    void set_name(std::string_view name [[maybe_unused]]) const noexcept
    {

    }
#endif

private:
    render_layout* m_parent{};
    tph::descriptor_pool m_pool{};
    std::array<descriptor_set_ptr, pool_size> m_sets{};
};

struct render_layout_info
{
    std::vector<tph::descriptor_set_layout_binding> bindings{};
    std::vector<tph::push_constant_range> push_constants{};
    std::unordered_map<std::uint32_t, cpt::binding> default_bindings{};
};

class CAPTAL_API render_layout : public asynchronous_resource
{
public:
    static constexpr std::uint32_t view_index{0};
    static constexpr std::uint32_t renderable_index{1};
    static constexpr std::uint32_t user_index{2};

public:
    explicit render_layout(const render_layout_info& view_info, const render_layout_info& renderable_info, std::span<const render_layout_info> user_info = {});
    ~render_layout() = default;
    render_layout(const render_layout&) = delete;
    render_layout& operator=(const render_layout&) = delete;
    render_layout(render_layout&&) noexcept = delete;
    render_layout& operator=(render_layout&&) noexcept = delete;

    descriptor_set_ptr make_set(std::uint32_t layout_index);

    tph::descriptor_set_layout& descriptor_set_layout(std::uint32_t layout_index) noexcept
    {
        return m_layout_data[layout_index].layout;
    }

    const tph::descriptor_set_layout& descriptor_set_layout(std::uint32_t layout_index) const noexcept
    {
        return m_layout_data[layout_index].layout;
    }

    std::span<const tph::descriptor_set_layout_binding> bindings(std::uint32_t layout_index) const noexcept
    {
        return m_layout_data[layout_index].bindings;
    }

    optional_ref<const cpt::binding> default_binding(std::uint32_t layout_index, std::uint32_t binding_index) const noexcept
    {
        return m_layout_data[layout_index].default_bindings.try_get(binding_index);
    }

    std::span<const tph::push_constant_range> push_constants(std::uint32_t layout_index) const noexcept
    {
        return m_layout_data[layout_index].push_constants;
    }

    std::size_t user_layout_count() const noexcept
    {
        return std::size(m_layout_data) - 2;
    }

    tph::pipeline_layout& pipeline_layout() noexcept
    {
        return m_layout;
    }

    const tph::pipeline_layout& pipeline_layout() const noexcept
    {
        return m_layout;
    }

#ifdef CAPTAL_DEBUG
    void set_name(std::string_view name);
#else
    void set_name(std::string_view name [[maybe_unused]]) const noexcept
    {

    }
#endif

private:
    struct layout_data
    {
        tph::descriptor_set_layout layout{};
        std::vector<tph::descriptor_set_layout_binding> bindings{};
        binding_buffer default_bindings{};
        std::vector<tph::push_constant_range> push_constants{};
        std::vector<tph::descriptor_pool_size> sizes{};
        std::vector<std::unique_ptr<descriptor_pool>> pools{};
    };

    static layout_data make_layout_data(const render_layout_info& info);
    static std::vector<tph::push_constant_range> make_push_constant_ranges(std::span<const layout_data> layouts);
    static std::vector<std::reference_wrapper<tph::descriptor_set_layout>> make_layout_refs(std::span<layout_data> layouts);

private:
    std::vector<layout_data> m_layout_data{};
    tph::pipeline_layout m_layout{};
    std::mutex m_mutex{};

#ifdef CAPTAL_DEBUG
    std::string m_name{};
#endif
};

using render_layout_ptr = std::shared_ptr<render_layout>;
using render_layout_weak_ptr = std::weak_ptr<render_layout>;

template<typename... Args>
render_layout_ptr make_render_layout(Args&&... args)
{
    return std::make_shared<render_layout>(std::forward<Args>(args)...);
}

enum class render_technique_options : std::uint32_t
{
    none = 0x00,
    no_default_color_blend_attachment = 0x01,

    no_defaults = no_default_color_blend_attachment
};

struct render_technique_info
{
    std::vector<tph::pipeline_shader_stage> stages{};
    tph::pipeline_tessellation tesselation{};
    tph::pipeline_rasterization rasterization{};
    tph::pipeline_multisample multisample{};
    tph::pipeline_depth_stencil depth_stencil{};
    tph::pipeline_color_blend color_blend{};
};

class CAPTAL_API render_technique : public asynchronous_resource
{
public:
    explicit render_technique(const render_target_ptr& target, const render_technique_info& info, render_layout_ptr layout = nullptr, render_technique_options options = render_technique_options::none);
    ~render_technique() = default;
    render_technique(const render_technique&) = delete;
    render_technique& operator=(const render_technique&) = delete;
    render_technique(render_technique&&) noexcept = delete;
    render_technique& operator=(render_technique&&) noexcept = delete;

    const render_layout_ptr& layout() const noexcept
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

#ifdef CAPTAL_DEBUG
    void set_name(std::string_view name);
#else
    void set_name(std::string_view name [[maybe_unused]]) const noexcept
    {

    }
#endif

private:
    render_layout_ptr m_layout{};
    tph::pipeline m_pipeline{};
};

using render_technique_ptr = std::shared_ptr<render_technique>;
using render_technique_weak_ptr = std::weak_ptr<render_technique>;

template<typename... Args>
render_technique_ptr make_render_technique(Args&&... args)
{
    return std::make_shared<render_technique>(std::forward<Args>(args)...);
}

}

template<> struct cpt::enable_enum_operations<cpt::render_technique_options> {static constexpr bool value{true};};

#endif
