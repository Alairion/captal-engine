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
    explicit descriptor_pool(render_layout& parent, std::size_t layout_index, tph::descriptor_pool pool);
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
    std::vector<tph::descriptor_set_layout_binding> view_bindings{};
    std::vector<tph::descriptor_set_layout_binding> renderable_bindings{};
    std::vector<tph::push_constant_range> view_push_constants{};
    std::vector<tph::push_constant_range> renderable_push_constants{};
};

class CAPTAL_API render_layout : public asynchronous_resource
{
public:
    explicit render_layout(render_layout_info info);
    ~render_layout() = default;
    render_layout(const render_layout&) = delete;
    render_layout& operator=(const render_layout&) = delete;
    render_layout(render_layout&&) noexcept = delete;
    render_layout& operator=(render_layout&&) noexcept = delete;

    descriptor_set_ptr make_set(std::uint32_t layout_index);

    void add_binding(std::uint32_t layout_index, std::uint32_t binding_index, cpt::binding binding);
    void set_binding(std::uint32_t layout_index, std::uint32_t binding_index, cpt::binding new_binding);

    const render_layout_info& info() const noexcept
    {
        return m_info;
    }

    optional_ref<const cpt::binding> binding(std::uint32_t layout_index, std::uint32_t binding_index) const
    {
        const auto it{m_bindings.find(make_binding_key(layout_index, binding_index))};

        if(it != std::end(m_bindings))
        {
            return it->second;
        }

        return nullref;
    }

    bool has_binding(std::uint32_t layout_index, std::uint32_t binding_index) const
    {
        return m_bindings.find(make_binding_key(layout_index, binding_index)) != std::end(m_bindings);
    }

    tph::descriptor_set_layout& descriptor_set_layout(std::uint32_t layout_index) noexcept
    {
        return m_set_layouts[layout_index];
    }

    const tph::descriptor_set_layout& descriptor_set_layout(std::uint32_t layout_index) const noexcept
    {
        return m_set_layouts[layout_index];
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
        std::vector<tph::descriptor_pool_size> sizes{};
        std::vector<std::unique_ptr<descriptor_pool>> pools{};
    };

    static std::uint64_t make_binding_key(std::uint32_t layout_index, std::uint32_t binding_index) noexcept
    {
        return static_cast<std::uint64_t>(layout_index) << 32 | binding_index;
    }

private:
    render_layout_info m_info{};
    std::unordered_map<std::uint64_t, cpt::binding> m_bindings{};
    std::vector<tph::descriptor_set_layout> m_set_layouts{};
    std::vector<layout_data> m_set_layout_data{};
    tph::pipeline_layout m_layout{};
    std::mutex m_mutex{};

#ifdef CAPTAL_DEBUG
    std::string m_name{};
#endif
};

using render_layout_ptr = std::shared_ptr<render_layout>;
using render_layout_weak_ptr = std::weak_ptr<render_layout>;

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
