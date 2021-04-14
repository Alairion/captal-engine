#ifndef APYRE_MONITOR_HPP_INCLUDED
#define APYRE_MONITOR_HPP_INCLUDED

#include "config.hpp"

#include <string>

namespace apr
{

class APYRE_API monitor
{
    friend class application;

public:
    constexpr monitor() = default;
    ~monitor() = default;
    monitor(const monitor&) = delete;
    monitor& operator=(const monitor&) = delete;
    monitor(monitor&& other) noexcept = default;
    monitor& operator=(monitor&& other) noexcept = default;

    std::int32_t id() const noexcept
    {
        return m_id;
    }

    bool is_main_monitor() const noexcept
    {
        return m_x == 0 && m_y == 0;
    }

    std::int32_t x() const noexcept
    {
        return m_x;
    }

    std::int32_t y() const noexcept
    {
        return m_y;
    }

    std::uint32_t width() const noexcept
    {
        return m_width;
    }

    std::uint32_t height() const noexcept
    {
        return m_height;
    }

    double vertical_dpi() const noexcept
    {
        return m_vertical_dpi;
    }

    double horizontal_dpi() const noexcept
    {
        return m_horizontal_dpi;
    }

    double frequency() const noexcept
    {
        return m_frequency;
    }

    std::string_view name() const noexcept
    {
        return m_name;
    }

private:
    std::int32_t m_id{};
    std::int32_t m_x{};
    std::int32_t m_y{};
    std::uint32_t m_width{};
    std::uint32_t m_height{};
    double m_vertical_dpi{};
    double m_horizontal_dpi{};
    double m_frequency{};
    std::string m_name{};
};

}

#endif
