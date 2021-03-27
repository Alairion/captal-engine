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

class CAPTAL_API render_texture final : public texture, public render_target
{
public:
    render_texture() = default;
    explicit render_texture(const render_texture_info& info, const tph::render_pass_info& render_pass, std::vector<render_target_attachment> attachments);
    explicit render_texture(const render_texture_info& info, const tph::sampling_options& sampling, const tph::render_pass_info& render_pass, std::vector<render_target_attachment> attachments);
    explicit render_texture(const render_texture_info& info, tph::sample_count sample_count = tph::sample_count::msaa_x1, tph::texture_format depth_format = tph::texture_format::undefined);
    explicit render_texture(const render_texture_info& info, const tph::sampling_options& sampling, tph::sample_count sample_count = tph::sample_count::msaa_x1, tph::texture_format depth_format = tph::texture_format::undefined);

    ~render_texture();
    render_texture(const render_texture&) = delete;
    render_texture& operator=(const render_texture&) = delete;
    render_texture(render_texture&&) = delete;
    render_texture& operator=(render_texture&&) = delete;

    std::optional<frame_render_info> begin_render(begin_render_options options) override;
    void present() override;
    void wait() override;

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

    std::span<const render_target_attachment> attachements() const noexcept
    {
        return m_attachments;
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
        tph::fence fence{};
        tph::query_pool query_pool{};
        asynchronous_resource_keeper keeper{};
        frame_presented_signal signal{};
        frame_time_signal time_signal{};
        std::uint32_t epoch{};
        bool timed{}; //true if register_frame_time has been called, false after frame data reset
        bool submitted{}; //true after present, false after frame data reset
    };

private:
    frame_data& next_frame();
    frame_data& add_frame_data();
    void reset_frame_data(frame_data& data);
    void time_results(frame_data& data);

private:
    std::uint32_t m_epoch{};
    std::vector<render_target_attachment> m_attachments{};
    tph::framebuffer m_framebuffer{};
    tph::command_pool m_pool{};
    std::vector<frame_data> m_frames_data{};
    frame_data* m_current{};

#ifdef CAPTAL_DEBUG
    std::string m_name{};
#endif
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
