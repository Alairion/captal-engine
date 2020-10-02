#ifndef TEPHRA_SYNCHRONIZATION_HPP_INCLUDED
#define TEPHRA_SYNCHRONIZATION_HPP_INCLUDED

#include "config.hpp"

#include <chrono>

#include "vulkan/vulkan.hpp"

namespace tph
{

class renderer;

class TEPHRA_API semaphore
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr semaphore() = default;
    explicit semaphore(renderer& renderer);

    explicit semaphore(vulkan::semaphore semaphore) noexcept
    :m_semaphore{std::move(semaphore)}
    {

    }

    ~semaphore() = default;
    semaphore(const semaphore&) = delete;
    semaphore& operator=(const semaphore&) = delete;
    semaphore(semaphore&& other) noexcept = default;
    semaphore& operator=(semaphore&& other) noexcept = default;

private:
    vulkan::semaphore m_semaphore{};
};

template<>
inline VkDevice underlying_cast(const semaphore& semaphore) noexcept
{
    return semaphore.m_semaphore.device();
}

template<>
inline VkSemaphore underlying_cast(const semaphore& semaphore) noexcept
{
    return semaphore.m_semaphore;
}

class TEPHRA_API fence
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr fence() = default;
    explicit fence(renderer& renderer, bool signaled = false);

    explicit fence(vulkan::fence fence) noexcept
    :m_fence{std::move(fence)}
    {

    }

    ~fence() = default;
    fence(const fence&) = delete;
    fence& operator=(const fence&) = delete;
    fence(fence&& other) noexcept = default;
    fence& operator=(fence&& other) noexcept = default;

    void wait() const
    {
        wait_impl(std::numeric_limits<std::uint64_t>::max());
    }

    bool try_wait() const
    {
        return wait_impl(0);
    }

    template<typename Rep, typename Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& timeout) const
    {
        return wait_impl(std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count());
    }

    template<typename Clock, typename Duration>
    bool wait_until(const std::chrono::time_point<Clock, Duration>& time) const
    {
        const auto current_time{Clock::now()};
        if(time < current_time)
        {
            return try_wait();
        }

        return wait_impl(std::chrono::duration_cast<std::chrono::nanoseconds>(time - current_time).count());
    }

    void reset();

private:
    bool wait_impl(std::uint64_t nanoseconds) const;

private:
    vulkan::fence m_fence{};
};

template<>
inline VkDevice underlying_cast(const fence& fence) noexcept
{
    return fence.m_fence.device();
}

template<>
inline VkFence underlying_cast(const fence& fence) noexcept
{
    return fence.m_fence;
}

class TEPHRA_API event
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr event() = default;
    explicit event(renderer& renderer);

    explicit event(vulkan::event event) noexcept
    :m_event{std::move(event)}
    {

    }

    ~event() = default;
    event(const event&) = delete;
    event& operator=(const event&) = delete;
    event(event&& other) noexcept = default;
    event& operator=(event&& other) noexcept = default;

    void set();
    void reset();

private:
    vulkan::event m_event{};
};

template<>
inline VkDevice underlying_cast(const event& event) noexcept
{
    return event.m_event.device();
}

template<>
inline VkEvent underlying_cast(const event& event) noexcept
{
    return event.m_event;
}

}

#endif
