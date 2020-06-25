#ifndef CAPTAL_RENDER_TEXTURE_HPP_INCLUDED
#define CAPTAL_RENDER_TEXTURE_HPP_INCLUDED

#include "config.hpp"

#include <tephra/commands.hpp>

#include "texture.hpp"
#include "render_target.hpp"

namespace cpt
{

struct render_texture_info
{
    std::uint32_t width{};
    std::uint32_t height{};
    tph::texture_format format{tph::texture_format::r8g8b8a8_srgb};
    tph::sample_count sample_count{tph::sample_count::msaa_x1}; //if not msaa_x1 uses multisampling with specified depth
    tph::texture_format depth_format{tph::texture_format::undefined}; //ifdef uses depth buffering with specified format
};

class CAPTAL_API render_texture : public texture, public render_target
{
public:
    render_texture() = default;
    render_texture(const render_texture_info& info, const tph::render_pass_info& render_pass, std::span<const render_target_attachment> attachments);
    render_texture(const render_texture_info& info);
    render_texture(const render_texture_info& info, const tph::sampling_options& sampling);

    ~render_texture();
    render_texture(const render_texture&) = delete;
    render_texture& operator=(const render_texture&) = delete;
    render_texture(render_texture&&) = default;
    render_texture& operator=(render_texture&&) = default;

    std::pair<tph::command_buffer&, frame_presented_signal&> begin_render();
    void present();

private:
    struct frame_data
    {
        tph::command_pool pool{};
        tph::command_buffer buffer{};
        tph::fence fence{};
        frame_presented_signal signal{};
        bool begin{};
    };

private:
    frame_data& add_frame_data();
    void wait_all();

private:
    render_texture_info m_info{};
    tph::texture m_multisampling_texture{};
    tph::texture m_depth_texture{};
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
