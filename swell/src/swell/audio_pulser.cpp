#include "audio_pulser.hpp"

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

#include <captal_foundation/stack_allocator.hpp>

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
        m_status = pulser_status::stopped;
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
                return m_status == pulser_status::running || m_status == pulser_status::aborted;
            });

            if(m_status == pulser_status::aborted)
            {
                break;
            }

            const auto now    {clock::now()};
            const auto elapsed{std::chrono::duration_cast<seconds>(now - m_last)};

            if(elapsed >= m_resync_threshold)
            {
                m_last = now;

                const auto jump{elapsed - m_minimum_latency};

                lock.unlock();
                m_world->discard(static_cast<std::size_t>(jump.count() * static_cast<double>(m_world->sample_rate())));

                lock.lock();
                register_listeners();
                lock.unlock();

                m_world->generate(static_cast<std::size_t>(m_minimum_latency.count() * static_cast<double>(m_world->sample_rate())));
            }
            else if(elapsed >= m_minimum_latency)
            {
                m_last = now;

                register_listeners();
                lock.unlock();

                m_world->generate(static_cast<std::size_t>(elapsed.count() * static_cast<double>(m_world->sample_rate())));
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
