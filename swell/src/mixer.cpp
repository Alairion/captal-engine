#include "mixer.hpp"

#include <cmath>
#include <numeric>
#include <algorithm>

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
template<typename T>
T sinc(T value)
{
    if(std::abs(value) >= std::numeric_limits<T>::epsilon())
        return std::sin(value * pi<T>) / (value * pi<T>);

    return static_cast<T>(1);
}

static std::vector<float> ressample(std::uint32_t input_frequency, std::vector<float> signal, std::uint32_t output_frequency)
{
    const std::uint32_t gcd{std::gcd(input_frequency, output_frequency)};
    const std::uint32_t up_factor{input_frequency / gcd};
    const std::uint32_t down_factor{output_frequency / gcd};

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
        return 0.0f;

    return std::sqrt(std::pow(10.0f, value * 3.0f) / 1e3f);
}

sound::sound(mixer& mixer, sound_reader& reader)
:m_data{mixer.make_sound()}
{
    m_data->reader = &reader;
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
    m_data->state.current_sample = 0;
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
    m_data->state.status = sound_status::paused;
}

void sound::resume()
{
    std::lock_guard lock{m_data->mutex};
    m_data->state.status = sound_status::playing;
}

void sound::fade_out(std::uint64_t samples)
{
    std::lock_guard lock{m_data->mutex};
    m_data->state.status = sound_status::fading_out;
    m_data->state.fading = samples;
}

void sound::fade_in(std::uint64_t samples)
{
    std::lock_guard lock{m_data->mutex};
    m_data->state.status = sound_status::fading_in;
    m_data->state.fading = samples;
}

void sound::set_volume(float volume)
{
    std::lock_guard lock{m_data->mutex};
    m_data->state.volume = get_volume_multiplier(volume);
}

void sound::set_loop_points(std::uint64_t begin_frame, std::uint64_t end_frame)
{
    std::lock_guard lock{m_data->mutex};
    m_data->state.loop_begin = begin_frame * m_data->reader->channels_count();
    m_data->state.loop_end = end_frame * m_data->reader->channels_count();
}

void sound::enable_spatialization()
{
    std::lock_guard lock{m_data->mutex};
    m_data->spatialization.enable = true;
}

void sound::disable_spatialization()
{
    std::lock_guard lock{m_data->mutex};
    m_data->spatialization.enable = false;
}

void sound::set_relative_spatialization()
{
    std::lock_guard lock{m_data->mutex};
    m_data->spatialization.relative = true;
}

void sound::set_absolute_spatialization()
{
    std::lock_guard lock{m_data->mutex};
    m_data->spatialization.relative = false;
}

void sound::set_minimum_distance(float distance)
{
    std::lock_guard lock{m_data->mutex};
    m_data->spatialization.minimum_distance = distance;
}

void sound::set_attenuation(float attenuation)
{
    std::lock_guard lock{m_data->mutex};
    m_data->spatialization.attenuation = attenuation;
}

void sound::move(float x, float y, float z)
{
    std::lock_guard lock{m_data->mutex};
    m_data->spatialization.position = impl::vec3{x, y, z};
}

sound_status sound::status() const
{
    std::lock_guard lock{m_data->mutex};
    return m_data->state.status;
}

template<typename T>
static float sgn(T value)
{
    return value >= T{} ? 1 : -1;
}

static inline float mix_amplitude(float value, std::size_t count) noexcept
{
    return (sgn(value) * (1.0f - std::pow(1.0f - std::abs(value), count)));
}

mixer::mixer(std::size_t frame_per_buffer, std::uint32_t channel_count, std::uint32_t frequency)
:m_buffer_size{frame_per_buffer}
,m_channel_count{channel_count}
,m_frequency{frequency}
,m_data_future{m_data_promise.get_future()}
,m_next_future{m_next_promise.get_future()}
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
        m_next_promise.set_value();
        m_process_thread.join();
    }
}

mixer::sample_buffer_type mixer::next()
{
    auto data = m_data_future.get();

    m_data_promise = std::promise<sample_buffer_type>{};
    m_data_future = m_data_promise.get_future();

    m_next_promise.set_value();

    return data;
}

void mixer::move_listener(float x, float y, float z)
{
    m_position = impl::vec3{x, y, z};
}

void mixer::set_up(float x, float y, float z)
{
    m_up = impl::vec3{x, y, z};
}

void mixer::set_listener_direction(float x, float y, float z)
{
    m_direction = impl::vec3{x, y, z};
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
        while(m_running.load(std::memory_order_acquire))
        {
            m_data_promise.set_value(mix_sounds(get_sounds_data()));

            m_next_future.get();
            m_next_promise = std::promise<void>{};
            m_next_future = m_next_promise.get_future();
        }
    }
    catch(const std::exception& e)
    {
        m_ok.store(false, std::memory_order_release);

        m_data_promise.set_value(sample_buffer_type{});

        m_next_future.get();
        m_next_promise = std::promise<void>{};
        m_next_future = m_next_promise.get_future();
    }
}

std::vector<mixer::sample_buffer_type> mixer::get_sounds_data()
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
                const auto sound_channels{sound.reader->channels_count()};

                sample_buffer_type sound_data{};

                sound_data = get_sound_data(sound, sound_channels);
                sound_data = spatialize(sound, std::move(sound_data), sound_channels);
                sound_data = apply_volume(sound, std::move(sound_data), sound_channels);

                sounds_data.push_back(std::move(sound_data));
            }
        }
        catch(const std::exception& e)
        {
            sound.state.status = sound_status::aborted;
        }
    }

    free_sounds();

    return sounds_data;
}

void mixer::free_sounds()
{
    const auto predicate = [](auto&& sound_ptr)
    {
        return sound_ptr->state.status == sound_status::freed;
    };

    m_sounds.erase(std::remove_if(std::begin(m_sounds), std::end(m_sounds), predicate), std::end(m_sounds));
}

mixer::sample_buffer_type mixer::mix_sounds(const std::vector<sample_buffer_type>& sounds_data)
{
    if(std::empty(sounds_data))
    {
        sample_buffer_type output{};
        output.resize(m_buffer_size * m_channel_count);

        return output;
    }

    if(std::size(sounds_data) == 1)
        return sounds_data[0];

    sample_buffer_type out_data{};
    out_data.resize(m_buffer_size * m_channel_count);

    for(std::size_t i{}; i < std::size(out_data); ++i)
    {
        float sum{};
        for(auto&& data : sounds_data)
            sum += data[i];

        const float average{sum / static_cast<float>(std::size(sounds_data))};
        out_data[i] = mix_amplitude(average, std::size(sounds_data));
    }

    return out_data;
}

mixer::sample_buffer_type mixer::get_sound_data(impl::sound_data& sound, std::uint32_t channels)
{
    sample_buffer_type sound_data{};
    sound_data.resize(m_buffer_size * channels);

    if((sound.state.current_sample + m_buffer_size) > sound.state.loop_end) //Loop
    {
        const std::uint32_t first_size = sound.state.loop_end - sound.state.current_sample;
        sound.reader->read(std::data(sound_data), first_size);

        sound.reader->seek(sound.state.loop_begin);
        sound.reader->read(std::data(sound_data) + first_size, m_buffer_size - first_size);

        sound.state.current_sample = sound.state.loop_begin + (m_buffer_size - first_size);
    }
    else
    {
        if(!sound.reader->read(std::data(sound_data), m_buffer_size))
            sound.state.status = sound_status::ended;

        sound.state.current_sample += m_buffer_size;
    }

    return sound_data;
}

mixer::sample_buffer_type mixer::apply_volume(impl::sound_data& sound, sample_buffer_type data, std::uint32_t channels)
{
    if(sound.state.fading != std::numeric_limits<std::uint64_t>::max())
    {
        for(std::uint32_t i{}; i < m_buffer_size; i += channels) //fading
        {
            const float fading_percent{1.0f - (static_cast<float>(sound.state.current_fading + i) / static_cast<float>(sound.state.fading))};
            if(fading_percent <= 0.0f)
            {
                sound.state.status = sound_status::ended;
                std::fill(std::data(data) + i, std::data(data) + m_buffer_size, 0.0f);
                break;
            }

            const float fading_multiplier{get_volume_multiplier(fading_percent)};
            for(std::uint32_t j{}; j < channels; ++j)
                data[i + j] *= sound.state.volume * fading_multiplier;
        }

        sound.state.current_fading += m_buffer_size;
    }
    else if(sound.state.volume != 1.0f)
    {
        for(auto& sample : data)
            sample *= sound.state.volume;
    }

    return data;
}

mixer::sample_buffer_type mixer::spatialize(impl::sound_data& sound, sample_buffer_type data, std::uint32_t channels)
{
    if(channels != m_channel_count)
    {
        if(channels == 1 && sound.spatialization.enable)
        {
            const glm::vec3 listener_position{m_position.x, m_position.y, m_position.z};
            const glm::vec3 sound_position{sound.spatialization.relative ? listener_position + glm::vec3{sound.spatialization.position.x, sound.spatialization.position.y, sound.spatialization.position.z} : glm::vec3{sound.spatialization.position.x, sound.spatialization.position.y, sound.spatialization.position.z}};

            const float distance{glm::distance(sound_position, listener_position)};
            const float minimum{sound.spatialization.minimum_distance};
            const float attenuation{sound.spatialization.attenuation};
            const float factor{minimum / (minimum + attenuation * (std::max(distance, minimum) - minimum))};

            for(auto& sample : data)
                sample *= factor;

            if(m_channel_count == 1) //Don't need to do anything more in mono
                return data;

            const glm::vec3 up{glm::normalize(glm::vec3{m_up.x, m_up.y, m_up.z})};
            const glm::vec3 listener_direction{glm::normalize(glm::vec3{m_direction.x, m_direction.y, m_direction.z})};
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

                for(std::size_t i{}; i < m_buffer_size; ++i)
                {
                    output[i * 2] = data[i] * ((-cosine) + 2.0f) / 4.0f; //right
                    output[(i * 2) + 1] = data[i] * (cosine + 2.0f) / 4.0f; //left
                }

                return output;
            }
        }

        const std::uint32_t maximum_channel{std::min(m_channel_count, channels)};

        sample_buffer_type output{};
        output.resize(m_buffer_size * m_channel_count);

        for(std::size_t i{}; i < m_buffer_size; ++i)
            for(std::size_t j{}; j < maximum_channel; ++j)
                output[(i * m_channel_count) + j] = data[(i * channels) + j];

        return output;
    }

    return data;
}

}
