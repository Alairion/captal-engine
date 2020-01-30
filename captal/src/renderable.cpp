#include "renderable.hpp"

#include <cassert>

#include <tephra/commands.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace cpt
{

std::vector<buffer_part> compute_buffer_parts(std::uint32_t vertex_count)
{
    return std::vector<buffer_part>
    {
        buffer_part{buffer_part_type::uniform, sizeof(glm::mat4)},
        buffer_part{buffer_part_type::vertex, vertex_count * sizeof(vertex)}
    };
}

std::vector<buffer_part> compute_buffer_parts(std::uint32_t index_count, std::uint32_t vertex_count)
{
    return std::vector<buffer_part>
    {
        buffer_part{buffer_part_type::uniform, sizeof(glm::mat4)},
        buffer_part{buffer_part_type::index, index_count * sizeof(std::uint16_t)},
        buffer_part{buffer_part_type::vertex, vertex_count * sizeof(vertex)}
    };
}

renderable::renderable(std::uint32_t vertex_count)
:m_vertex_count{vertex_count}
,m_buffer{compute_buffer_parts(vertex_count)}
,m_texture{engine::instance().dummy_texture()}
{
    update();
}

renderable::renderable(std::uint32_t index_count, std::uint32_t vertex_count)
:m_index_count{index_count}
,m_vertex_count{vertex_count}
,m_buffer{compute_buffer_parts(index_count, vertex_count)}
,m_texture{engine::instance().dummy_texture()}
{
    update();
}

void renderable::set_indices(const std::vector<std::uint16_t>& indices) noexcept
{
    assert(m_index_count > 0 && "cpt::renderable::set_indexes called on a renderable without index buffer");

    std::memcpy(&m_buffer.get<std::uint16_t>(1), std::data(indices), std::size(indices) * sizeof(std::uint16_t));
    m_need_upload = true;
}

void renderable::set_vertices(const std::vector<vertex>& vertices) noexcept
{
    std::memcpy(&m_buffer.get<vertex>(m_index_count > 0 ? 2 : 1), std::data(vertices), std::size(vertices) * sizeof(vertex));
    m_need_upload = true;
}

void renderable::set_render_technique(render_technique_ptr technique) noexcept
{
    m_render_technique = std::move(technique);
    m_need_descriptor_update = true;
}

void renderable::set_texture(texture_ptr texture) noexcept
{
    m_texture = std::move(texture);
    m_need_descriptor_update = true;
}

void renderable::set_view(const view_ptr& view)
{
    const auto it{m_descriptor_sets.find(view)};
    if(it == std::end(m_descriptor_sets))
    {
        auto& set{m_descriptor_sets[view]};

        set = m_render_technique->make_set();
        tph::write_descriptor(engine::instance().renderer(), set->set(), 0, view->buffer(), 0, view->buffer().size());
        tph::write_descriptor(engine::instance().renderer(), set->set(), 1, m_buffer.buffer(), 0, sizeof(glm::mat4));
        tph::write_descriptor(engine::instance().renderer(), set->set(), 2, m_texture->get_texture());

        m_current_set = set.get();
        update();
    }
    else
    {
        if(m_need_descriptor_update)
        {
            it->second = m_render_technique->make_set();
            tph::write_descriptor(engine::instance().renderer(), it->second->set(), 0, view->buffer(), 0, view->buffer().size());
            tph::write_descriptor(engine::instance().renderer(), it->second->set(), 1, m_buffer.buffer(), 0, sizeof(glm::mat4));
            tph::write_descriptor(engine::instance().renderer(), it->second->set(), 2, m_texture->get_texture());
        }

        m_current_set = it->second.get();
        update();
    }

    m_need_descriptor_update = false;
}

void renderable::update()
{
    m_need_upload = true;
}

void renderable::upload()
{
    if(std::exchange(m_need_upload, false))
    {
        m_buffer.get<glm::mat4>(0) = glm::translate(glm::rotate(glm::scale(glm::mat4{1}, glm::vec3{m_scale, m_scale, m_scale}), m_rotation, glm::vec3{0.0f, 0.0f, 1.0f}), m_position - m_origin);
        m_buffer.upload();
    }
}

void renderable::draw(tph::command_buffer& buffer)
{
    if(m_index_count > 0)
    {
        tph::cmd::bind_index_buffer(buffer, m_buffer.buffer(), m_buffer.compute_offset(1), tph::index_type::uint16);
        tph::cmd::bind_vertex_buffer(buffer, m_buffer.buffer(), m_buffer.compute_offset(2));
        tph::cmd::bind_descriptor_set(buffer, m_current_set->set(), m_render_technique->pipeline_layout());
        tph::cmd::draw_indexed(buffer, m_index_count, 1, 0, 0, 0);
    }
    else
    {
        tph::cmd::bind_vertex_buffer(buffer, m_buffer.buffer(), m_buffer.compute_offset(1));
        tph::cmd::bind_descriptor_set(buffer, m_current_set->set(), m_render_technique->pipeline_layout());
        tph::cmd::draw(buffer, m_vertex_count, 1, 0, 0);
    }
}

}
