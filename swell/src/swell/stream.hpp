#ifndef SWELL_STREAM_HPP_INCLUDED
#define SWELL_STREAM_HPP_INCLUDED

#include "config.hpp"

#include <mutex>
#include <memory>
#include <functional>

#include "physical_device.hpp"

namespace swl
{

class application;
class mixer;

struct stream_info
{
    sample_format format{};
    std::uint32_t channel_count{};
    std::uint32_t sample_rate{};
    seconds latency{};
};

enum class stream_callback_flags : std::uint32_t
{
    none      = 0x00,
    underflow = 0x01,
    overflow  = 0x02
};

class SWELL_API stream
{
    static inline std::mutex mutex{};

public:
    using output_callback = std::function<stream_callback_result(std::size_t frame_count, void* samples, seconds time, stream_callback_flags flags)>;
    using input_callback  = std::function<stream_callback_result(std::size_t frame_count, const void* samples, seconds time, stream_callback_flags flags)>;

public:
    constexpr stream() = default;
    explicit stream(application& application, const physical_device& device, const stream_info& info, output_callback callback);
    explicit stream(application& application, const physical_device& device, const stream_info& info, input_callback callback);

    ~stream();
    stream(const stream&) = delete;
    stream& operator=(const stream&) = delete;
    stream(stream&& other) noexcept;
    stream& operator=(stream&& other) noexcept;

    void start();
    void stop();
    bool active() const noexcept;
    seconds time() const noexcept;

    const stream_info& info() const noexcept
    {
        return m_info;
    }

private:
    void* m_stream{};
    stream_info m_info{};
    std::unique_ptr<output_callback> m_output_callback{};
    std::unique_ptr<input_callback> m_input_callback{};
};

bool is_input_format_supported(application& application, physical_device& device, const stream_info& info);
bool is_output_format_supported(application& application, physical_device& device, const stream_info& info);

}

template<> struct swl::enable_enum_operations<swl::stream_callback_flags> {static constexpr bool value{true};};

#endif
