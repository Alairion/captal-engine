#ifndef CAPTAL_RENDER_WINDOW_HPP_INCLUDED
#define CAPTAL_RENDER_WINDOW_HPP_INCLUDED

#include "config.hpp"

#include <apyre/window.hpp>
#include <apyre/event.hpp>

#include <tephra/surface.hpp>
#include <tephra/swapchain.hpp>
#include <tephra/render_target.hpp>
#include <tephra/commands.hpp>
#include <tephra/query.hpp>

#include "render_target.hpp"
#include "window.hpp"
#include "color.hpp"

namespace cpt
{

struct video_mode
{
    std::uint32_t image_count{2};
    tph::texture_usage usage{tph::texture_usage::color_attachment};
    tph::surface_composite composite{tph::surface_composite::opaque};
    tph::present_mode present_mode{tph::present_mode::fifo};
    bool clipping{true};
    tph::texture_format surface_format{tph::texture_format::undefined};
    tph::sample_count sample_count{tph::sample_count::msaa_x1};
    tph::texture_format depth_format{tph::texture_format::undefined};
};

enum class render_window_status : std::uint32_t
{
    ok = 0,
    unrenderable = 1,
    surface_lost = 2
};

class CAPTAL_API render_window final : public render_target
{
public:
    render_window() = default;
    explicit render_window(window_ptr window, video_mode mode);

    ~render_window();
    render_window(const render_window&) = delete;
    render_window& operator=(const render_window&) = delete;
    render_window(render_window&&) = default;
    render_window& operator=(render_window&&) = default;

    std::optional<frame_render_info> begin_render(begin_render_options options) override;
    void present() override;
    void wait() override;

    void set_clear_color(const color& color) noexcept
    {
        m_clear_color = tph::clear_color_float_value{color.red, color.green, color.blue, color.alpha};
    }

    void set_clear_color(const tph::clear_color_value& color) noexcept
    {
        m_clear_color = color;
    }

    void set_clear_depth_stencil(float depth, std::uint32_t stencil) noexcept
    {
        m_clear_depth_stencil = tph::clear_depth_stencil_value{depth, stencil};
    }

    const window_ptr& window() const noexcept
    {
        return m_window;
    }

    render_window_status status() const noexcept
    {
        return m_status;
    }

    optional_ref<const tph::swapchain_info> info() const noexcept
    {
        if(m_swapchain)
            return m_swapchain->info();

        return nullref;
    }

#ifdef CAPTAL_DEBUG
    void set_name(std::string_view name);
#else
    void set_name(std::string_view name [[maybe_unused]]) const noexcept
    {

    }
#endif

private:
    struct frame_data
    {
        tph::command_buffer buffer{};
        tph::semaphore image_available{};
        tph::semaphore image_presentable{};
        tph::fence fence{};
        tph::query_pool query_pool{};
        asynchronous_resource_keeper keeper{};
        frame_presented_signal signal{};
        frame_time_signal time_signal{};
        std::uint32_t epoch{};
        bool begin{}; //true if register_frame_time or begin_render has been called, false after present
        bool timed{}; //true if register_frame_time has been called, false after frame data reset
        bool submitted{}; //true after present, false after frame data reset
    };

private:
    void setup_frame_data();
    void setup_framebuffers();
    void update_clear_values(tph::framebuffer& framebuffer);

    bool check_renderability();
    void flush_frame_data(frame_data& data);
    void reset_frame_data(frame_data& data);
    void time_results(frame_data& data);
    bool acquire(frame_data& data);
    bool recreate();

private:
    window_ptr m_window{};
    video_mode m_mode{};
    std::optional<tph::swapchain> m_swapchain{};
    tph::texture m_msaa_texture{};
    tph::texture m_depth_texture{};
    tph::clear_color_value m_clear_color{};
    tph::clear_depth_stencil_value m_clear_depth_stencil{};
    std::uint32_t m_epoch{1};
    std::uint32_t m_frame_index{};
    render_window_status m_status{};
    bool m_fake_frame{};

    tph::command_pool m_pool{};
    std::vector<frame_data> m_frames_data{};
    std::vector<tph::framebuffer> m_framebuffers{};

#ifdef CAPTAL_DEBUG
    std::string m_name{};
#endif
};

using render_window_ptr = std::shared_ptr<render_window>;
using render_window_weak_ptr = std::weak_ptr<render_window>;

template<typename... Args>
render_window_ptr make_render_window(Args&&... args)
{
    return std::make_shared<render_window>(std::forward<Args>(args)...);
}

}

#endif
