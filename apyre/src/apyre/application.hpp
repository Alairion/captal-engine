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

enum class application_extension : std::uint32_t
{
    none = 0x00,
    extended_client_area = 0x01
};

class APYRE_API application
{
public:
    application(application_extension extensions = application_extension::none);
    ~application();
    application(const application&) = delete;
    application& operator=(const application&) = delete;
    application(application&& other) noexcept;
    application& operator=(application&& other) noexcept;

    apr::event_queue& event_queue() noexcept
    {
        return *m_event_queue;
    }

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

    application_extension extensions() const noexcept
    {
        return m_extensions;
    }

private:
    std::unique_ptr<apr::event_queue> m_event_queue{};
    std::vector<monitor> m_monitors{};
    application_extension m_extensions{};
    bool m_free{};
};

}

template<> struct apr::enable_enum_operations<apr::application_extension> {static constexpr bool value{true};};

#endif
