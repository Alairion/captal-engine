#include <swell/application.hpp>
#include <swell/audio_pulser.hpp>
#include <swell/wave.hpp>
#include <swell/ogg.hpp>
#include <swell/stream.hpp>
#include <swell/sound_file.hpp>

#include <cmath>

int main()
{
    const std::filesystem::path path{u8"input.ogg"};

    swl::application application{};
    const swl::physical_device& device{application.default_output_device()};

    swl::audio_world world{44100};
    world.set_up(swl::vec3f{0.0f, 0.0f, 1.0f});

    auto reader{swl::open_file(path)};
    std::vector<float> buffer{};
    buffer.resize(reader->info().frame_count * reader->info().channel_count);
    reader->read(std::data(buffer), reader->info().frame_count);
    std::ofstream ofs{"test.raw", std::ios_base::binary};
    ofs.write(reinterpret_cast<const char*>(std::data(buffer)), std::size(buffer) * sizeof(float));

    swl::sound sound{world, swl::open_file(path)};
    //sound.enable_spatialization();
    sound.move_to(swl::vec3f{-1.0f, 0.0f, 0.0f});

    swl::audio_pulser pulser{world};
    swl::listener_bind listener{pulser.bind(swl::listener{2})};
    listener->set_direction(swl::vec3f{0.0f, 1.0f, 0.0f});

    const swl::stream_info info
    {
        .format        = swl::sample_format::float32,
        .channel_count = 2,
        .sample_rate   = 44100,
        .latency       = device.default_low_output_latency()
    };

    swl::stream stream{application, device, info, swl::listener_bridge{*listener}};

    pulser.start();
    sound.start();
    stream.start();

    std::this_thread::sleep_for(std::chrono::milliseconds{1000});

    sound.change_reader(swl::open_file(path, swl::sound_reader_options::buffered));
    sound.start();

    std::this_thread::sleep_for(std::chrono::milliseconds{1000});

    sound.change_reader(swl::open_file(path, swl::sound_reader_options::decoded));
    sound.start();

    std::this_thread::sleep_for(std::chrono::milliseconds{1000});
}
