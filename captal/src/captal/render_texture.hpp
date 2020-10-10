#ifndef CAPTAL_RENDER_TEXTURE_HPP_INCLUDED
#define CAPTAL_RENDER_TEXTURE_HPP_INCLUDED

#include "config.hpp"

#include <tephra/commands.hpp>
#include <tephra/synchronization.hpp>
#include <tephra/query.hpp>

#include "texture.hpp"
#include "render_target.hpp"

namespace cpt
{

struct render_texture_info
{
    std::uint32_t width{};
    std::uint32_t height{};
    tph::texture_format format{tph::texture_format::r8g8b8a8_srgb};
    tph::texture_usage usage{};
};

struct current_target_t{};
inline constexpr current_target_t current_target{};

using render_texture_attachment = std::variant<texture_ptr, current_target_t>;

class CAPTAL_API render_texture : public texture, public render_target
{
public:
    render_texture() = default;
    explicit render_texture(const render_texture_info& info, const tph::render_pass_info& render_pass, std::vector<render_texture_attachment> attachments);
    explicit render_texture(const render_texture_info& info, const tph::sampling_options& sampling, const tph::render_pass_info& render_pass, std::vector<render_texture_attachment> attachments);
    explicit render_texture(const render_texture_info& info, tph::sample_count sample_count = tph::sample_count::msaa_x1, tph::texture_format depth_format = tph::texture_format::undefined);
    explicit render_texture(const render_texture_info& info, const tph::sampling_options& sampling, tph::sample_count sample_count = tph::sample_count::msaa_x1, tph::texture_format depth_format = tph::texture_format::undefined);

    ~render_texture();
    render_texture(const render_texture&) = delete;
    render_texture& operator=(const render_texture&) = delete;
    render_texture(render_texture&&) = delete;
    render_texture& operator=(render_texture&&) = delete;

    frame_time_signal& register_frame_time() override;
    std::pair<tph::command_buffer&, frame_presented_signal&> begin_render() override;
    void present() override;

    tph::framebuffer& framebuffer() noexcept
    {
        return m_framebuffer;
    }

    const tph::framebuffer& framebuffer() const noexcept
    {
        return m_framebuffer;
    }

    texture& attachement(std::size_t index) noexcept
    {
        return std::visit([this](auto&& attachement) -> texture&
        {
            if constexpr(std::is_same_v<std::decay_t<decltype(attachement)>, current_target_t>)
            {
                return *this;
            }
            else
            {
                return *attachement;
            }
        }, m_attachments[index]);
    }

    const texture& attachement(std::size_t index) const noexcept
    {
        return std::visit([this](auto&& attachement) -> const texture&
        {
            if constexpr(std::is_same_v<std::decay_t<decltype(attachement)>, current_target_t>)
            {
                return *this;
            }
            else
            {
                return *attachement;
            }
        }, m_attachments[index]);
    }

    std::span<const render_texture_attachment> attachements() const noexcept
    {
        return m_attachments;
    }

private:
    struct frame_data
    {
        tph::command_pool pool{};
        tph::command_buffer buffer{};
        tph::fence fence{};
        tph::query_pool query_pool{};
        frame_presented_signal signal{};
        frame_time_signal time_signal{};
        bool begin{}; //true if register_frame_time or begin_render has been called, false after present
        bool timed{}; //true if register_frame_time has been called, false after frame data reset
    };

private:
    frame_data& next_frame();
    frame_data& add_frame_data();
    void reset(frame_data& data);
    void time_results(frame_data& data);
    void wait_all();

private:
    std::vector<render_texture_attachment> m_attachments{};
    tph::framebuffer m_framebuffer{};
    std::vector<frame_data> m_frames_data{};
};

using render_texture_ptr = std::shared_ptr<render_texture>;
using render_texture_weak_ptr = std::weak_ptr<render_texture>;

template<typename... Args>
render_texture_ptr make_render_texture(Args&&... args)
{
    return std::make_shared<render_texture>(std::forward<Args>(args)...);
}

}

#endif
