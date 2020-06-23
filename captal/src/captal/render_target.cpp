#include "render_target.hpp"

#include "engine.hpp"

namespace cpt
{

render_target::render_target(const tph::render_pass_info& info)
:m_render_pass{engine::instance().renderer(), info}
{

}

}
