#include "renderable.hpp"

#include <cassert>

#include <tephra/commands.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace cpt
{

static std::vector<buffer_part> compute_buffer_parts(std::uint32_t vertex_count)
{
    return std::vector<buffer_part>
    {
        buffer_part{buffer_part_type::uniform, sizeof(renderable::uniform_data)},
        buffer_part{buffer_part_type::vertex, vertex_count * sizeof(vertex)}
    };
}

static std::vector<buffer_part> compute_buffer_parts(std::uint32_t index_count, std::uint32_t vertex_count)
{
    return std::vector<buffer_part>
    {
        buffer_part{buffer_part_type::uniform, sizeof(renderable::uniform_data)},
        buffer_part{buffer_part_type::index, index_count * sizeof(std::uint32_t)},
        buffer_part{buffer_part_type::vertex, vertex_count * sizeof(vertex)}
    };
}

renderable::renderable(std::uint32_t vertex_count)
:m_vertex_count{vertex_count}
,m_buffer{compute_buffer_parts(vertex_count)}
{
    update();
}

renderable::renderable(std::uint32_t index_count, std::uint32_t vertex_count)
:m_index_count{index_count}
,m_vertex_count{vertex_count}
,m_buffer{compute_buffer_parts(index_count, vertex_count)}
{
    update();
}

void renderable::set_indices(const std::vector<std::uint32_t>& indices) noexcept
{
    assert(m_index_count > 0 && "cpt::renderable::set_indexes called on a renderable without index buffer");

    std::memcpy(&m_buffer.get<std::uint32_t>(1), std::data(indices), std::size(indices) * sizeof(std::uint32_t));

    m_need_upload = true;
}

void renderable::set_vertices(const std::vector<vertex>& vertices) noexcept
{
    const bool has_indexes{m_index_count > 0};
    const auto index{static_cast<std::size_t>(has_indexes ? 2 : 1)};

    std::memcpy(&m_buffer.get<vertex>(index), std::data(vertices), std::size(vertices) * sizeof(vertex));

    m_need_upload = true;
}

void renderable::set_texture(texture_ptr texture) noexcept
{
    m_texture = std::move(texture);
    m_need_descriptor_update = true;
}

void renderable::set_view(const view_ptr& view)
{
    const auto has_binding = [](const view_ptr& view, std::uint32_t binding)
    {
        for(auto&& layout_binding : view->render_technique()->bindings())
        {
            if(layout_binding.binding == binding)
            {
                return true;
            }
        }

        return false;
    };

    const auto write = [](const descriptor_set_ptr& set, std::uint32_t binding, cpt::uniform_binding& data)
    {
        if(get_uniform_binding_type(data) == uniform_binding_type::buffer)
        {
            const tph::descriptor_buffer_info info{std::get<framed_buffer_ptr>(data)->buffer(), 0, std::get<framed_buffer_ptr>(data)->size()};

            return tph::descriptor_write{set->set(), binding, 0, tph::descriptor_type::uniform_buffer, info};
        }
        else
        {
            const tph::descriptor_texture_info info{std::get<texture_ptr>(data)->get_texture(), tph::texture_layout::shader_read_only_optimal};

            return tph::descriptor_write{set->set(), binding, 0, tph::descriptor_type::image_sampler, info};
        }
    };

    const auto write_bindings = [has_binding, write, &view, this](const descriptor_set_ptr& set)
    {
        auto& texture{m_texture ? m_texture->get_texture() : engine::instance().default_texture().get_texture()};

        std::vector<tph::descriptor_write> writes{};

        writes.emplace_back(tph::descriptor_write{set->set(), 0, 0, tph::descriptor_type::uniform_buffer, tph::descriptor_buffer_info{view->buffer(), 0, view->buffer().size()}});
        writes.emplace_back(tph::descriptor_write{set->set(), 1, 0, tph::descriptor_type::uniform_buffer, tph::descriptor_buffer_info{m_buffer.buffer(), 0, m_buffer.size()}});
        writes.emplace_back(tph::descriptor_write{set->set(), 2, 0, tph::descriptor_type::image_sampler, tph::descriptor_texture_info{texture, tph::texture_layout::shader_read_only_optimal}});

        for(auto&& [binding, data] : m_uniform_bindings)
        {
            if(has_binding(view, binding))
            {
                writes.emplace_back(write(set, binding, data));
            }
        }

        for(auto&& [binding, data] : view->uniform_bindings())
        {
            if(has_binding(view, binding))
            {
                writes.emplace_back(write(set, binding, data));
            }
        }

        tph::write_descriptors(engine::instance().renderer(), writes);
    };

    const auto it{m_descriptor_sets.find(view.get())};
    if(it == std::end(m_descriptor_sets))
    {
        auto& set{m_descriptor_sets[view.get()]};

        set = view->render_technique()->make_set();
        write_bindings(set);

        m_current_set = set.get();
        update();
    }
    else
    {
        if(m_need_descriptor_update || view->need_descriptor_update())
        {
            it->second = view->render_technique()->make_set();
            write_bindings(it->second);
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
        glm::mat4 model{1.0f};
        model = glm::scale(model, glm::vec3{m_scale, m_scale, m_scale});
        model = glm::translate(model, m_position);
        model = glm::rotate(model, m_rotation, glm::vec3{0.0f, 0.0f, 1.0f});
        model = glm::translate(model, -m_origin);

        m_buffer.get<uniform_data>(0).model = model;

        m_buffer.upload();
    }
}

void renderable::draw(tph::command_buffer& buffer)
{
    if(m_index_count > 0)
    {
        tph::cmd::bind_index_buffer(buffer, m_buffer.buffer(), m_buffer.compute_offset(1), tph::index_type::uint32);
        tph::cmd::bind_vertex_buffer(buffer, m_buffer.buffer(), m_buffer.compute_offset(2));
        tph::cmd::bind_descriptor_set(buffer, m_current_set->set(), m_current_set->pool().technique().pipeline_layout());
        tph::cmd::draw_indexed(buffer, m_index_count, 1, 0, 0, 0);
    }
    else
    {
        tph::cmd::bind_vertex_buffer(buffer, m_buffer.buffer(), m_buffer.compute_offset(1));
        tph::cmd::bind_descriptor_set(buffer, m_current_set->set(), m_current_set->pool().technique().pipeline_layout());
        tph::cmd::draw(buffer, m_vertex_count, 1, 0, 0);
    }
}

}
