#ifndef CAPTAL_RENDER_TARGET_HPP_INCLUDED
#define CAPTAL_RENDER_TARGET_HPP_INCLUDED

#include "config.hpp"

#include <concepts>

#include <tephra/render_target.hpp>

#include "signal.hpp"
#include "texture.hpp"

namespace cpt
{

using frame_time_t = std::chrono::duration<std::uint64_t, std::nano>;
using frame_presented_signal = cpt::signal<>;
using frame_time_signal = cpt::signal<frame_time_t>;

struct frame_render_info
{
    tph::command_buffer& buffer;
    frame_presented_signal& signal;
    asynchronous_resource_keeper& keeper;
};

class CAPTAL_API render_target
{
public:
    render_target() = default;
    explicit render_target(const tph::render_pass_info& info);

    virtual ~render_target() = default;

    virtual frame_time_signal& register_frame_time() = 0;
    virtual frame_render_info begin_render() = 0;
    virtual void present() = 0;

    void disable_rendering() noexcept
    {
        m_enable = false;
    }

    void enable_rendering() noexcept
    {
        m_enable = true;
    }

    tph::render_pass& get_render_pass() noexcept
    {
        return m_render_pass;
    }

    const tph::render_pass& get_render_pass() const noexcept
    {
        return m_render_pass;
    }

    bool is_rendering_enable() const noexcept
    {
        return m_enable;
    }

protected:
    render_target(const render_target&) = delete;
    render_target& operator=(const render_target&) = delete;
    render_target(render_target&&) noexcept = default;
    render_target& operator=(render_target&&) noexcept = default;

private:
    tph::render_pass m_render_pass{};
    bool m_enable{true};
};

using render_target_ptr = std::shared_ptr<render_target>;
using render_target_weak_ptr = std::weak_ptr<render_target>;

}

#endif
