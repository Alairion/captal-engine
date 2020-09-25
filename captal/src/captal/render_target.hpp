#ifndef CAPTAL_RENDER_TARGET_HPP_INCLUDED
#define CAPTAL_RENDER_TARGET_HPP_INCLUDED

#include "config.hpp"

#include <tephra/render_target.hpp>

#include "signal.hpp"
#include "texture.hpp"

namespace cpt
{

using frame_presented_signal = cpt::signal<>;

class CAPTAL_API render_target
{
public:
    render_target() = default;
    render_target(const tph::render_pass_info& info);

    virtual ~render_target() = default;
    render_target(const render_target&) = delete;
    render_target& operator=(const render_target&) = delete;
    render_target(render_target&&) noexcept = default;
    render_target& operator=(render_target&&) noexcept = default;

    virtual std::pair<tph::command_buffer&, frame_presented_signal&> begin_render() = 0;
    virtual void present() = 0;

    tph::render_pass& get_render_pass() noexcept
    {
        return m_render_pass;
    }

    const tph::render_pass& get_render_pass() const noexcept
    {
        return m_render_pass;
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

private:
    tph::render_pass m_render_pass{};
    bool m_enable{true};
};

using render_target_ptr = std::shared_ptr<render_target>;
using render_target_weak_ptr = std::weak_ptr<render_target>;

}

#endif
