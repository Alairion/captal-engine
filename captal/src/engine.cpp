#include "engine.hpp"

#include "texture.hpp"

namespace cpt
{

static constexpr const char default_vertex_shader_spv[]
{
    #include "data/vertex.vert.spv.str"
};

static constexpr const char default_fragment_shader_spv[]
{
    #include "data/fragment.frag.spv.str"
};

static constexpr std::array<std::uint8_t, 16> dummy_texture_data{255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
static constexpr std::array<std::uint8_t, 16> dummy_normal_map_data{0, 0, 255, 255, 0, 0, 255, 255, 0, 0, 255, 255, 0, 0, 255, 255};
static constexpr std::array<std::uint8_t, 16> dummy_height_map_data{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static constexpr std::array<std::uint8_t, 16> dummy_specular_map_data{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

using clock = std::chrono::steady_clock;

engine* engine::m_instance{nullptr};

engine::engine(const std::string& application_name, tph::version version)
:m_application{application_name, version}
,m_audio_device{m_application.audio_application().default_device()}
,m_audio_mixer{m_audio_device.default_low_latency_buffer_size(), std::min(m_audio_device.max_output_channel(), 2u), m_audio_device.default_sample_rate()}
,m_audio_stream{m_application.audio_application(), m_audio_device, m_audio_mixer}
,m_graphics_device{m_application.graphics_application().default_physical_device()}
,m_renderer{m_application.graphics_application(), m_graphics_device}
{
    init();
}

static const swl::physical_device& default_audio_device(const swl::application& application, const audio_parameters& parameters)
{
    if(parameters.physical_device.has_value())
        return parameters.physical_device.value();

    const swl::physical_device& default_device{application.default_device()};

    if(default_device.max_output_channel() >= parameters.channel_count && default_device.default_sample_rate() == parameters.frequency)
        return default_device;

    for(const swl::physical_device& device : application.enumerate_physical_devices())
    {
        if(device.max_output_channel() >= parameters.channel_count && device.default_sample_rate() == parameters.frequency)
            return device;
    }

    if(default_device.max_output_channel() >= parameters.channel_count)
        return default_device;

    for(const swl::physical_device& device : application.enumerate_physical_devices())
    {
        if(device.max_output_channel() >= parameters.channel_count)
            return device;
    }

    throw std::runtime_error{"Can not find any suitable audio device."};
}

static const tph::physical_device& default_graphics_device(const tph::application& application, const graphics_parameters& parameters)
{
    if(parameters.physical_device.has_value())
        return parameters.physical_device.value();

    const auto requirements = [&parameters](const tph::physical_device& device) -> bool
    {
        if(parameters.features.wide_lines && !device.features().wide_lines)
            return false;

        if(parameters.features.large_points && ! device.features().large_points)
            return false;

        return true;
    };

    return application.select_physical_device(requirements);
}

engine::engine(const std::string& application_name, tph::version version, const audio_parameters& audio, const graphics_parameters& graphics)
:m_application{application_name, version}
,m_audio_device{default_audio_device(m_application.audio_application(), audio)}
,m_audio_mixer{m_audio_device.default_low_latency_buffer_size(), audio.channel_count, audio.frequency}
,m_audio_stream{m_application.audio_application(), m_audio_device, m_audio_mixer}
,m_graphics_device{default_graphics_device(m_application.graphics_application(), graphics)}
,m_renderer{m_application.graphics_application(), m_graphics_device, graphics.options, graphics.features}
{
    init();
}

engine::engine(cpt::application application, const audio_parameters& audio, const graphics_parameters& graphics)
:m_application{std::move(application)}
,m_audio_device{default_audio_device(m_application.audio_application(), audio)}
,m_audio_mixer{m_audio_device.default_low_latency_buffer_size(), audio.channel_count, audio.frequency}
,m_audio_stream{m_application.audio_application(), m_audio_device, m_audio_mixer}
,m_graphics_device{default_graphics_device(m_application.graphics_application(), graphics)}
,m_renderer{m_application.graphics_application(), m_graphics_device, graphics.options, graphics.features}
{
    init();
}

void engine::remove_window(render_window& window)
{
    const auto it{std::find_if(std::begin(m_windows), std::end(m_windows), [&window](const std::unique_ptr<render_window>& other)
    {
        return window.id() == other->id();
    })};

    if(it != std::end(m_windows))
        m_windows.erase(it);
}

std::pair<tph::command_buffer&, transfer_ended_signal&> engine::begin_transfer()
{
    if(!m_current_transfer_buffer)
    {
        m_transfer_buffers.push_back(transfer_buffer{m_frame_id, tph::cmd::begin(m_transfer_pool, tph::command_buffer_level::primary, tph::command_buffer_flags::one_time_submit), tph::fence{m_renderer}});
        m_current_transfer_buffer = &m_transfer_buffers.back();

        tph::cmd::pipeline_barrier(m_current_transfer_buffer->buffer, tph::pipeline_stage::color_attachment_output, tph::pipeline_stage::transfer);
    }

    return {m_current_transfer_buffer->buffer, m_current_transfer_buffer->signal};
}

void engine::flush_transfers()
{
    if(m_current_transfer_buffer)
    {
        tph::cmd::end(m_current_transfer_buffer->buffer);

        tph::submit_info info{};
        info.command_buffers.push_back(m_current_transfer_buffer->buffer);

        std::lock_guard lock{m_queue_mutex};
        tph::submit(m_renderer, info, m_current_transfer_buffer->fence);

        m_current_transfer_buffer = nullptr;
    }

    for(auto it{std::begin(m_transfer_buffers)}; it != std::end(m_transfer_buffers);)
    {
        if(it->fence.try_wait())
        {
            it->signal();
            it = m_transfer_buffers.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool engine::run()
{
    update_window();
    update_frame();

    const auto window_closed = [](auto&& window_ptr) -> bool
    {
        return window_ptr->is_closed();
    };

    if(std::all_of(std::begin(m_windows), std::end(m_windows), window_closed))
    {
        m_renderer.wait();
        return false;
    }

    m_update_signal(m_frame_time);

    return true;
}

void engine::init()
{
    m_instance = this;

    m_audio_mixer.set_up(0.0f, 0.0f, 1.0f);
    m_audio_mixer.set_listener_direction(0.0f, 1.0f, 0.0f);
    m_audio_stream.start();

    m_transfer_pool = tph::command_pool{m_renderer};

    m_default_vertex_shader = tph::shader{m_renderer, tph::shader_stage::vertex, std::string_view{default_vertex_shader_spv, std::size(default_vertex_shader_spv) - 1}, tph::load_from_memory};
    m_default_fragment_shader = tph::shader{m_renderer, tph::shader_stage::fragment, std::string_view{default_fragment_shader_spv, std::size(default_fragment_shader_spv) - 1}, tph::load_from_memory};

    m_dummy_texture = make_texture(2, 2, std::data(dummy_texture_data), tph::sampling_options{tph::filter::nearest, tph::filter::nearest, tph::address_mode::repeat});
    m_dummy_normal_map = make_texture(2, 2, std::data(dummy_normal_map_data), tph::sampling_options{tph::filter::nearest, tph::filter::nearest, tph::address_mode::repeat});
    m_dummy_height_map = make_texture(2, 2, std::data(dummy_height_map_data), tph::sampling_options{tph::filter::nearest, tph::filter::nearest, tph::address_mode::repeat});
    m_dummy_specular_map = make_texture(2, 2, std::data(dummy_specular_map_data), tph::sampling_options{tph::filter::nearest, tph::filter::nearest, tph::address_mode::repeat});
}

void engine::update_window()
{
    for(auto& window_ptr : m_windows)
        window_ptr->update();
}

void engine::update_frame()
{
    ++m_frame_id;
    ++m_frame_per_second_counter;

    m_frame_time = std::chrono::duration_cast<std::chrono::duration<float>>(clock::now() - m_last_update).count();
    m_last_update = clock::now();

    m_frame_per_second_timer += m_frame_time;
    if(m_frame_per_second_timer > 1.0f)
    {
        m_frame_per_second = m_frame_per_second_counter;

        m_frame_per_second_signal(m_frame_per_second);

        m_frame_per_second_counter = 0;
        m_frame_per_second_timer -= 1.0f;
    }

    if(m_frame_rate_limit != no_frame_rate_limit)
    {
        const float frame_time_target{1.0f / static_cast<float>(m_frame_rate_limit)};

        if(m_frame_time < frame_time_target)
        {
            std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<float>(frame_time_target - m_frame_time)));
        }
    }
}

void engine::set_framerate_limit(std::uint32_t frame_per_second)
{
    m_frame_rate_limit = frame_per_second;
}

}
