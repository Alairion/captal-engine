//MIT License
//
//Copyright (c) 2021 Alexy Pellegrini
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "audio_world.hpp"

#include <cassert>
#include <numbers>
#include <utility>

namespace swl
{

/*
static constexpr std::array volumes_lut  //std::sqrt(std::pow(10.0f, value * 3.0f) / 1000.0f) with value = [0.0f, 1.0f]
{
    0.0000000f, 0.0327341f, 0.0338844f, 0.0350752f, 0.0363078f, 0.0375837f, 0.0389045f, 0.0402717f,
    0.0416869f, 0.0431519f, 0.0446684f, 0.0462381f, 0.0478630f, 0.0495450f, 0.0512861f, 0.0530884f,
    0.0549541f, 0.0568853f, 0.0588844f, 0.0609537f, 0.0630957f, 0.0653131f, 0.0676083f, 0.0699842f,
    0.0724436f, 0.0749894f, 0.0776247f, 0.0803526f, 0.0831764f, 0.0860994f, 0.0891251f, 0.0922571f,
    0.0954993f, 0.0988553f, 0.1023293f, 0.1059254f, 0.1096478f, 0.1135011f, 0.1174897f, 0.1216186f,
    0.1258925f, 0.1303166f, 0.1348962f, 0.1396368f, 0.1445439f, 0.1496235f, 0.1548816f, 0.1603245f,
    0.1659586f, 0.1717907f, 0.1778278f, 0.1840771f, 0.1905460f, 0.1972422f, 0.2041737f, 0.2113488f,
    0.2187760f, 0.2264643f, 0.2344227f, 0.2426608f, 0.2511884f, 0.2600157f, 0.2691532f, 0.2786118f,
    0.2884029f, 0.2985380f, 0.3090292f, 0.3198892f, 0.3311307f, 0.3427674f, 0.3548129f, 0.3672819f,
    0.3801889f, 0.3935496f, 0.4073797f, 0.4216959f, 0.4365152f, 0.4518553f, 0.4677345f, 0.4841716f,
    0.5011864f, 0.5187992f, 0.5370309f, 0.5559034f, 0.5754390f, 0.5956612f, 0.6165940f, 0.6382624f,
    0.6606923f, 0.6839104f, 0.7079445f, 0.7328232f, 0.7585761f, 0.7852341f, 0.8128289f, 0.8413934f,
    0.8709618f, 0.9015692f, 0.9332523f, 0.9660488f, 1.0000000f
};
*/

static float get_volume_multiplier(float value) noexcept
{
    if(value == 0.0f)
    {
        return 0.0f;
    }

    return std::sqrt(std::pow(10.0f, value * 3.0f) / 1000.0f);
}

listener::listener(std::uint32_t channel_count)
:m_data{std::make_unique<impl::listener_data>()}
{
    m_data->state.channel_count = channel_count;
}

void listener::set_volume(float volume)
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.volume = get_volume_multiplier(volume);
}

void listener::enable_spatialization()
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.spatialization.enable = true;
}

void listener::disable_spatialization()
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.spatialization.enable = false;
}

void listener::move(const vec3f& relative)
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.spatialization.position += relative;
}

void listener::move_to(const vec3f& position)
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.spatialization.position = position;
}

void listener::set_direction(const vec3f& direction)
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.spatialization.direction = direction;
}

float listener::volume() const
{
    std::lock_guard lock{m_data->mutex};

    return m_data->state.volume;
}

std::uint32_t listener::channel_count() const
{
    std::lock_guard lock{m_data->mutex};

    return m_data->state.channel_count;
}

bool listener::is_spatialization_enabled() const
{
    std::lock_guard lock{m_data->mutex};

    return m_data->state.spatialization.enable;
}

vec3f listener::position() const
{
    std::lock_guard lock{m_data->mutex};

    return m_data->state.spatialization.position;
}

vec3f listener::direction() const
{
    std::lock_guard lock{m_data->mutex};

    return m_data->state.spatialization.direction;
}

sound::sound(audio_world& world, std::unique_ptr<sound_reader> reader)
:m_data{world.make_sound()}
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

    assert((m_data->state.status == sound_status::stopped || m_data->state.status == sound_status::ended || m_data->state.status == sound_status::aborted) && "swl::sound::start() can only be called on stopped, ended or arboted sound.");

    m_data->reader->seek(0);
    m_data->state.status = sound_status::playing;
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

    m_data->state.pause_initial_status = m_data->state.status;
    m_data->state.status = sound_status::paused;
}

void sound::resume()
{
    std::lock_guard lock{m_data->mutex};

    assert(m_data->state.status == sound_status::paused && "swl::sound::resume() can only be called on paused sound.");

    m_data->state.status = m_data->state.pause_initial_status;
}

void sound::fade_in(std::uint64_t frames)
{
    std::lock_guard lock{m_data->mutex};

    assert((m_data->state.status == sound_status::stopped || m_data->state.status == sound_status::ended || m_data->state.status == sound_status::aborted || m_data->state.status == sound_status::paused) && "swl::sound::fade_in() can only be called on stopped, ended, paused or aborted sound.");

    if(m_data->state.status == sound_status::stopped || m_data->state.status == sound_status::ended || m_data->state.status == sound_status::aborted)
    {
        //Can not call start() because the mutex is already locked
        m_data->reader->seek(0);
        m_data->state.current_fading = 0;
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

    assert(m_data->reader->info().seekable && "looped sound's reader must be seekable.");
    assert(m_data->reader->info().frame_count >= end_frame && "looped sound's end frame outside reader bounds.");

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

void sound::move(const vec3f& relative)
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.spatialization.position += relative;
}

void sound::move_to(const vec3f& position)
{
    std::lock_guard lock{m_data->mutex};

    m_data->state.spatialization.position = position;
}

void sound::seek(std::uint64_t frame)
{
    std::lock_guard lock{m_data->mutex};

    m_data->reader->seek(frame);
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

vec3f sound::position() const
{
    std::lock_guard lock{m_data->mutex};

    return m_data->state.spatialization.position;
}

std::uint64_t sound::tell() const
{
    std::lock_guard lock{m_data->mutex};

    return m_data->reader->tell();
}

static constexpr float fast_pow(float value, std::size_t count) noexcept
{
    if(value == 0.0f)
    {
        return 0.0f;
    }

    if(value == 1.0f)
    {
        return 1.0f;
    }

    if(count == 0)
    {
        return 1.0f;
    }

    float output{1.0f};

    if(count % 2 == 1)
    {
        output *= value;
    }

    for(std::size_t i{count / 2}; i != 0; i /= 2)
    {
        value *= value;

        if(i % 2 == 1)
        {
            output *= value;
        }
    }

    return output;
}

static constexpr float sign(float value) noexcept
{
    return value >= 0.0f ? 1.0f : -1.0f;
}

inline float mix_amplitude(float value, std::size_t count) noexcept
{
    return sign(value) * (1.0f - fast_pow(1.0f - std::abs(value), count));
}

audio_world::audio_world(std::uint32_t sample_rate)
:m_sample_rate{sample_rate}
{

}

void audio_world::set_up(const vec3f& direction)
{
    m_up = cpt::normalize(direction);
}

void audio_world::discard(std::size_t frame_count)
{
    std::unique_lock lock{m_mutex};

    discard_impl(frame_count);
    free_resources();
}

void audio_world::generate(std::size_t frame_count)
{
    std::unique_lock lock{m_mutex};

    if(std::empty(m_listeners_data))
    {
        discard_impl(frame_count);

        return;
    }

    m_sample_buffer.clear();
    store_sounds_data(frame_count);

    lock.unlock();

    for(auto& sound : m_sounds_data)
    {
        apply_fading(sound, frame_count);
    }

    for(auto& listener : m_listeners_data)
    {
        std::lock_guard lock{*listener.queue};

        m_listener_samples = listener.queue->begin(frame_count * listener.state.channel_count);

        for(auto& sound : m_sounds_data)
        {
            if(sound.state.channel_count == 1 && listener.state.spatialization.enable && sound.state.spatialization.enable)
            {
                spatialize(listener, sound, frame_count);
            }
            else if(sound.state.channel_count != listener.state.channel_count)
            {
                adjust_channels(listener, sound, frame_count);
            }
            else //no spacialization and sound.state.channel_count == listener.state.channel_count
            {
                const float volume{sound.state.volume * listener.state.volume};

                for(std::size_t i{}; i < std::size(sound.samples); ++i)
                {
                    m_listener_samples[i] += sound.samples[i] * volume;
                }
            }
        }

        mix_sounds();

        listener.queue->end();
    }

    lock.lock();

    free_resources();
}

vec3f audio_world::up() const
{
    std::lock_guard lock{m_mutex};

    return m_up;
}

impl::sound_data* audio_world::make_sound()
{
    std::lock_guard lock{m_mutex};

    m_sounds.push_back(std::make_unique<impl::sound_data>());

    return m_sounds.back().get();
}

void audio_world::discard_impl(std::size_t frame_count)
{
    m_sample_buffer.reserve(4096);
    m_sample_buffer.resize(m_sample_buffer.capacity());

    for(auto& sound : m_sounds)
    {
        std::lock_guard sound_lock{sound->mutex};

        try
        {
            if(sound->state.status == sound_status::playing || sound->state.status == sound_status::fading_in || sound->state.status == sound_status::fading_out)
            {
                sound->state.channel_count = sound->reader->info().channel_count;

                discard_sound_data(*sound, frame_count);
            }
        }
        catch(...)
        {
            sound->state.status = sound_status::aborted;
        }
    }
}

void audio_world::discard_sound_data(impl::sound_data& sound, std::size_t frame_count)
{
    const auto position{sound.reader->tell()};

    if(position + frame_count > sound.state.loop_end) //Loop
    {
        const auto loop_first{sound.state.loop_end - position};
        const auto loop_size {sound.state.loop_end - sound.state.loop_begin};

        sound.reader->seek(sound.state.loop_begin + ((frame_count - loop_first) % loop_size));
    }
    else
    {
        if(sound.reader->info().seekable)
        {
            if(sound.reader->tell() + frame_count > sound.reader->info().frame_count)
            {
                sound.reader->seek(sound.reader->info().frame_count);
                sound.state.status = sound_status::ended;
            }
            else
            {
                sound.reader->seek(sound.reader->tell() + frame_count);
            }
        }
        else
        {
            const auto buffer_size{std::size(m_sample_buffer) / sound.state.channel_count};

            std::size_t read{};
            while(read < frame_count)
            {
                const auto count{std::min(buffer_size, frame_count - read)};

                if(!sound.reader->read(std::data(m_sample_buffer), count))
                {
                    sound.state.status = sound_status::ended;
                    return;
                }

                read += count;
            }
        }

        if(sound.state.fading != std::numeric_limits<std::uint64_t>::max() && sound.state.current_fading + frame_count > sound.state.fading)
        {
            sound.state.status = sound_status::ended;
        }
    }
}

void audio_world::store_sounds_data(std::size_t frame_count)
{
    m_sounds_data.clear();
    m_sounds_data.reserve(std::size(m_sounds));

    for(auto& sound : m_sounds)
    {
        std::lock_guard sound_lock{sound->mutex};

        try
        {
            if(sound->state.status == sound_status::playing || sound->state.status == sound_status::fading_in || sound->state.status == sound_status::fading_out)
            {
                sound->state.channel_count = sound->reader->info().channel_count;

                m_sounds_data.emplace_back(get_sound_data(*sound, frame_count), sound->state);
            }
        }
        catch(...)
        {
            sound->state.status = sound_status::aborted;
        }
    }
}

std::span<float> audio_world::get_sound_data(impl::sound_data& sound, std::size_t frame_count)
{
    const auto begin{std::size(m_sample_buffer)};
    const auto count{frame_count * sound.state.channel_count};

    m_sample_buffer.resize(begin + count);

    const auto output  {std::data(m_sample_buffer) + begin};
    const auto position{sound.reader->tell()};

    if((position + frame_count) > sound.state.loop_end) //Loop
    {
        const auto max  {sound.state.loop_end - sound.state.loop_begin};
        const auto first{sound.state.loop_end - position};

        sound.reader->read(output, first);

        auto read{first};

        if(frame_count - first > max) //Loops multiple time
        {
            sound.reader->seek(sound.state.loop_begin);
            sound.reader->read(output + first, max);

            read += max;

            const auto source{output + first};
            while(frame_count - read > max)
            {
                std::copy_n(source, max, output + read);
                read += max;
            }
        }

        sound.reader->seek(sound.state.loop_begin);
        sound.reader->read(output + read, frame_count - read);
    }
    else
    {
        if(!sound.reader->read(output, frame_count))
        {
            sound.state.status = sound_status::ended;
        }
    }

    if(sound.state.fading != std::numeric_limits<std::uint64_t>::max() && sound.state.current_fading + frame_count > sound.state.fading)
    {
        sound.state.status = sound_status::ended;
    }

    return std::span<float>{output, frame_count * sound.state.channel_count};
}

void audio_world::apply_fading(sound_data_buffer& sound, std::size_t frame_count)
{
    if(sound.state.fading != std::numeric_limits<std::uint64_t>::max())
    {
        for(std::uint32_t i{}; i < frame_count; ++i) //fading
        {
            const float fading_percent{1.0f - (static_cast<float>(sound.state.current_fading + i) / static_cast<float>(sound.state.fading))};
            if(fading_percent <= 0.0f)
            {
                sound.state.current_fading += i;

                std::fill(std::begin(sound.samples) + i * sound.state.channel_count, std::end(sound.samples), 0.0f);
                break;
            }

            const float multiplier{get_volume_multiplier(fading_percent)};
            for(std::uint32_t j{}; j < sound.state.channel_count; ++j)
            {
                sound.samples[i * sound.state.channel_count + j] *= multiplier;
            }
        }

        sound.state.current_fading += frame_count;
    }
}

void audio_world::spatialize(listener_data_buffer& listener, sound_data_buffer& sound, std::size_t frame_count) noexcept
{
    const vec3f listener_position  {listener.state.spatialization.position};
    const vec3f sound_base_position{sound.state.spatialization.position};
    const vec3f sound_position     {sound.state.spatialization.relative ? listener_position + sound_base_position : sound_base_position};

    const float distance   {cpt::distance(sound_position, listener_position)};
    const float minimum    {sound.state.spatialization.minimum_distance};
    const float attenuation{sound.state.spatialization.attenuation};
    const float volume     {sound.state.volume * listener.state.volume};
    const float factor     {(minimum / (minimum + attenuation * (std::max(distance, minimum) - minimum))) * volume};

    if(listener.state.channel_count == 1)
    {
        for(std::size_t i{}; i < frame_count; ++i)
        {
            m_listener_samples[i] += sound.samples[i] * factor;
        }

        return;
    }

    const vec3f listener_direction{cpt::normalize(listener.state.spatialization.direction)};
    const vec3f sound_direction   {sound_position == listener_position ? -listener_direction : cpt::normalize(sound_position - listener_position)};

    const vec3f cross{cpt::cross(sound_direction, listener_direction)};
    const float deter{cpt::dot(m_up, cross)};
    const float dot  {cpt::dot(sound_direction, listener_direction)};
    const float angle{std::atan2(deter, dot)};
    const float sine {std::sin(angle)}; //1 is left, -1 is right
    //const float cosine{std::sin(angle + (std::numbers::pi_v<float> / 2.0f))}; //1 is front, -1 is back

    if(listener.state.channel_count == 2)
    {
        for(std::size_t i{}; i < frame_count; ++i)
        {
            m_listener_samples[i * 2]     += sound.samples[i] * factor * ((-sine) + 2.0f) / 4.0f; //right
            m_listener_samples[i * 2 + 1] += sound.samples[i] * factor * (sine + 2.0f) / 4.0f; //left
        }
    }
    else
    {
        assert(false && "swl::audio_world can only spacialize in stereo, yet.");
    }
}

void audio_world::adjust_channels(listener_data_buffer& listener, sound_data_buffer& sound, std::size_t frame_count) noexcept
{
    const float volume{sound.state.volume * listener.state.volume};

    if(listener.state.channel_count == 2 && sound.state.channel_count == 1) //Mono -> Stereo
    {
        for(std::size_t i{}; i < frame_count; ++i)
        {
            const float sample{sound.samples[i] * volume};

            m_listener_samples[i * 2]     += sample; //right
            m_listener_samples[i * 2 + 1] += sample; //left
        }
    }
    else if(listener.state.channel_count == 1 && sound.state.channel_count == 2)
    {
        for(std::size_t i{}; i < frame_count; ++i)
        {
            const float sample{(sound.samples[i * 2] + sound.samples[i * 2 + 1]) * volume};

            m_listener_samples[i] += mix_amplitude(sample, 2);
        }
    }
}

void audio_world::mix_sounds()
{
    for(auto& sample : m_listener_samples)
    {
        sample = mix_amplitude(sample, std::size(m_sounds_data));
    }
}

void audio_world::free_resources()
{
    m_listeners_data.clear();

    const auto sound_predicate = [](auto&& sound)
    {
        return sound->state.status == sound_status::freed;
    };

    m_sounds.erase(std::remove_if(std::begin(m_sounds), std::end(m_sounds), sound_predicate), std::end(m_sounds));
}

}
