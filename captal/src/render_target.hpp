#ifndef CAPTAL_RENDER_TARGET_HPP_INCLUDED
#define CAPTAL_RENDER_TARGET_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <vector>

#include <tephra/commands.hpp>
#include <tephra/render_target.hpp>

#include "render_technique.hpp"

#include <sigslots/signal.hpp>

namespace cpt
{

using frame_presented_signal = sigslot::signal<>;

class CAPTAL_API render_target
{
public:
    render_target() = default;

    template<typename... Args>
    render_target(Args&&... args)
    :m_render_target{std::forward<Args>(args)...}
    {
        set_render_technique(add_render_technique(render_technique_info{}));
    }

    virtual ~render_target() = default;
    render_target(const render_target&) = delete;
    render_target& operator=(const render_target&) = delete;
    render_target(render_target&&) noexcept = default;
    render_target& operator=(render_target&&) noexcept = default;

    render_technique_ptr add_render_technique(const render_technique_info& info);
    render_technique_ptr add_render_technique(render_technique_ptr technique);
    void remove_render_technique(render_technique_ptr technique);

    virtual std::pair<tph::command_buffer&, frame_presented_signal&> begin_render() = 0;
    virtual void present() = 0;

    tph::render_target& get_target() noexcept
    {
        return m_render_target;
    }

    const tph::render_target& get_target() const noexcept
    {
        return m_render_target;
    }

    void disable_rendering() noexcept
    {
        m_enable = false;
    }

    void enable_rendering() noexcept
    {
        m_enable = true;
    }

    bool is_rendering_enable() const noexcept
    {
        return m_enable;
    }

    void set_render_technique(render_technique_ptr technique)
    {
        m_current_render_technique = technique;
    }

    void set_default_render_technique()
    {
        m_current_render_technique = m_render_techniques[0];
    }

    render_technique_ptr get_render_technique() const
    {
        return m_current_render_technique;
    }

private:
    tph::render_target m_render_target{};
    std::vector<render_technique_ptr> m_render_techniques{};
    render_technique_ptr m_current_render_technique{};

    bool m_enable{true};
};

}

#endif
