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

#include "application.hpp"

namespace cpt
{

#ifdef CAPTAL_DEBUG
static constexpr auto graphics_layers{tph::application_layer::validation};
static constexpr auto graphics_extensions{tph::application_extension::debug_utils | tph::application_extension::surface};
static constexpr auto debug_severities{tph::debug_message_severity::error | tph::debug_message_severity::warning};
static constexpr auto debug_types{tph::debug_message_type::general | tph::debug_message_type::performance | tph::debug_message_type::validation};
#else
static constexpr auto graphics_layers{tph::application_layer::none};
static constexpr auto graphics_extensions{tph::application_extension::surface};
#endif

application::application(const std::string& application_name, cpt::version version, apr::application_extension apr_extensions, tph::application_layer tph_layers, tph::application_extension tph_extensions)
:m_system_application{apr_extensions}
,m_graphics_application{application_name, version, tph_layers | graphics_layers, tph_extensions | graphics_extensions}
#ifdef CAPTAL_DEBUG
,m_debug_messenger{m_graphics_application, tph::debug_messenger_default_callback, debug_severities, debug_types}
#endif
{

}

}
