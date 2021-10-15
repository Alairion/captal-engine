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

#ifndef SWELL_AUDIO_WORLD_HPP_INCLUDED
#define SWELL_AUDIO_WORLD_HPP_INCLUDED

#include "config.hpp"

#include <captal_foundation/math.hpp>

#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <span>

#include "sound_reader.hpp"
#include "stream.hpp"

namespace swl
{

class listener;
class sound;
class audio_world;

class audio_queue
{
    friend class audio_world;

public:
    audio_queue(std::size_t reserved = 0)
    {
        m_data.reserve(reserved);
    }

    ~audio_queue() = default;
    audio_queue(const audio_queue&) = delete;
    audio_queue& operator=(const audio_queue&) = delete;
    audio_queue(audio_queue&&) noexcept = delete;
    audio_queue& operator=(audio_queue&&) noexcept = delete;

    std::span<float> begin(std::size_t size)
    {
        const auto begin{std::size(m_data)};
        m_data.resize(begin + size);

        return std::span{std::data(m_data) + begin, size};
    }

    void end()
    {
        m_buffered.store(std::size(m_data), std::memory_order_release);

        m_condition.notify_one();
    }

    void lock()
    {
        m_mutex.lock();
    }

    void unlock()
    {
        m_mutex.unlock();
    }

    template<typename OutputIt>
    void drain(OutputIt output, std::size_t count)
    {
        std::unique_lock lock{m_mutex};

        if(std::size(m_data) < count)
        {
            m_condition.wait(lock, [this, count]
            {
                return buffered() >= count;
            });
        }

        m_buffered -= count;

        std::copy_n(std::begin(m_data), count, output);
        m_data.erase(std::begin(m_data), std::begin(m_data) + count);
    }

    template<typename OutputIt>
    std::size_t drain_n(OutputIt output, std::size_t count)
    {
        std::unique_lock lock{m_mutex};

        count = std::min(buffered(), count);
        m_buffered -= count;

        std::copy_n(std::begin(m_data), count, output);
        m_data.erase(std::begin(m_data), std::begin(m_data) + count);

        return count;
    }

    void discard(std::size_t count)
    {
        std::unique_lock lock{m_mutex};

        m_data.erase(std::begin(m_data), std::begin(m_data) + count);
        m_buffered.store(std::size(m_data), std::memory_order_release);
    }

    void discard()
    {
        std::unique_lock lock{m_mutex};

        m_data.clear();
        m_buffered.store(0, std::memory_order_release);
    }

    std::size_t buffered()
    {
        return m_buffered.load(std::memory_order_acquire);
    }

private:
    std::mutex m_mutex{};
    std::condition_variable m_condition{};
    std::atomic<std::size_t> m_buffered{};
    std::vector<float> m_data{};
};

namespace impl
{

struct listener_spatialization
{
    bool enable{true};
    vec3f position{};
    vec3f direction{0.0f, 0.0f, 1.0f};
};

struct listener_state
{
    float volume{1.0f};
    std::uint32_t channel_count{1};
    listener_spatialization spatialization{};
};

struct listener_data
{
    listener_state state{};
    audio_queue queue{};
    std::mutex mutex{};
};

}

class SWELL_API listener
{
    friend class audio_world;

public:
    listener() = default;
    explicit listener(std::uint32_t channel_count);

    ~listener() = default;
    listener(const listener&) = delete;
    listener& operator=(const listener&) = delete;
    listener(listener&& other) noexcept = default;
    listener& operator=(listener&& other) noexcept = default;

    void set_volume(float volume);
    void enable_spatialization();
    void disable_spatialization();
    void move(const vec3f& relative);
    void move_to(const vec3f& position);
    void set_direction(const vec3f& direction);

    float volume() const;
    std::uint32_t channel_count() const;
    bool is_spatialization_enabled() const;
    vec3f position() const;
    vec3f direction() const;

    template<typename OutputIt>
    void drain(OutputIt output, std::size_t count)
    {
        m_data->queue.drain(output, count * m_data->state.channel_count);
    }

    template<typename OutputIt>
    std::size_t drain_n(OutputIt output, std::size_t count)
    {
        return m_data->queue.drain_n(output, count * m_data->state.channel_count);
    }

    std::size_t buffered() const noexcept
    {
        return m_data->queue.buffered();
    }

    audio_queue& queue() noexcept
    {
        return m_data->queue;
    }

    const audio_queue& queue() const noexcept
    {
        return m_data->queue;
    }

private:
    std::unique_ptr<impl::listener_data> m_data{};
};

enum class sound_status : std::uint32_t
{
    stopped = 0,
    playing = 1,
    fading_in = 2,
    fading_out = 3,
    paused = 4,
    ended = 5,
    aborted = 6,
    freed = 7,
};

namespace impl
{

struct sound_spatialization
{
    bool enable{};
    bool relative{};
    float minimum_distance{1.0f};
    float attenuation{1.0f};
    vec3f position{};
};

struct sound_state
{
    sound_status status{sound_status::stopped};
    sound_status pause_initial_status{sound_status::playing};
    float volume{1.0};
    std::uint32_t channel_count{};
    std::uint64_t loop_begin{};
    std::uint64_t loop_end{std::numeric_limits<std::uint64_t>::max()};
    std::uint64_t fading{std::numeric_limits<std::uint64_t>::max()};
    std::uint64_t current_fading{};
    sound_spatialization spatialization{};
};

struct sound_data
{
    std::unique_ptr<sound_reader> reader{};
    sound_state state{};
    std::mutex mutex{};
};

}

class SWELL_API sound
{
public:
    sound() = default;
    sound(audio_world& world, std::unique_ptr<sound_reader> reader);

    ~sound();
    sound(const sound&) = delete;
    sound& operator=(const sound&) = delete;
    sound(sound&& other) noexcept;
    sound& operator=(sound&& other) noexcept;

    void start();
    void stop();
    void pause();
    void resume();
    void fade_in(std::uint64_t frames);
    void fade_out(std::uint64_t frames);

    template<typename Rep, typename Period>
    void fade_in(std::chrono::duration<Rep, Period> time)
    {
        fade_in(time_to_frame(time));
    }

    template<typename Rep, typename Period>
    void fade_out(std::chrono::duration<Rep, Period> time)
    {
        fade_out(time_to_frame(time));
    }

    void set_volume(float volume);
    void set_loop_points(std::uint64_t begin_frame, std::uint64_t end_frame);
    void enable_spatialization();
    void disable_spatialization();
    void relative_spatialization();
    void absolute_spatialization();
    void set_minimum_distance(float distance);
    void set_attenuation(float attenuation);
    void move(const vec3f& relative);
    void move_to(const vec3f& position);
    void seek(std::uint64_t frame);

    template<typename Rep1, typename Period1, typename Rep2, typename Period2>
    void set_loop_points(std::chrono::duration<Rep1, Period1> begin, std::chrono::duration<Rep2, Period2> end)
    {
        set_loop_points(time_to_frame(begin), time_to_frame(end));
    }

    template<typename Rep, typename Period>
    void seek(std::chrono::duration<Rep, Period> time)
    {
        seek(time_to_frame(time));
    }

    std::unique_ptr<sound_reader> change_reader(std::unique_ptr<sound_reader> new_reader);

    sound_status status() const;
    float volume() const;
    std::pair<std::uint64_t, std::uint64_t> loop_points() const;
    bool is_spatialization_enabled() const;
    bool is_spatialization_relative() const;
    float minimum_distance() const;
    float attenuation() const;
    vec3f position() const;
    std::uint64_t tell() const;

    template<typename DurationT>
    DurationT frames_to_time(std::uint64_t frames) const
    {
        return std::chrono::duration_cast<DurationT>(seconds{static_cast<double>(frames) / m_data->reader->info().frequency});
    }

    template<typename Rep, typename Period>
    std::uint64_t time_to_frame(std::chrono::duration<Rep, Period> time) const
    {
        return static_cast<std::uint64_t>(std::chrono::duration_cast<seconds>(time).count() * m_data->reader->info().frequency);
    }

private:
    impl::sound_data* m_data{};
};

class SWELL_API audio_world
{
    template<typename T>
    class default_init_allocator : public std::allocator<T>
    {
        template<typename U>
        void construct(U* ptr) noexcept(std::is_nothrow_default_constructible<U>::value)
        {
            new (ptr) U;
        }

        template <typename U, typename...Args>
        void construct(U* ptr, Args&&... args)
        {
            std::construct_at(ptr, std::forward<Args>(args)...);
        }
    };

public:
    audio_world() = default;
    explicit audio_world(std::uint32_t sample_rate);

    ~audio_world() = default;
    audio_world(const audio_world&) = delete;
    audio_world& operator=(const audio_world&) = delete;
    audio_world(audio_world&& other) noexcept = delete;
    audio_world& operator=(audio_world&& other) noexcept = delete;

    void set_up(const vec3f& direction);

    template<typename... Listeners>
    void bind_listener(Listeners&... listeners)
    {
        static_assert(sizeof...(Listeners) != 0 && (std::is_same_v<listener, std::decay_t<Listeners>> && ...), "One of the parameters is not a swl::listener.");

        const std::array locks{std::lock_guard{listeners.m_data->mutex}...};
        (m_listeners_data.emplace_back(&listeners.m_data->queue, listeners.m_data->state), ...);
    }

    void discard(std::size_t frame_count);
    void generate(std::size_t frame_count);

    vec3f up() const;

    std::uint32_t sample_rate() const noexcept
    {
        return m_sample_rate;
    }

    impl::sound_data* make_sound();

private:
    struct sound_data_buffer
    {
        std::span<float> samples{};
        impl::sound_state state{};
    };

    struct listener_data_buffer
    {
        audio_queue* queue{};
        impl::listener_state state{};
    };

private:
    void discard_impl(std::size_t frame_count);
    void discard_sound_data(impl::sound_data& sound, std::size_t frame_count);

    void store_sounds_data(std::size_t frame_count);
    std::span<float> get_sound_data(impl::sound_data& sound, std::size_t frame_count);

    void apply_fading(sound_data_buffer& sound, std::size_t frame_count);
    void spatialize(listener_data_buffer& listener, sound_data_buffer& sound, std::size_t frame_count) noexcept;
    void adjust_channels(listener_data_buffer& listener, sound_data_buffer& sound, std::size_t frame_count) noexcept;
    void mix_sounds();

    void free_resources();

private:
    std::uint32_t m_sample_rate{};

    vec3f m_up{0.0f, 1.0f, 0.0f};

    std::vector<std::unique_ptr<impl::sound_data>> m_sounds{};
    std::vector<float, default_init_allocator<float>> m_sample_buffer{};
    std::vector<sound_data_buffer> m_sounds_data{};
    std::vector<listener_data_buffer> m_listeners_data{};

    std::span<float> m_listener_samples{};

    mutable std::mutex m_mutex{};
};

}

#endif
