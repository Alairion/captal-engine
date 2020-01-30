#include "application.hpp"

namespace cpt
{

#ifdef CAPTAL_DEBUG
static constexpr tph::application_options graphics_options{tph::application_options::enable_validation};
#else
static constexpr tph::application_options graphics_options{};
#endif

application::application(const std::string& application_name, tph::version version)
:m_graphics_application{application_name, version, graphics_options}
{

}

}
