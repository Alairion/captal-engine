#ifndef APYRE_APPLICATION_HPP_INCLUDED
#define APYRE_APPLICATION_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <vector>
#include <span>

#include "monitor.hpp"
#include "event.hpp"

namespace apr
{

class window;

class APYRE_API application
{
public:
    application();
    ~application();
    application(const application&) = delete;
    application& operator=(const application&) = delete;
    application(application&& other) noexcept;
    application& operator=(application&& other) noexcept;

    std::span<const monitor> enumerate_monitors() const noexcept
    {
        return m_monitors;
    }

    const monitor& main_monitor() const noexcept
    {
        for(const auto& monitor : m_monitors)
        {
            if(monitor.is_main_monitor())
            {
                return monitor;
            }
        }

        return m_monitors[0];
    }

    apr::event_queue& event_queue() noexcept
    {
        return *m_event_queue;
    }

private:
    std::unique_ptr<apr::event_queue> m_event_queue{};
    std::vector<monitor> m_monitors{};
    bool m_free{};
};

}

#endif
