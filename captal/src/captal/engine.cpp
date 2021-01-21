#include "engine.hpp"

#include <iostream>

#include <apyre/power.hpp>

namespace cpt
{

#ifdef CAPTAL_DEBUG
static constexpr auto graphics_layers{tph::renderer_layer::validation};
static constexpr auto graphics_extensions{tph::renderer_extension::swapchain};
#else
static constexpr auto graphics_layers{tph::renderer_layer::none};
static constexpr auto graphics_extensions{tph::renderer_extension::swapchain};
#endif

static constexpr auto default_vertex_shader_spv = std::to_array<std::uint32_t>(
{
    #include "data/default.vert.spv.str"
});

static constexpr auto default_fragment_shader_spv = std::to_array<std::uint32_t>(
{
    #include "data/default.frag.spv.str"
});

static constexpr std::array<std::uint8_t, 4> default_texture_data{255, 255, 255, 255};

using clock = std::chrono::steady_clock;

engine* engine::m_instance{nullptr};

engine::engine(const std::string& application_name, cpt::version version)
:m_application{application_name, version}
,m_audio_device{m_application.audio_application().default_physical_device()}
,m_audio_mixer{m_audio_device.default_sample_rate(), std::min(m_audio_device.max_output_channel(), 2u)}
,m_audio_stream{m_application.audio_application(), m_audio_device, m_audio_mixer}
,m_graphics_device{m_application.graphics_application().default_physical_device()}
,m_renderer{m_graphics_device, graphics_layers, graphics_extensions}
,m_transfer_scheduler{m_renderer}
,m_uniform_pool{tph::buffer_usage::uniform | tph::buffer_usage::vertex | tph::buffer_usage::index}
{
    init();
}

static const swl::physical_device& default_audio_device(const swl::application& application, const audio_parameters& parameters)
{
    if(parameters.physical_device.has_value())
    {
        return parameters.physical_device.value();
    }

    const swl::physical_device& default_device{application.default_physical_device()};

    if(default_device.max_output_channel() >= parameters.channel_count && default_device.default_sample_rate() == parameters.frequency)
    {
        return default_device;
    }

    for(const swl::physical_device& device : application.enumerate_physical_devices())
    {
        if(device.max_output_channel() >= parameters.channel_count && device.default_sample_rate() == parameters.frequency)
        {
            return device;
        }
    }

    if(default_device.max_output_channel() >= parameters.channel_count)
    {
        return default_device;
    }

    for(const swl::physical_device& device : application.enumerate_physical_devices())
    {
        if(device.max_output_channel() >= parameters.channel_count)
        {
            return device;
        }
    }

    throw std::runtime_error{"Can not find any suitable audio device."};
}

static const tph::physical_device& default_graphics_device(const tph::application& application, const graphics_parameters& parameters)
{
    if(parameters.physical_device.has_value())
    {
        return parameters.physical_device.value();
    }

    const auto requirements = [&parameters](const tph::physical_device& device) -> bool
    {
        if(parameters.features.wide_lines && !device.features().wide_lines)
        {
            return false;
        }

        if(parameters.features.large_points && ! device.features().large_points)
        {
            return false;
        }

        return true;
    };

    return application.select_physical_device(requirements);
}

engine::engine(const std::string& application_name, cpt::version version, const system_parameters& system, const audio_parameters& audio, const graphics_parameters& graphics)
:engine{cpt::application{application_name, version}, system, audio, graphics}
{

}

engine::engine(cpt::application application, const system_parameters& system [[maybe_unused]], const audio_parameters& audio, const graphics_parameters& graphics)
:m_application{std::move(application)}
,m_audio_device{default_audio_device(m_application.audio_application(), audio)}
,m_audio_mixer{audio.frequency, audio.channel_count}
,m_audio_stream{m_application.audio_application(), m_audio_device, m_audio_mixer}
,m_graphics_device{default_graphics_device(m_application.graphics_application(), graphics)}
,m_renderer{m_graphics_device, graphics_layers | graphics.layers, graphics_extensions | graphics.extensions, graphics.features, graphics.options}
,m_transfer_scheduler{m_renderer}
,m_uniform_pool{tph::buffer_usage::uniform | tph::buffer_usage::vertex | tph::buffer_usage::index}
{
    init();
}

engine::~engine()
{
    m_renderer.wait();

    m_update_signal.disconnect_all();
    m_frame_per_second_signal.disconnect_all();

    m_instance = nullptr;
}

void engine::set_framerate_limit(std::uint32_t frame_per_second) noexcept
{
    m_frame_rate_limit = frame_per_second;
}

void engine::set_translator(cpt::translator new_translator)
{
    m_translator = std::move(new_translator);
}

void engine::set_default_texture(texture_ptr new_default_texture) noexcept
{
    m_default_texture = std::move(new_default_texture);

    #ifdef CAPTAL_DEBUG
    tph::set_object_name(m_renderer, m_default_texture->get_texture(), "cpt::engine's default texture");
    #endif
}

void engine::set_default_vertex_shader(tph::shader new_default_vertex_shader) noexcept
{
    m_default_vertex_shader = std::move(new_default_vertex_shader);

    #ifdef CAPTAL_DEBUG
    tph::set_object_name(m_renderer, m_default_vertex_shader, "cpt::engine's default vertex shader");
    #endif
}

void engine::set_default_fragment_shader(tph::shader new_default_fragment_shader) noexcept
{
    m_default_fragment_shader = std::move(new_default_fragment_shader);

    #ifdef CAPTAL_DEBUG
    tph::set_object_name(m_renderer, m_default_fragment_shader, "cpt::engine's default fragment shader");
    #endif
}

memory_transfer_info engine::begin_transfer()
{
    return m_transfer_scheduler.begin_transfer();
}

void engine::submit_transfers()
{
    m_uniform_pool.upload();
    m_transfer_scheduler.submit_transfers();
}

bool engine::run()
{
    update_frame();
    m_update_signal(m_frame_time);

    for(auto&& event : apr::event_iterator{m_application.system_application()})
    {
        if(std::holds_alternative<apr::quit_event>(event))
        {
            return false;
        }
    }

    return true;
}

void engine::init()
{
    assert(!m_instance && "Can not create a new engine if one already exists.");

    m_instance = this;

    m_audio_mixer.set_up(vec3f{0.0f, 0.0f, 1.0f});
    m_audio_mixer.set_listener_direction(vec3f{0.0f, 1.0f, 0.0f});
    m_audio_stream.start();

    set_default_vertex_shader(tph::shader{m_renderer, tph::shader_stage::vertex, default_vertex_shader_spv});
    set_default_fragment_shader(tph::shader{m_renderer, tph::shader_stage::fragment, default_fragment_shader_spv});
    set_default_texture(make_texture(1, 1, std::data(default_texture_data), tph::sampling_options{tph::filter::nearest, tph::filter::nearest, tph::address_mode::repeat}));

    if constexpr(debug_enabled)
    {
        m_uniform_pool.set_name("cpt::engine's uniform pool");

        //Display initialization info
        const auto format_power_state = [](apr::power_state state) -> std::string_view
        {
            switch(state)
            {
                case apr::power_state::on_battery: return "On battery";
                case apr::power_state::no_battery: return "No battery";
                case apr::power_state::charging: return "Charging";
                case apr::power_state::charged: return "Charged";
                default: return "Unknown";
            }
        };

        const auto format_uuid = [](const std::array<std::uint8_t, 16>& uuid)
        {
            std::stringstream ss{};
            ss << std::hex << std::setfill('0');

            for(std::size_t i{}; i < std::size(uuid); ++i)
            {
                if(i == 4 || i == 6 || i == 8 || i == 10)
                {
                    ss << '-';
                }

                ss << std::setw(2) << static_cast<std::uint32_t>(uuid[i]);
            }

            return ss.str();
        };

        const auto format_data = [](std::size_t amount)
        {
            std::stringstream ss{};
            ss << std::setprecision(2);

            if(amount < 1024)
            {
                ss << amount << " o";
            }
            else if(amount < 1024 * 1024)
            {
                ss << std::fixed << static_cast<double>(amount) / 1024.0 << " kio";
            }
            else
            {
                ss << std::fixed << static_cast<double>(amount) / (1024.0 * 1024.0) << " Mio";
            }

            return ss.str();
        };

        const auto format_driver = [](tph::driver_id driver) -> std::string_view
        {
            switch (driver)
            {
                case tph::driver_id::amd_proprietary: return "AMD Proprietary";
                case tph::driver_id::amd_open_source: return "AMD Open Source";
                case tph::driver_id::mesa_radv: return "Mesa RADV";
                case tph::driver_id::nvidia_proprietary: return "Nvidia Proprietary";
                case tph::driver_id::intel_proprietary_windows: return "Intel Proprietary";
                case tph::driver_id::intel_open_source_mesa: return "Intel Open Source Mesa";
                case tph::driver_id::imagination_proprietary: return "Imagination Proprietary";
                case tph::driver_id::qualcomm_proprietary: return "Qualcomm Proprietary";
                case tph::driver_id::arm_proprietary: return "ARM Proprietary";
                case tph::driver_id::google_swift_shader: return "Google SwiftShader";
                case tph::driver_id::ggp_proprietary: return "GGP Proprietary";
                case tph::driver_id::broadcom_proprietary: return "Broadcom Proprietary";
                case tph::driver_id::mesa_llvmpipe: return "Mesa LLVM Pipe";
                case tph::driver_id::moltenvk: return "MoltenVK";
                default: return "Unknown";
            }
        };

        const auto power_status{apr::get_power_status(m_application.system_application())};

        std::cout << "Captal engine initialized.\n";

        std::cout << "  System:\n";
        std::cout << "    Power status: " << format_power_state(power_status.state) << "\n";

        if(power_status.battery)
        {
            std::cout << "    Battery life: " << static_cast<std::uint32_t>(power_status.battery->remaining * 100.0) << "%\n";
        }

        std::cout << "  Audio device: " << m_audio_device.name() << "\n";
        std::cout << "    Channels: " << m_audio_mixer.channel_count() << "\n";
        std::cout << "    Sample rate: " << m_audio_mixer.sample_rate() << "Hz\n";
        std::cout << "    Output latency: " << m_audio_device.default_low_output_latency().count() << "s\n";

        std::cout << "  Graphics device: " << m_graphics_device.properties().name << "\n";
        std::cout << "    Pipeline Cache UUID: " << format_uuid(m_graphics_device.properties().uuid) << "\n";
        std::cout << "    Heap sizes:\n";
        std::cout << "      Host shared: " << format_data(m_renderer.allocator().default_heap_sizes().host_shared) << "\n";
        std::cout << "      Device shared: " << format_data(m_renderer.allocator().default_heap_sizes().device_shared) << "\n";
        std::cout << "      Device local: " << format_data(m_renderer.allocator().default_heap_sizes().device_local) << "\n";

        if(m_graphics_device.driver())
        {
            std::cout << "    Driver: \n";
            std::cout << "      ID: " << format_driver(m_graphics_device.driver()->id) << "\n";
            std::cout << "      Name: " << m_graphics_device.driver()->name << "\n";
            std::cout << "      Info: " << m_graphics_device.driver()->info << "\n";
        }
        else
        {
            std::cout << "    Driver: Can not be determined\n";
        }

        std::cout << std::flush;
    }
}

void engine::update_frame()
{
    ++m_frame_id;
    ++m_frame_per_second_counter;

    m_frame_time = std::chrono::duration_cast<std::chrono::duration<float>>(clock::now() - m_last_update).count();
    m_last_update = clock::now();

    m_frame_per_second_timer += m_frame_time;

    while(m_frame_per_second_timer > 2.0f)
    {
        m_frame_per_second_timer -= 1.0f;
    }

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

}
