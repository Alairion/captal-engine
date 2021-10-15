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
