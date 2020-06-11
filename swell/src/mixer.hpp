#ifndef SWELL_MIXER_HPP_INCLUDED
#define SWELL_MIXER_HPP_INCLUDED

#include "config.hpp"

#include <vector>
#include <iostream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <chrono>

#include <glm/vec3.hpp>

namespace swl
{

using clock = std::chrono::steady_clock;
using sample_buffer_type = std::vector<float>;

enum class sound_reader_options : std::uint32_t
{
    none = 0x00,
    buffered = 0x01
};

class sound_reader
{
public:
    virtual ~sound_reader()
    {

    }

    virtual bool read(float* output [[maybe_unused]], std::size_t frame_count [[maybe_unused]])
    {
        return false;
    }

    virtual void seek(std::uint64_t frame_offset [[maybe_unused]])
    {

    }

    virtual std::uint64_t tell()
    {
        return 0;
    }

    virtual std::uint64_t frame_count()
    {
        return 0;
    }

    virtual std::uint32_t frequency()
    {
        return 0;
    }

    virtual std::uint32_t channel_count()
    {
        return 0;
    }

protected:
    sound_reader() = default;
    sound_reader(const sound_reader&) = delete;
    sound_reader& operator=(const sound_reader&) = delete;
    sound_reader(sound_reader&& other) noexcept = default;
    sound_reader& operator=(sound_reader&& other) noexcept = default;
};

enum class sound_status : std::uint32_t
{
    stopped = 0,
    playing = 1,
    paused = 2,
    fading_in = 3,
    fading_out = 4,
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
    glm::vec3 position{};
};

struct sound_state
{
    sound_status status{sound_status::stopped};
    float volume{1.0};
    std::uint64_t current_frame{};
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

class audio_queue
{
public:
    audio_queue() = default;
    ~audio_queue() = default;
    audio_queue(const audio_queue&) = delete;
    audio_queue& operator=(const audio_queue&) = delete;
    audio_queue(audio_queue&&) noexcept = delete;
    audio_queue& operator=(audio_queue&&) noexcept = delete;

    template<typename InputIt>
    void push(InputIt begin, InputIt end)
    {
        std::unique_lock lock{m_mutex};
        m_data.insert(std::end(m_data), begin, end);
        lock.unlock();

        m_condition.notify_one();
    }

    template<typename OutputIt>
    void drain(OutputIt output, std::size_t count)
    {
        std::unique_lock lock{m_mutex};

        if(std::size(m_data) < count)
        {
            m_condition.wait(lock, [this, count]
            {
                return std::size(m_data) >= count;
            });
        }

        std::copy_n(std::begin(m_data), count, output);
        m_data.erase(std::begin(m_data), std::begin(m_data) + count);
    }

    std::size_t buffered()
    {
        std::unique_lock lock{m_mutex};

        return std::size(m_data);
    }

private:
    std::mutex m_mutex{};
    std::condition_variable m_condition{};
    std::vector<float> m_data{};
};

}

class mixer;

class sound
{
public:
    sound() = default;
    sound(mixer& mixer, std::unique_ptr<sound_reader> reader);

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
    void move(const glm::vec3& relative);
    void move_to(const glm::vec3& position);
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
    glm::vec3 position() const;
    std::uint64_t tell() const;
    sound_reader& reader() const;

    template<typename DurationT>
    DurationT frames_to_time(std::uint64_t frames) const
    {
        return std::chrono::duration_cast<DurationT>(time_type{static_cast<double>(frames) / m_data->reader->frequency()});
    }

    template<typename Rep, typename Period>
    std::uint64_t time_to_frame(std::chrono::duration<Rep, Period> time) const
    {
        return static_cast<std::uint64_t>(std::chrono::duration_cast<time_type>(time).count() * m_data->reader->frequency());
    }

private:
    impl::sound_data* m_data{};
};

class mixer
{
public:
    mixer(std::uint32_t sample_rate, std::uint32_t channel_count, time_type minimum_latency = time_type{0.002}, time_type discard_threshold = time_type{0.005});
    ~mixer();
    mixer(const mixer&) = delete;
    mixer& operator=(const mixer&) = delete;
    mixer(mixer&& other) noexcept = delete;
    mixer& operator=(mixer&& other) noexcept = delete;

    template<typename OutputIt>
    void next(OutputIt output, std::size_t frame_count)
    {
        m_queue.drain(output, frame_count * m_channel_count);
    }

    void move_listener(const glm::vec3& relative);
    void move_listener_to(const glm::vec3& position);
    void set_listener_direction(const glm::vec3& direction);
    void set_up(const glm::vec3& direction);

    impl::sound_data* make_sound();

    std::uint32_t sample_rate() const noexcept
    {
        return m_sample_rate;
    }

    std::uint32_t channel_count() const noexcept
    {
        return m_channel_count;
    }

    bool is_ok() const noexcept
    {
        return m_ok.load(std::memory_order_acquire);
    }

private:
    void process() noexcept;

    std::vector<sample_buffer_type> get_sounds_data(std::size_t frame_count);
    sample_buffer_type get_sound_data(impl::sound_data& sound, std::size_t frame_count, std::uint32_t channels);
    sample_buffer_type apply_volume(impl::sound_data& sound, sample_buffer_type data, std::size_t frame_count, std::uint32_t channels);
    sample_buffer_type spatialize(impl::sound_data& sound, sample_buffer_type data, std::size_t frame_count, std::uint32_t channels);
    sample_buffer_type mix_sounds(const std::vector<sample_buffer_type>& sounds_data, std::size_t frame_count);

    void free_sounds();

private:
    std::uint32_t m_sample_rate{};
    std::uint32_t m_channel_count{};
    time_type m_minimum_latency{};
    time_type m_discard_threshold{};

    glm::vec3 m_position{0.0f, 0.0f, 0.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};
    glm::vec3 m_direction{0.0f, 0.0f, 1.0f};

    std::vector<std::unique_ptr<impl::sound_data>> m_sounds{};

    impl::audio_queue m_queue{};
    std::atomic<bool> m_running{};
    std::atomic<bool> m_ok{};
    std::mutex m_mutex{};
    std::thread m_process_thread{};
};

}

template<> struct swl::enable_enum_operations<swl::sound_reader_options> {static constexpr bool value{true};};

#endif
