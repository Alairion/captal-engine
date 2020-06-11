#include "mixer.hpp"

#include <cmath>
#include <numeric>
#include <algorithm>

#ifndef NDEBUG
    #include <iostream>
#endif

#define GLM_FORCE_RADIANS
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

namespace swl
{

template<typename T>
constexpr T pi{static_cast<T>(3.1415926535897932385)};

namespace ressample
{
/*
constepxr float lerp(float first, float second, float t) noexcept
{
  return first + t * (second - first);
}

template<typename T>
T sinc(T value)
{
    if(std::abs(value) >= std::numeric_limits<T>::epsilon())
        return std::sin(value * pi<T>) / (value * pi<T>);

    return static_cast<T>(1);
}

static std::vector<float> ressample(std::uint32_t input_sample_rate, std::vector<float> signal, std::uint32_t output_sample_rate)
{
    const std::uint32_t gcd{std::gcd(input_sample_rate, output_sample_rate)};
    const std::uint32_t up_factor{input_sample_rate / gcd};
    const std::uint32_t down_factor{output_sample_rate / gcd};

    if(up_factor == down_factor)
        return signal;

    const std::size_t input_size{std::size(signal)};
    const std::size_t output_size{static_cast<std::size_t>(std::ceil((input_size * up_factor) / down_factor))};

    const std::uint32_t max_factor{std::max(up_factor, down_factor)};


    return {};
}*/

}

static float get_volume_multiplier(float value) noexcept
{
    if(value == 0.0f)
    {
        return 0.0f;
    }

    return std::sqrt(std::pow(10.0f, value * 3.0f) / 1e3f);
}

sound::sound(mixer& mixer, std::unique_ptr<sound_reader> reader)
:m_data{mixer.make_sound()}
{
    m_data->reader = std::move(reader);
}

sound::~sound()
{
    if(m_data)
    {
        std::lock_guard lock{m_data->mutex};

        m_data->state.status = sound_status::freed;
    }
}

sound::sound(sound&& other) noexcept
:m_data{std::exchange(other.m_data, nullptr)}
{

}

sound& sound::operator=(sound&& other) noexcept
{
    m_data = std::exchange(other.m_data, m_data);

    return *this;
}

void sound::start()
{
    std::lock_guard lock{m_data->mutex};

    m_data->reader->seek(0);
    m_data->state.status = sound_status::playing;
    m_data->state.current_frame = 0;
    m_data->state.current_fading = 0;
    m_data->state.fading = std::numeric_limits<std::uint64_t>::max();
}

void sound::stop()
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.status = sound_status::stopped;
}

void sound::pause()
{
    std::lock_guard lock{m_data->mutex};

    assert((m_data->state.status == sound_status::playing || m_data->state.status == sound_status::fading_in || m_data->state.status == sound_status::fading_out) && "swl::sound::pause() can only be called on playing or fading sound.");

    m_data->state.status = sound_status::paused;
}

void sound::resume()
{
    std::lock_guard lock{m_data->mutex};

    assert(m_data->state.status == sound_status::paused && "swl::sound::resume() can only be called on paused sound.");

    m_data->state.status = sound_status::playing;
}

void sound::fade_in(std::uint64_t frames)
{
    std::unique_lock lock{m_data->mutex};

    assert((m_data->state.status == sound_status::stopped || m_data->state.status == sound_status::paused || m_data->state.status == sound_status::aborted) && "swl::sound::fade_in() can only be called on stopped, paused or aborted sound.");

    if(m_data->state.status == sound_status::stopped || m_data->state.status == sound_status::aborted)
    {
        //Can not call start() because the mutex is already locked
        m_data->reader->seek(0);
        m_data->state.current_frame = 0;
        m_data->state.current_fading = 0;
        m_data->state.fading = std::numeric_limits<std::uint64_t>::max();
    }

    m_data->state.status = sound_status::fading_in;
    m_data->state.fading = frames;
}

void sound::fade_out(std::uint64_t frames)
{
    std::lock_guard lock{m_data->mutex};

    assert(m_data->state.status == sound_status::playing && "swl::sound::fade_out() can only be called on playing sound.");

    m_data->state.status = sound_status::fading_out;
    m_data->state.fading = frames;
}

void sound::set_volume(float volume)
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.volume = get_volume_multiplier(volume);
}

void sound::set_loop_points(std::uint64_t begin_frame, std::uint64_t end_frame)
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.loop_begin = begin_frame;
    m_data->state.loop_end = end_frame;
}

void sound::enable_spatialization()
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.spatialization.enable = true;
}

void sound::disable_spatialization()
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.spatialization.enable = false;
}

void sound::relative_spatialization()
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.spatialization.relative = true;
}

void sound::absolute_spatialization()
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.spatialization.relative = false;
}

void sound::set_minimum_distance(float distance)
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.spatialization.minimum_distance = distance;
}

void sound::set_attenuation(float attenuation)
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.spatialization.attenuation = attenuation;
}

void sound::move(const glm::vec3& relative)
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.spatialization.position += relative;
}

void sound::move_to(const glm::vec3& position)
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.spatialization.position = position;
}

void sound::seek(std::uint64_t frame)
{
    std::lock_guard lock{m_data->mutex};

    m_data->reader->seek(frame);
    m_data->state.current_frame = frame;
}

std::unique_ptr<sound_reader> sound::change_reader(std::unique_ptr<sound_reader> new_reader)
{
    std::lock_guard lock{m_data->mutex};

    std::unique_ptr<sound_reader> output{std::move(m_data->reader)};

    m_data->reader = std::move(new_reader);
    m_data->state.status = sound_status::stopped;

    return output;
}

sound_status sound::status() const
{
    std::lock_guard lock{m_data->mutex};

    return m_data->state.status;
}

float sound::volume() const
{
    std::lock_guard lock{m_data->mutex};

    return m_data->state.volume;
}

std::pair<std::uint64_t, std::uint64_t> sound::loop_points() const
{
    std::lock_guard lock{m_data->mutex};

    return std::make_pair(m_data->state.loop_begin, m_data->state.loop_end);
}

bool sound::is_spatialization_enabled() const
{
    std::lock_guard lock{m_data->mutex};

    return m_data->state.spatialization.enable;
}

bool sound::is_spatialization_relative() const
{
    std::lock_guard lock{m_data->mutex};

    return m_data->state.spatialization.relative;
}

float sound::minimum_distance() const
{
    std::lock_guard lock{m_data->mutex};

    return m_data->state.spatialization.minimum_distance;
}

float sound::attenuation() const
{
    std::lock_guard lock{m_data->mutex};

    return m_data->state.spatialization.attenuation;
}

glm::vec3 sound::position() const
{
    std::lock_guard lock{m_data->mutex};

    return m_data->state.spatialization.position;
}

std::uint64_t sound::tell() const
{
    std::lock_guard lock{m_data->mutex};

    return m_data->state.current_frame;
}

sound_reader& sound::reader() const
{
    std::lock_guard lock{m_data->mutex};

    return *m_data->reader;
}

template<typename T>
static float sgn(T value)
{
    return value >= T{} ? 1.0f : -1.0f;
}

static float mix_amplitude(float value, std::size_t count) noexcept
{
    return sgn(value) * (1.0f - std::pow(1.0f - std::abs(value), static_cast<float>(count)));
}

mixer::mixer(std::uint32_t sample_rate, std::uint32_t channel_count, time_type minimum_latency, time_type discard_threshold)
:m_sample_rate{sample_rate}
,m_channel_count{channel_count}
,m_minimum_latency{minimum_latency}
,m_discard_threshold{discard_threshold}
,m_running{true}
,m_ok{true}
,m_process_thread{&mixer::process, this}
{

}

mixer::~mixer()
{
    if(m_process_thread.joinable())
    {
        m_running.store(false, std::memory_order_release);
        m_process_thread.join();
    }
}

void mixer::move_listener(const glm::vec3& relative)
{
    std::unique_lock lock{m_mutex};

    m_position += relative;
}

void mixer::move_listener_to(const glm::vec3& position)
{
    std::unique_lock lock{m_mutex};

    m_position = position;
}

void mixer::set_up(const glm::vec3& direction)
{
    std::unique_lock lock{m_mutex};

    m_up = direction;
}

void mixer::set_listener_direction(const glm::vec3& direction)
{
    std::unique_lock lock{m_mutex};

    m_direction = direction;
}

impl::sound_data* mixer::make_sound()
{
    std::lock_guard lock{m_mutex};

    m_sounds.push_back(std::make_unique<impl::sound_data>());

    return m_sounds.back().get();
}

void mixer::process() noexcept
{
    try
    {
        clock::time_point last{clock::now()};

        while(m_running.load(std::memory_order_acquire))
        {
            const clock::time_point now{clock::now()};
            time_type elapsed{std::chrono::duration_cast<time_type>(now - last)};

            #ifndef NDEBUG
            time_type discaded{};
            #endif

            while(elapsed > m_discard_threshold)
            {
                elapsed -= m_discard_threshold;

                #ifndef NDEBUG
                discaded += m_discard_threshold;
                #endif
            }

            #ifndef NDEBUG
            if(discaded > time_type::zero())
            {
                std::cerr << discaded.count() << "s of audio processing discarded." << std::endl;
            }
            #endif

            if(elapsed >= m_minimum_latency)
            {
                last = now;

                const std::size_t frame_count{static_cast<std::size_t>(elapsed.count() * static_cast<double>(m_sample_rate))};
                const sample_buffer_type data{mix_sounds(get_sounds_data(frame_count), frame_count)};

                m_queue.push(std::begin(data), std::end(data));
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds{1});
            }
        }
    }
    catch(...)
    {
        m_ok.store(false, std::memory_order_release);
    }
}


std::vector<sample_buffer_type> mixer::get_sounds_data(std::size_t frame_count)
{
    std::unique_lock lock{m_mutex};

    std::vector<sample_buffer_type> sounds_data{};

    for(auto& sound_ptr : m_sounds)
    {
        impl::sound_data& sound{*sound_ptr};
        std::unique_lock lock{sound.mutex};

        try
        {
            if(sound.state.status == sound_status::playing || sound.state.status == sound_status::fading_in || sound.state.status == sound_status::fading_out)
            {
                const std::uint32_t sound_channels{sound.reader->channel_count()};

                sample_buffer_type sound_data{};

                sound_data = get_sound_data(sound, frame_count, sound_channels);
                sound_data = spatialize(sound, std::move(sound_data), frame_count, sound_channels);
                sound_data = apply_volume(sound, std::move(sound_data), frame_count, sound_channels);

                sounds_data.push_back(std::move(sound_data));
            }
        }
        catch(...)
        {
            sound.state.status = sound_status::aborted;
        }
    }

    free_sounds();

    return sounds_data;
}

sample_buffer_type mixer::get_sound_data(impl::sound_data& sound, std::size_t frame_count, std::uint32_t channels)
{
    sample_buffer_type output{};
    output.resize(frame_count * channels);

    if((sound.state.current_frame + frame_count) > sound.state.loop_end) //Loop
    {
        const std::uint64_t first_size = sound.state.loop_end - sound.state.current_frame;
        sound.reader->read(std::data(output), first_size);

        sound.reader->seek(sound.state.loop_begin);
        sound.reader->read(std::data(output) + first_size, frame_count - first_size);

        sound.state.current_frame = sound.state.loop_begin + (frame_count - first_size);
    }
    else
    {
        if(!sound.reader->read(std::data(output), frame_count))
        {
            sound.state.status = sound_status::ended;
        }

        sound.state.current_frame += frame_count;
    }

    return output;
}

sample_buffer_type mixer::apply_volume(impl::sound_data& sound, sample_buffer_type data, std::size_t frame_count, std::uint32_t channels)
{
    if(sound.state.fading != std::numeric_limits<std::uint64_t>::max())
    {
        for(std::uint32_t i{}; i < frame_count; ++i) //fading
        {
            const float fading_percent{1.0f - (static_cast<float>(sound.state.current_fading + i) / static_cast<float>(sound.state.fading))};
            if(fading_percent <= 0.0f)
            {
                sound.state.status = sound_status::ended;
                std::fill(std::begin(data) + i * channels, std::end(data), 0.0f);
                break;
            }

            const float fading_multiplier{get_volume_multiplier(fading_percent)};
            const float multiplier{sound.state.volume * fading_multiplier};

            for(std::uint32_t j{}; j < channels; ++j)
            {
                data[i * channels + j] *= multiplier;
            }
        }

        sound.state.current_fading += frame_count;
    }
    else if(sound.state.volume != 1.0f)
    {
        for(auto& sample : data)
        {
            sample *= sound.state.volume;
        }
    }

    return data;
}

sample_buffer_type mixer::spatialize(impl::sound_data& sound, sample_buffer_type data, std::size_t frame_count, std::uint32_t channels)
{
    if(channels != m_channel_count)
    {
        if(channels == 1 && sound.state.spatialization.enable)
        {
            const glm::vec3 listener_position{m_position};
            const glm::vec3 sound_base_position{sound.state.spatialization.position};
            const glm::vec3 sound_position{sound.state.spatialization.relative ? listener_position + sound_base_position : sound_base_position};

            const float distance{glm::distance(sound_position, listener_position)};
            const float minimum{sound.state.spatialization.minimum_distance};
            const float attenuation{sound.state.spatialization.attenuation};
            const float factor{minimum / (minimum + attenuation * (std::max(distance, minimum) - minimum))};

            for(auto& sample : data)
            {
                sample *= factor;
            }

            if(m_channel_count == 1) //Don't need to do anything more in mono
            {
                return data;
            }

            const glm::vec3 up{glm::normalize(m_up)};
            const glm::vec3 listener_direction{glm::normalize(m_direction)};
            const glm::vec3 sound_direction{sound_position == listener_position ? -listener_direction : glm::normalize(sound_position - listener_position)};

            const glm::vec3 cross{glm::cross(sound_direction, listener_direction)};
            const float determinant{glm::dot(up, cross)};
            const float dot{glm::dot(sound_direction, listener_direction)};
            const float angle{std::atan2(determinant, dot)};
            const float cosine{std::cos(angle - (pi<float> / 2.0f))};

            if(m_channel_count == 2)
            {
                sample_buffer_type output{};
                output.resize(std::size(data) * 2);

                for(std::size_t i{}; i < frame_count; ++i)
                {
                    output[i * 2] = data[i] * ((-cosine) + 2.0f) / 4.0f; //right
                    output[(i * 2) + 1] = data[i] * (cosine + 2.0f) / 4.0f; //left
                }

                return output;
            }
        }

        const std::uint32_t maximum_channel{std::min(m_channel_count, channels)};

        sample_buffer_type output{};
        output.resize(frame_count * m_channel_count);

        for(std::size_t i{}; i < frame_count; ++i)
        {
            for(std::size_t j{}; j < maximum_channel; ++j)
            {
                output[(i * m_channel_count) + j] = data[(i * channels) + j];
            }
        }

        return output;
    }

    return data;
}

sample_buffer_type mixer::mix_sounds(const std::vector<sample_buffer_type>& sounds_data, std::size_t frame_count)
{
    if(std::empty(sounds_data))
    {
        sample_buffer_type output{};
        output.resize(frame_count * m_channel_count);

        return output;
    }

    if(std::size(sounds_data) == 1)
    {
        return sounds_data[0];
    }

    sample_buffer_type out_data{};
    out_data.resize(frame_count * m_channel_count);

    for(std::size_t i{}; i < std::size(out_data); ++i)
    {
        float sum{};
        for(auto&& data : sounds_data)
        {
            sum += data[i];
        }

        const float average{sum / static_cast<float>(std::size(sounds_data))};

        out_data[i] = mix_amplitude(average, std::size(sounds_data));
    }

    return out_data;
}

void mixer::free_sounds()
{
    const auto predicate = [](auto&& sound_ptr)
    {
        return sound_ptr->state.status == sound_status::freed;
    };

    m_sounds.erase(std::remove_if(std::begin(m_sounds), std::end(m_sounds), predicate), std::end(m_sounds));
}

}
