#include "render_target.hpp"

#include "engine.hpp"

namespace cpt
{

render_technique_ptr render_target::add_render_technique(const render_technique_info& info)
{
    return m_render_techniques.emplace_back(cpt::make_render_technique(*this, info));
}

render_technique_ptr render_target::add_render_technique(render_technique_ptr technique)
{
    return m_render_techniques.emplace_back(technique);
}

void render_target::remove_render_technique(render_technique_ptr technique)
{
    m_render_techniques.erase(std::find(std::begin(m_render_techniques), std::end(m_render_techniques), technique));
}

}
