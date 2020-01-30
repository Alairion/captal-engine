#ifndef CAPTAL_RENDER_TEXTURE_HPP_INCLUDED
#define CAPTAL_RENDER_TEXTURE_HPP_INCLUDED

#include "config.hpp"

#include "texture.hpp"
#include "render_target.hpp"

namespace cpt
{

class render_texture : public texture, public render_target
{
public:
    render_texture() = default;
    render_texture(std::uint32_t width, std::uint32_t height, tph::render_target_options target_options = tph::render_target_options::clipping, tph::sample_count sample_count = tph::sample_count::msaa_x1);
    render_texture(std::uint32_t width, std::uint32_t height, const tph::sampling_options& sampling, tph::render_target_options target_options = tph::render_target_options::clipping, tph::sample_count sample_count = tph::sample_count::msaa_x1);
    ~render_texture() = default;
    render_texture(const render_texture&) = delete;
    render_texture& operator=(const render_texture&) = delete;
    render_texture(render_texture&&) = default;
    render_texture& operator=(render_texture&&) = default;

    std::pair<tph::command_buffer&, frame_presented_signal&> begin_render();
    void present();

    tph::render_target_options options() const noexcept
    {
        return m_options;
    }

    tph::sample_count sample_count() const noexcept
    {
        return m_sample_count;
    }

private:
    struct frame_data
    {
        tph::command_pool pool{};
        tph::command_buffer buffer{};
        tph::semaphore image_available{};
        tph::semaphore image_presentable{};
        tph::fence fence{};
        frame_presented_signal signal{};
        bool begin{};
    };

private:
    frame_data& add_frame_data();

private:
    tph::render_target_options m_options{};
    tph::sample_count m_sample_count{};
    std::vector<frame_data> m_frames_data{};
};

}

#endif
