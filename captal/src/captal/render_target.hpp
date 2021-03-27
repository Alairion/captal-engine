#ifndef CAPTAL_RENDER_TARGET_HPP_INCLUDED
#define CAPTAL_RENDER_TARGET_HPP_INCLUDED

#include "config.hpp"

#include <concepts>

#include <tephra/render_target.hpp>

#include "signal.hpp"
#include "texture.hpp"
#include "window.hpp"

namespace cpt
{

using frame_time_t = std::chrono::duration<std::uint64_t, std::nano>;
using frame_presented_signal = cpt::signal<>;
using frame_time_signal = cpt::signal<frame_time_t>;

struct current_target_t{};
inline constexpr current_target_t current_target{};

using render_target_attachment = std::variant<texture_ptr, current_target_t>;

struct frame_render_info
{
    tph::command_buffer& buffer;
    frame_presented_signal& signal;
    asynchronous_resource_keeper& keeper;
    optional_ref<frame_time_signal> time_signal{};
};

enum class begin_render_options : std::uint32_t
{
    none = 0x00,
    timed = 0x01,
    reset = 0x02
};

class CAPTAL_API render_target
{
public:
    render_target() = default;
    explicit render_target(const tph::render_pass_info& info);

    virtual ~render_target() = default;

    virtual std::optional<frame_render_info> begin_render(begin_render_options options) = 0;
    virtual void present() = 0;
    virtual void wait() = 0;

    tph::render_pass& get_render_pass() noexcept
    {
        return m_render_pass;
    }

    const tph::render_pass& get_render_pass() const noexcept
    {
        return m_render_pass;
    }

protected:
    render_target(const render_target&) = delete;
    render_target& operator=(const render_target&) = delete;
    render_target(render_target&&) noexcept = default;
    render_target& operator=(render_target&&) noexcept = default;

private:
    tph::render_pass m_render_pass{};
};

using render_target_ptr = std::shared_ptr<render_target>;
using render_target_weak_ptr = std::weak_ptr<render_target>;

}

template<> struct cpt::enable_enum_operations<cpt::begin_render_options> {static constexpr bool value{true};};

#endif
