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

#ifndef SWELL_AUDIO_PULSER_HPP_INCLUDED
#define SWELL_AUDIO_PULSER_HPP_INCLUDED

#include "config.hpp"

#include <captal_foundation/math.hpp>

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <span>
#include <chrono>

#include "audio_world.hpp"
#include "stream.hpp"

namespace swl
{

namespace impl
{

struct listener_bind_data
{
    listener listener{};
};

}

class audio_pulser;

class SWELL_API listener_bind
{
public:
    listener_bind() = default;

    listener_bind(audio_pulser* parent, impl::listener_bind_data* data)
    :m_parent{parent}
    ,m_data{data}
    {

    }

    ~listener_bind();

    listener_bind(const listener_bind&) = delete;
    listener_bind& operator=(const listener_bind&) = delete;

    listener_bind(listener_bind&& other) noexcept
    :m_parent{other.m_parent}
    ,m_data{std::exchange(other.m_data, nullptr)}
    {

    }

    listener_bind& operator=(listener_bind&& other) noexcept
    {
        m_parent = other.m_parent;
        m_data = std::exchange(other.m_data, nullptr);

        return *this;
    }

    listener unregister();

    listener* operator->() noexcept
    {
        return &m_data->listener;
    }

    const listener* operator->() const noexcept
    {
        return &m_data->listener;
    }

    listener& operator*() noexcept
    {
        return m_data->listener;
    }

    const listener& operator*() const noexcept
    {
        return m_data->listener;
    }

private:
    audio_pulser* m_parent{};
    impl::listener_bind_data* m_data{};
};

enum class pulser_status : std::uint32_t
{
    stopped = 0,
    running = 1,
    aborted = 2
};

class SWELL_API audio_pulser
{
    friend class listener_bind;

public:
    audio_pulser(audio_world& world, seconds minimum_latency = seconds{0.010}, seconds resync_threshold = seconds{0.050});
    ~audio_pulser();
    audio_pulser(const audio_pulser&) = delete;
    audio_pulser& operator=(const audio_pulser&) = delete;
    audio_pulser(audio_pulser&& other) noexcept = delete;
    audio_pulser& operator=(audio_pulser&& other) noexcept = delete;

    void start();
    void stop();

    listener_bind bind(listener listener);

    pulser_status status() const;

private:
    void process() noexcept;
    void register_listeners();
    listener unregister_bind(impl::listener_bind_data* bind);

private:
    audio_world* m_world{};
    seconds m_minimum_latency{};
    seconds m_resync_threshold{};
    seconds m_elapsed{};
    double m_frequency{};
    double m_period{};
    clock::time_point m_last{};
    pulser_status m_status{};
    std::thread m_thread{};
    std::condition_variable m_start_condition{};
    mutable std::mutex m_mutex{};
    std::vector<std::unique_ptr<impl::listener_bind_data>> m_binds{};
};

class SWELL_API listener_bridge
{
public:
    listener_bridge() = default;

    explicit listener_bridge(listener& listener) noexcept
    :m_listener{&listener}
    {

    }

    ~listener_bridge() = default;
    listener_bridge(const listener_bridge&) = default;
    listener_bridge& operator=(const listener_bridge&) = default;
    listener_bridge(listener_bridge&&) noexcept = default;
    listener_bridge& operator=(listener_bridge&&) noexcept = default;

    stream_callback_result operator()(std::size_t frame_count, void* samples, seconds time, stream_callback_flags flags);

private:
    listener* m_listener{};
};

}

#endif
