#ifndef CAPTAL_SOUND_HPP_INCLUDED
#define CAPTAL_SOUND_HPP_INCLUDED

#include "config.hpp"

#include <swell/mixer.hpp>
#include <swell/sound_file.hpp>

namespace cpt
{

class CAPTAL_API sound : swl::sound_file_reader, swl::sound
{
public:
    sound();
    sound(const std::filesystem::path& file, swl::sound_reader_options options = swl::sound_reader_options::none);
    sound(const std::string_view& data, swl::sound_reader_options options = swl::sound_reader_options::none);
    sound(std::istream& stream, swl::sound_reader_options options = swl::sound_reader_options::none);
    sound(swl::sound_file_reader reader);

    ~sound() = default;
    sound(const sound&) = delete;
    sound& operator=(const sound&) = delete;
    sound(sound&&) noexcept = default;
    sound& operator=(sound&&) noexcept = default;

    using swl::sound::start;
    using swl::sound::stop;
    using swl::sound::pause;
    using swl::sound::resume;

    using swl::sound::fade_out;
    using swl::sound::fade_in;

    using swl::sound::set_volume;
    using swl::sound::set_loop_points;

    using swl::sound::enable_spatialization;
    using swl::sound::disable_spatialization;
    using swl::sound::set_relative_spatialization;
    using swl::sound::set_absolute_spatialization;
    using swl::sound::set_minimum_distance;
    using swl::sound::set_attenuation;
    using swl::sound::move;

    using swl::sound::status;

    using swl::sound_reader::seek;

    template<typename Rep, typename Period>
    void fade_in(std::chrono::duration<Rep, Period> time)
    {
        fade_in(static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::duration<double>>(time).count() * frequency()));
    }

    template<typename Rep, typename Period>
    void fade_out(std::chrono::duration<Rep, Period> time)
    {
        fade_out(static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::duration<double>>(time).count() * frequency()));
    }

    template<typename Rep1, typename Period1, typename Rep2, typename Period2>
    void set_loop_points(std::chrono::duration<Rep1, Period1> begin, std::chrono::duration<Rep2, Period2> end)
    {
        set_loop_points(static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::duration<double>>(begin).count() * frequency()),
                        static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::duration<double>>(end).count() * frequency()));
    }

    template<typename Rep, typename Period>
    void seek(std::chrono::duration<Rep, Period> time)
    {
        seek(static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::duration<double>>(time).count() * frequency()));
    }
};

using sound_ptr = std::shared_ptr<sound>;
using sound_weak_ptr = std::weak_ptr<sound>;

template<typename... Args>
sound_ptr make_sound(Args&&... args)
{
    return std::make_shared<sound>(std::forward<Args>(args)...);
}

}

#endif
