#ifndef SWELL_PHYSICAL_DEVICE_HPP_INCLUDED
#define SWELL_PHYSICAL_DEVICE_HPP_INCLUDED

#include "config.hpp"

#include <string>
#include <chrono>

namespace swl
{

class physical_device
{
public:
    physical_device() = default;

    physical_device(std::int32_t id, std::uint32_t host, std::uint32_t output_channel, seconds default_low_output_latency, seconds default_high_output_latency, std::uint32_t default_sample_rate, std::string name)
    :m_id{id}
    ,m_host{host}
    ,m_max_output_channel{output_channel}
    ,m_default_low_output_latency{default_low_output_latency}
    ,m_default_high_output_latency{default_high_output_latency}
    ,m_default_sample_rate{default_sample_rate}
    ,m_name{std::move(name)}
    {

    }

    ~physical_device() = default;
    physical_device(const physical_device&) = delete;
    physical_device& operator=(const physical_device&) = delete;
    physical_device(physical_device&& other) noexcept = default;
    physical_device& operator=(physical_device&& other) noexcept = default;

    std::int32_t id() const noexcept
    {
        return m_id;
    }

    std::uint32_t host_api() const noexcept
    {
        return m_host;
    }

    std::uint32_t max_output_channel() const noexcept
    {
        return m_max_output_channel;
    }

    seconds default_low_output_latency() const noexcept
    {
        return m_default_low_output_latency;
    }

    seconds default_high_output_latency() const noexcept
    {
        return m_default_high_output_latency;
    }

    std::uint32_t default_sample_rate() const noexcept
    {
        return m_default_sample_rate;
    }

    std::string_view name() const noexcept
    {
        return m_name;
    }

    std::size_t default_low_latency_buffer_size() const noexcept
    {
        return static_cast<std::size_t>(static_cast<double>(m_default_sample_rate) * m_default_low_output_latency.count());
    }

    std::size_t default_high_latency_buffer_size() const noexcept
    {
        return static_cast<std::size_t>(static_cast<double>(m_default_sample_rate) * m_default_high_output_latency.count());
    }

    std::size_t default_low_latency_buffer_size(std::uint32_t sample_rate) const noexcept
    {
        return static_cast<std::size_t>(static_cast<double>(sample_rate) * m_default_low_output_latency.count());
    }

    std::size_t default_high_latency_buffer_size(std::uint32_t sample_rate) const noexcept
    {
        return static_cast<std::size_t>(static_cast<double>(sample_rate) * m_default_high_output_latency.count());
    }

private:
    std::int32_t m_id{};
    std::uint32_t m_host{};
    std::uint32_t m_max_output_channel{};
    seconds m_default_low_output_latency{};
    seconds m_default_high_output_latency{};
    std::uint32_t m_default_sample_rate{};
    std::string m_name{};
};

}

#endif
