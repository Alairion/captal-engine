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
