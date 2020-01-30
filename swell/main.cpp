#include "src/application.hpp"
#include "src/mixer.hpp"
#include "src/wave.hpp"
#include "src/ogg.hpp"
#include "src/stream.hpp"
#include "src/sound_file.hpp"

#include <cmath>

int main()
{
    swl::application application{};
    const swl::physical_device& device{application.default_device()};

    swl::mixer mixer{device.default_low_latency_buffer_size(44100), 2, 44100};
    //Swap y and z axis
    mixer.set_up(0.0f, 0.0f, 1.0f);
    mixer.set_listener_direction(0.0f, 1.0f, 0.0f);

    swl::sound_file_reader reader{"car_alarm.wav", swl::load_from_file};
    swl::sound sound{mixer, reader};
    sound.enable_spatialization();
    sound.move(-1.0f, 1.0f, 0.0f);
    sound.set_volume(0.75f);
    sound.set_loop_points(3352, 43316);

    swl::stream stream{application, device, mixer};
    stream.start();

    for(std::size_t i{}; i < 10; ++i)
    {
        sound.start();
        std::this_thread::sleep_for(std::chrono::milliseconds{500});
    }
}
