#ifndef CAPTAL_ENGINE_HPP_INCLUDED
#define CAPTAL_ENGINE_HPP_INCLUDED

#include "config.hpp"

#include <optional>
#include <memory>

#include <swell/stream.hpp>
#include <swell/mixer.hpp>

#include <tephra/renderer.hpp>
#include <tephra/commands.hpp>
#include <tephra/shader.hpp>

#include <entt/entity/registry.hpp>

#include <sigslots/signal.hpp>

#include "application.hpp"
#include "render_window.hpp"
#include "texture.hpp"

namespace cpt
{

struct audio_parameters
{
    std::uint32_t channel_count{};
    std::uint32_t frequency{};
    std::optional<std::reference_wrapper<const swl::physical_device>> physical_device{};
};

struct graphics_parameters
{
    tph::renderer_options options{};
    tph::physical_device_features features{};
    std::optional<std::reference_wrapper<const tph::physical_device>> physical_device{};
};

using update_signal = sigslot::signal<float>;
using transfer_ended_signal = sigslot::signal<>;
using frame_per_second_signal = sigslot::signal<std::uint32_t>;

class CAPTAL_API engine
{
    static engine* m_instance;

public:
    static constexpr std::uint32_t no_frame_rate_limit{std::numeric_limits<std::uint32_t>::max()};

public:
    engine(const std::string& application_name, tph::version version);
    engine(const std::string& application_name, tph::version version, const audio_parameters& audio, const graphics_parameters& graphics);
    engine(cpt::application application, const audio_parameters& audio, const graphics_parameters& graphics);
    ~engine();
    engine(const engine&) = delete;
    engine& operator=(const engine&) = delete;
    engine(engine&& other) noexcept = delete;
    engine& operator=(engine&& other) noexcept = delete;

    template<typename... Args>
    render_window_ptr make_window(Args&&... args)
    {
        return m_windows.emplace_back(std::make_shared<render_window>(std::forward<Args>(args)...));
    }

    void remove_window(render_window_ptr window);

    std::pair<tph::command_buffer&, transfer_ended_signal&> begin_transfer();
    void flush_transfers();

    bool run();

    void set_framerate_limit(std::uint32_t frame_per_second);

    static engine& instance() noexcept
    {
        assert(m_instance && "cpt::engine::instance called before engine creation.");

        return *m_instance;
    }

    static const engine& cinstance() noexcept
    {
        assert(m_instance && "cpt::engine::cinstance called before engine creation.");

        return *m_instance;
    }

    cpt::application& application() noexcept
    {
        return m_application;
    }

    const cpt::application& application() const noexcept
    {
        return m_application;
    }

    const swl::physical_device& audio_device() const noexcept
    {
        return m_audio_device;
    }

    swl::mixer& audio_mixer() noexcept
    {
        return m_audio_mixer;
    }

    const swl::mixer& audio_mixer() const noexcept
    {
        return m_audio_mixer;
    }

    swl::stream& audio_stream() noexcept
    {
        return m_audio_stream;
    }

    const swl::stream& audio_stream() const noexcept
    {
        return m_audio_stream;
    }

    const tph::physical_device& graphics_device() const noexcept
    {
        return m_graphics_device;
    }

    tph::renderer& renderer() noexcept
    {
        return m_renderer;
    }

    const tph::renderer& renderer() const noexcept
    {
        return m_renderer;
    }

    update_signal& on_update() noexcept
    {
        return m_update_signal;
    }

    std::mutex& submit_mutex() noexcept
    {
        return m_queue_mutex;
    }

    tph::shader& default_vertex_shader() noexcept
    {
        return m_default_vertex_shader;
    }

    tph::shader& default_fragment_shader() noexcept
    {
        return m_default_fragment_shader;
    }

    texture& dummy_texture() noexcept
    {
        return m_dummy_texture;
    }

    float frame_time() const noexcept
    {
        return m_frame_time;
    }

    std::uint32_t frame_per_second() const noexcept
    {
        return m_frame_per_second;
    }

    std::uint64_t frame() const noexcept
    {
        return m_frame_id;
    }

    frame_per_second_signal& frame_per_second_update_signal() noexcept
    {
        return m_frame_per_second_signal;
    }

private:
    void init();
    void update_window();
    void update_frame();

private:
    struct transfer_buffer
    {
        std::uint64_t frame_id{};
        tph::command_buffer buffer{};
        tph::fence fence{};
        transfer_ended_signal signal{};
    };

private:
    cpt::application m_application;
    const swl::physical_device& m_audio_device;
    swl::mixer m_audio_mixer;
    swl::stream m_audio_stream;
    const tph::physical_device& m_graphics_device;
    tph::renderer m_renderer;

    std::mutex m_queue_mutex{};
    tph::command_pool m_transfer_pool{};
    std::vector<transfer_buffer> m_transfer_buffers{};
    transfer_buffer* m_current_transfer_buffer{};

    tph::shader m_default_vertex_shader{};
    tph::shader m_default_fragment_shader{};
    texture m_dummy_texture{};

    std::vector<std::shared_ptr<render_window>> m_windows{};

    std::chrono::steady_clock::time_point m_last_update{std::chrono::steady_clock::now()};
    float m_frame_time{};
    std::uint32_t m_frame_rate_limit{no_frame_rate_limit};
    float m_frame_per_second_timer{};
    std::uint32_t m_frame_per_second_counter{};
    std::uint32_t m_frame_per_second{};
    std::uint64_t m_frame_id{};
    frame_per_second_signal m_frame_per_second_signal{};

    update_signal m_update_signal{};
};

}

#endif
