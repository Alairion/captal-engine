#ifndef CAPTAL_RENDER_TARGET_HPP_INCLUDED
#define CAPTAL_RENDER_TARGET_HPP_INCLUDED

#include "config.hpp"

#include <concepts>

#include <tephra/render_target.hpp>

#include "signal.hpp"
#include "texture.hpp"

namespace cpt
{

class CAPTAL_API frame_resource_keeper
{
public:
    frame_resource_keeper() = default;
    ~frame_resource_keeper() = default;
    frame_resource_keeper(const frame_resource_keeper&) = delete;
    frame_resource_keeper& operator=(const frame_resource_keeper&) = delete;
    frame_resource_keeper(frame_resource_keeper&&) noexcept = default;
    frame_resource_keeper& operator=(frame_resource_keeper&&) noexcept = default;

    template<typename T>
    void keep(T&& resource)
    {
        m_resources.emplace_back(std::forward<T>(resource));
    }

    void reserve(std::size_t size)
    {
        m_resources.reserve(size);
    }

    void clear() noexcept
    {
        m_resources.clear();
    }

private:
    std::vector<asynchronous_resource_ptr> m_resources{};
};

using frame_time_t = std::chrono::duration<std::uint64_t, std::nano>;
using frame_presented_signal = cpt::signal<>;
using frame_time_signal = cpt::signal<frame_time_t>;

struct render_info
{

};

class CAPTAL_API render_target
{
public:
    render_target() = default;
    explicit render_target(const tph::render_pass_info& info);

    virtual ~render_target() = default;
    render_target(const render_target&) = delete;
    render_target& operator=(const render_target&) = delete;
    render_target(render_target&&) noexcept = default;
    render_target& operator=(render_target&&) noexcept = default;

    virtual frame_time_signal& register_frame_time() = 0;
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
