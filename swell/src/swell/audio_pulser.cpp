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

#include "audio_pulser.hpp"

#include <captal_foundation/stack_allocator.hpp>

#include <cassert>
#include <numbers>
#include <iostream>

#if defined(_WIN32)
    #include <windows.h>

    namespace swl
    {

    static void increase_thread_priority() noexcept
    {
        //SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    }

    }
#else
    namespace swl
    {

    static void increase_thread_priority() noexcept
    {

    }

    }
#endif

namespace swl
{

listener_bind::~listener_bind()
{
    if(m_data)
    {
        m_parent->unregister_bind(m_data);
    }
}

listener listener_bind::unregister()
{
    assert(m_data && "swl::listener_bind::unregister called on invalid listener bind.");

    listener output{m_parent->unregister_bind(m_data)};

    m_data = nullptr;

    return output;
}

audio_pulser::audio_pulser(audio_world& world, seconds minimum_latency, seconds resync_threshold)
:m_world{&world}
,m_minimum_latency{minimum_latency}
,m_resync_threshold{resync_threshold}
,m_frequency{static_cast<double>(world.sample_rate())}
,m_period{1.0 / m_frequency}
,m_thread{&audio_pulser::process, this}
{
    assert(m_minimum_latency < m_resync_threshold);
}

audio_pulser::~audio_pulser()
{
    if(m_thread.joinable())
    {
        std::unique_lock lock{m_mutex};
        m_status = pulser_status::aborted;
        lock.unlock();

        m_start_condition.notify_one();
        m_thread.join();
    }
}

void audio_pulser::start()
{
    std::unique_lock lock{m_mutex};

    if(m_status == pulser_status::stopped)
    {
        m_status = pulser_status::running;
        m_last = clock::now();
        m_start_condition.notify_one();
    }
}

void audio_pulser::stop()
{
    std::unique_lock lock{m_mutex};

    if(m_status == pulser_status::running)
    {
        m_status = pulser_status::stopping;
        m_stop_condition.wait(lock);
        assert(m_status == pulser_status::stopped);
    }
}

listener_bind audio_pulser::bind(listener listener)
{
    std::lock_guard lock{m_mutex};

    m_binds.emplace_back(std::make_unique<impl::listener_bind_data>(std::move(listener)));

    return listener_bind{this, m_binds.back().get()};
}

pulser_status audio_pulser::status() const
{
    std::lock_guard lock{m_mutex};

    return m_status;
}

void audio_pulser::process() noexcept
{
    increase_thread_priority();

    try
    {
        while(true)
        {
            std::unique_lock lock{m_mutex};
            m_start_condition.wait(lock, [this]
            {
                return m_status == pulser_status::running || m_status == pulser_status::aborted || m_status == pulser_status::stopping;
            });

            if(m_status == pulser_status::stopping)
            {
                m_status = pulser_status::stopped;
                m_stop_condition.notify_one();
                continue; // keep thread alive
            }
            else if(m_status == pulser_status::aborted)
            {
                break;
            }

            const auto now{clock::now()};
            m_elapsed += std::chrono::duration_cast<seconds>(now - m_last);
            m_last = now;

            if(m_elapsed >= m_resync_threshold)
            {
                const auto jump{std::floor((m_elapsed - m_minimum_latency).count())};

                const auto discard_count{std::floor(jump * m_frequency)};
                m_elapsed -= seconds{discard_count * m_period};

                lock.unlock();
                m_world->discard(static_cast<std::size_t>(discard_count));

                lock.lock();
                register_listeners();
                lock.unlock();

                const auto count{std::floor(m_elapsed.count() * m_frequency)};
                m_elapsed -= seconds{count * m_period};

                m_world->generate(static_cast<std::size_t>(m_minimum_latency.count()));
            }
            else if(m_elapsed >= m_minimum_latency)
            {
                register_listeners();
                lock.unlock();

                const auto count{std::floor(m_elapsed.count() * m_frequency)};
                m_elapsed -= seconds{count * m_period};

                m_world->generate(static_cast<std::size_t>(count));
            }
            else
            {
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds{1});
            }
        }
    }
    catch(...)
    {
        std::unique_lock lock{m_mutex};
        m_status = pulser_status::aborted;
    }
}

void audio_pulser::register_listeners()
{
    for(auto& listener : m_binds)
    {
        m_world->bind_listener(listener->listener);
    }
}

listener audio_pulser::unregister_bind(impl::listener_bind_data* bind)
{
    const auto predicate = [bind](const std::unique_ptr<impl::listener_bind_data>& ptr)
    {
        return ptr.get() == bind;
    };

    std::lock_guard lock{m_mutex};

    listener output{std::move(bind->listener)};

    const auto it{std::find_if(std::begin(m_binds), std::end(m_binds), predicate)};
    assert(it != std::end(m_binds) && "Invalid listener bind");
    m_binds.erase(it);

    return output;
}

stream_callback_result listener_bridge::operator()(std::size_t frame_count, void* samples, seconds time [[maybe_unused]], stream_callback_flags flags [[maybe_unused]])
{
    m_listener->drain(static_cast<float*>(samples), frame_count);

    return stream_callback_result::play;
}

}
