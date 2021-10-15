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

#ifndef CAPTAL_APPLICATION_HPP_INCLUDED
#define CAPTAL_APPLICATION_HPP_INCLUDED

#include "config.hpp"

#include <apyre/application.hpp>
#include <swell/application.hpp>
#include <tephra/application.hpp>
#include <tephra/debug_utils.hpp>

namespace cpt
{

class CAPTAL_API application
{
public:
    explicit application(const std::string& application_name, cpt::version version,
                         apr::application_extension apr_extensions = apr::application_extension::none,
                         tph::application_layer tph_layers = tph::application_layer::none,
                         tph::application_extension tph_extensions = tph::application_extension::none);

    ~application() = default;
    application(const application&) = delete;
    application& operator=(const application&) = delete;
    application(application&& other) noexcept = default;
    application& operator=(application&& other) noexcept = default;

    apr::application& system_application() noexcept
    {
        return m_system_application;
    }

    const apr::application& system_application() const noexcept
    {
        return m_system_application;
    }

    swl::application& audio_application() noexcept
    {
        return m_audio_application;
    }

    const swl::application& audio_application() const noexcept
    {
        return m_audio_application;
    }

    tph::application& graphics_application() noexcept
    {
        return m_graphics_application;
    }

    const tph::application& graphics_application() const noexcept
    {
        return m_graphics_application;
    }

    const tph::debug_messenger& debug_messenger() const noexcept
    {
        return m_debug_messenger;
    }

private:
    apr::application m_system_application{};
    swl::application m_audio_application{};
    tph::application m_graphics_application{};
    tph::debug_messenger m_debug_messenger{};
};

}

#endif
