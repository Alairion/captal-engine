#ifndef SWELL_MIXER_HPP_INCLUDED
#define SWELL_MIXER_HPP_INCLUDED

#include "config.hpp"

#include <vector>
#include <thread>
#include <mutex>
#include <future>
#include <chrono>

namespace swl
{

using clock = std::chrono::steady_clock;

enum class sound_reader_options : std::uint32_t
{
    none = 0x00,
    buffered = 0x01
};

class sound_reader
{
public:
    virtual ~sound_reader() = default;

    bool read(float* output, std::size_t sample_count)
    {
        return read_samples(output, sample_count);
    }

    void seek(std::uint64_t frame_offset)
    {
        seek_samples(frame_offset);
    }

    std::uint64_t frame_count()
    {
        return get_frame_count();
    }

    std::uint32_t frequency()
    {
        return get_frequency();
    }

    std::uint32_t channels_count()
    {
        return get_channels();
    }

protected:
    sound_reader() = default;
    sound_reader(const sound_reader&) = delete;
    sound_reader& operator=(const sound_reader&) = delete;
    sound_reader(sound_reader&& other) noexcept = default;
    sound_reader& operator=(sound_reader&& other) noexcept = default;

    virtual bool read_samples(float* output [[maybe_unused]], std::size_t frame_count [[maybe_unused]])
    {
        return false;
    }

    virtual void seek_samples(std::uint64_t frame_offset [[maybe_unused]])
    {

    }

    virtual std::uint64_t get_frame_count()
    {
        return 0;
    }

    virtual std::uint32_t get_frequency()
    {
        return 0;
    }

    virtual std::uint32_t get_channels()
    {
        return 0;
    }
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

struct vec3
{
    float x{};
    float y{};
    float z{};
};

struct sound_spatialization
{
    bool enable{};
    bool relative{};
    float minimum_distance{1.0f};
    float attenuation{1.0f};
    vec3 position{};
};

struct sound_state
{
    sound_status status{sound_status::stopped};
    float volume{1.0};
    std::uint64_t current_sample{};
    std::uint64_t loop_begin{};
    std::uint64_t loop_end{std::numeric_limits<std::uint64_t>::max()};
    std::uint64_t fading{std::numeric_limits<std::uint64_t>::max()};
    std::uint64_t current_fading{};
    sound_spatialization spatialization{};
};

struct sound_snapshot
{
    sound_state state{};
    clock::time_point end{};
};

struct sound_data
{
    sound_reader* reader{};
    sound_state state{};
    std::vector<sound_snapshot> timeline{};
    std::mutex mutex{};
};

}

class mixer;

class sound
{
public:
    sound() = default;
    sound(mixer& mixer, sound_reader& reader);

    ~sound();
    sound(const sound&) = delete;
    sound& operator=(const sound&) = delete;
    sound(sound&& other) noexcept;
    sound& operator=(sound&& other) noexcept;

    void start();
    void stop();
    void pause();
    void resume();
    void fade_out(std::uint64_t samples);
    void fade_in(std::uint64_t samples);

    void set_volume(float volume);
    void set_loop_points(std::uint64_t begin_frame, std::uint64_t end_frame);
    void enable_spatialization();
    void disable_spatialization();
    void set_relative_spatialization();
    void set_absolute_spatialization();
    void set_minimum_distance(float distance);
    void set_attenuation(float attenuation);
    void move(float x, float y, float z);

    sound_status status() const;

private:
    void snapshot();

private:
    impl::sound_data* m_data{};
};

class mixer
{
public:
    using sample_buffer_type = std::vector<float>;

public:
    mixer(std::size_t frame_per_buffer, std::uint32_t channel_count, std::uint32_t sample_rate, time_type minimum_latency = time_type{0.002});
    ~mixer();
    mixer(const mixer&) = delete;
    mixer& operator=(const mixer&) = delete;
    mixer(mixer&& other) noexcept = delete;
    mixer& operator=(mixer&& other) noexcept = delete;

    sample_buffer_type next();
    void move_listener(float x, float y, float z);
    void set_up(float x, float y, float z);
    void set_listener_direction(float x, float y, float z);

    impl::sound_data* make_sound();

    std::size_t frame_per_buffer() const noexcept
    {
        return m_buffer_size;
    }

    std::uint32_t channel_count() const noexcept
    {
        return m_channel_count;
    }

    std::uint32_t sample_rate() const noexcept
    {
        return m_sample_rate;
    }

    bool is_ok() const noexcept
    {
        return m_ok.load(std::memory_order_acquire);
    }

private:
    void process() noexcept;

    std::vector<sample_buffer_type> get_sounds_data(std::chrono::steady_clock::time_point now, std::size_t frame_count);
    sample_buffer_type get_sound_data(impl::sound_data& sound, std::size_t frame_count, std::uint32_t channels);
    sample_buffer_type apply_volume(impl::sound_data& sound, sample_buffer_type data, std::uint32_t channels);
    sample_buffer_type spatialize(impl::sound_data& sound, sample_buffer_type data, std::uint32_t channels);
    sample_buffer_type mix_sounds(const std::vector<sample_buffer_type>& sounds_data);

    void free_sounds();

private:
    std::size_t m_buffer_size{};
    std::uint32_t m_channel_count{};
    std::uint32_t m_sample_rate{};
    time_type m_minimum_latency{};
    impl::vec3 m_position{0.0f, 0.0f, 0.0f};
    impl::vec3 m_up{0.0f, 1.0f, 0.0f};
    impl::vec3 m_direction{0.0f, 0.0f, 1.0f};
    std::vector<std::unique_ptr<impl::sound_data>> m_sounds{};
    std::promise<sample_buffer_type> m_data_promise{};
    std::future<sample_buffer_type> m_data_future{};
    std::promise<void> m_next_promise{};
    std::future<void> m_next_future{};
    std::atomic<bool> m_running{};
    std::atomic<bool> m_ok{};
    std::mutex m_mutex{};
    std::thread m_process_thread{};
    clock::time_point m_last_tick{clock::now()};
    std::size_t m_buffered{};
};

}

template<> struct swl::enable_enum_operations<swl::sound_reader_options> {static constexpr bool value{true};};

#endif
