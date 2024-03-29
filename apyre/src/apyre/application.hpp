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
