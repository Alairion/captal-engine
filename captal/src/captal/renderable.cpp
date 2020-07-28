#include "renderable.hpp"

#include <cassert>

#include <tephra/commands.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include "engine.hpp"

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
,m_buffer{make_uniform_buffer(compute_buffer_parts(vertex_count))}
{
    update();
}

renderable::renderable(std::uint32_t index_count, std::uint32_t vertex_count)
:m_index_count{index_count}
,m_vertex_count{vertex_count}
,m_buffer{make_uniform_buffer(compute_buffer_parts(index_count, vertex_count))}
{
    update();
}

void renderable::set_indices(std::span<const std::uint32_t> indices) noexcept
{
    assert(m_index_count > 0 && "cpt::renderable::set_indices called on a renderable without index buffer.");
    assert(std::size(indices) == m_index_count && "cpt::renderable::set_indices called with a wrong number of indices.");

    std::memcpy(&m_buffer->get<std::uint32_t>(1), std::data(indices), std::size(indices) * sizeof(std::uint32_t));

    update();
}

void renderable::set_vertices(std::span<const vertex> vertices) noexcept
{
    assert(std::size(vertices) == m_vertex_count && "cpt::renderable::set_vertices called with a wrong number of vertices.");

    const bool has_indices{m_index_count > 0};
    const auto index{static_cast<std::size_t>(has_indices ? 2 : 1)};

    std::memcpy(&m_buffer->get<vertex>(index), std::data(vertices), std::size(vertices) * sizeof(vertex));

    update();
}

void renderable::set_texture(texture_ptr texture) noexcept
{
    m_texture = std::move(texture);
    m_need_descriptor_update = true;
}

void renderable::set_view(const view_ptr& view)
{
    const auto write_set = [this](const view_ptr& view, const descriptor_set_ptr& set)
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

        const auto write = [](const descriptor_set_ptr& set, std::uint32_t binding, cpt::binding& data)
        {
            if(get_binding_type(data) == binding_type::uniform_buffer)
            {
                const tph::descriptor_buffer_info info{std::get<uniform_buffer_ptr>(data)->get_buffer(), 0, std::get<uniform_buffer_ptr>(data)->size()};

                return tph::descriptor_write{set->set(), binding, 0, tph::descriptor_type::uniform_buffer, info};
            }
            else if(get_binding_type(data) == binding_type::texture)
            {
                const tph::descriptor_texture_info info{std::get<texture_ptr>(data)->get_texture(), tph::texture_layout::shader_read_only_optimal};

                return tph::descriptor_write{set->set(), binding, 0, tph::descriptor_type::image_sampler, info};
            }
            else
            {
                const tph::descriptor_buffer_info info{std::get<storage_buffer_ptr>(data)->get_buffer(), 0, std::get<storage_buffer_ptr>(data)->size()};

                return tph::descriptor_write{set->set(), binding, 0, tph::descriptor_type::storage_buffer, info};
            }
        };

        auto& texture{m_texture ? m_texture->get_texture() : engine::instance().default_texture().get_texture()};

        std::vector<tph::descriptor_write> writes{};
        writes.emplace_back(tph::descriptor_write{set->set(), 0, 0, tph::descriptor_type::uniform_buffer, tph::descriptor_buffer_info{view->get_buffer(), 0, sizeof(view::uniform_data)}});
        writes.emplace_back(tph::descriptor_write{set->set(), 1, 0, tph::descriptor_type::uniform_buffer, tph::descriptor_buffer_info{m_buffer->get_buffer(), 0, sizeof(renderable::uniform_data)}});
        writes.emplace_back(tph::descriptor_write{set->set(), 2, 0, tph::descriptor_type::image_sampler, tph::descriptor_texture_info{texture, tph::texture_layout::shader_read_only_optimal}});

        for(auto&& [binding, data] : m_bindings)
        {
            if(has_binding(view, binding))
            {
                writes.emplace_back(write(set, binding, data));
            }
        }

        for(auto&& [binding, data] : view->bindings())
        {
            if(has_binding(view, binding))
            {
                writes.emplace_back(write(set, binding, data));
            }
        }

        tph::write_descriptors(engine::instance().renderer(), writes);
    };

    auto it{m_descriptor_sets.find(view.get())};

    if(it == std::end(m_descriptor_sets)) //New view
    {
        const auto [new_item, success] = m_descriptor_sets.emplace(std::make_pair(view.get(), view->render_technique()->make_set()));
        assert(success);

        it = new_item;
        write_set(view, it->second);
    }
    else if(m_need_descriptor_update || view->need_descriptor_update()) //Already known view check
    {
        it->second = view->render_technique()->make_set();
        write_set(view, it->second);
    }

    m_current_set = it->second;
    m_need_descriptor_update = false;
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

        m_buffer->get<uniform_data>(0).model = model;

        m_buffer->upload();
    }
}

void renderable::draw(tph::command_buffer& buffer)
{
    if(m_index_count > 0)
    {
        tph::cmd::bind_index_buffer(buffer, m_buffer->get_buffer(), m_buffer->compute_offset(1), tph::index_type::uint32);
        tph::cmd::bind_vertex_buffer(buffer, m_buffer->get_buffer(), m_buffer->compute_offset(2));
        tph::cmd::bind_descriptor_set(buffer, m_current_set->set(), m_current_set->pool().technique().pipeline_layout());
        tph::cmd::draw_indexed(buffer, m_index_count, 1, 0, 0, 0);
    }
    else
    {
        tph::cmd::bind_vertex_buffer(buffer, m_buffer->get_buffer(), m_buffer->compute_offset(1));
        tph::cmd::bind_descriptor_set(buffer, m_current_set->set(), m_current_set->pool().technique().pipeline_layout());
        tph::cmd::draw(buffer, m_vertex_count, 1, 0, 0);
    }
}

cpt::binding& renderable::add_binding(std::uint32_t index, cpt::binding binding)
{
    auto [it, success] = m_bindings.try_emplace(index, std::move(binding));
    assert(success && "cpt::view::add_binding called with already used binding.");

    m_need_descriptor_update = true;
    return it->second;
}

void renderable::set_uniform(std::uint32_t index, cpt::binding new_binding)
{
    m_bindings.at(index) = std::move(new_binding);
    m_need_descriptor_update = true;
}

sprite::sprite(std::uint32_t width, std::uint32_t height, const color& color)
:renderable{6, 4}
,m_width{width}
,m_height{height}
{
    init(color);
}

sprite::sprite(texture_ptr texture)
:renderable{6, 4}
,m_width{texture->width()}
,m_height{texture->height()}
{
    init(colors::white);
    set_texture(std::move(texture));
}

sprite::sprite(std::uint32_t width, std::uint32_t height, texture_ptr texture)
:renderable{6, 4}
,m_width{width}
,m_height{height}
{
    init(colors::white);
    set_texture(std::move(texture));
}

void sprite::set_color(const color& color) noexcept
{
    const auto vertices{get_vertices()};

    vertices[0].color = static_cast<glm::vec4>(color);
    vertices[1].color = static_cast<glm::vec4>(color);
    vertices[2].color = static_cast<glm::vec4>(color);
    vertices[3].color = static_cast<glm::vec4>(color);

    update();
}

void sprite::set_texture_coords(std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2) noexcept
{
    set_relative_texture_coords(static_cast<float>(x1) / texture()->width(),
                                static_cast<float>(y1) / texture()->height(),
                                static_cast<float>(x2) / texture()->width(),
                                static_cast<float>(y2) / texture()->height());
}

void sprite::set_texture_rect(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) noexcept
{
    set_texture_coords(x, y, x + width, y + height);
}

void sprite::set_relative_texture_coords(float x1, float y1, float x2, float y2) noexcept
{
    const auto vertices{get_vertices()};

    vertices[0].texture_coord = glm::vec2{x1, y1};
    vertices[1].texture_coord = glm::vec2{x2, y1};
    vertices[2].texture_coord = glm::vec2{x2, y2};
    vertices[3].texture_coord = glm::vec2{x1, y2};

    update();
}

void sprite::set_relative_texture_rect(float x, float y, float width, float height) noexcept
{
    set_relative_texture_coords(x, y, x + width, y + height);
}

void sprite::set_spritesheet_coords(std::uint32_t x, std::uint32_t y) noexcept
{
    set_texture_rect(x * m_width, y * m_height, m_width, m_height);
}

void sprite::resize(std::uint32_t width, std::uint32_t height) noexcept
{
    m_width = width;
    m_height = height;

    const auto vertices{get_vertices()};

    vertices[0].position = glm::vec3{0.0f, 0.0f, 0.0f};
    vertices[1].position = glm::vec3{static_cast<float>(m_width), 0.0f, 0.0f};
    vertices[2].position = glm::vec3{static_cast<float>(m_width), static_cast<float>(m_height), 0.0f};
    vertices[3].position = glm::vec3{0.0f, static_cast<float>(m_height), 0.0f};

    update();
}

void sprite::init(const color& color)
{
    set_indices(std::array<std::uint32_t, 6>{0, 1, 2, 2, 3, 0});
    resize(m_width, m_height);
    set_color(color);
    set_relative_texture_coords(0.0f, 0.0f, 1.0f, 1.0f);
}

polygon::polygon(std::vector<glm::vec2> points, const color& color)
:renderable{static_cast<std::uint32_t>(std::size(points) * 3), static_cast<std::uint32_t>(std::size(points) + 1)}
{
    assert(std::size(points) > 2 && "cpt::polygon created with less than 3 points.");

    init(std::move(points), color);
}

void polygon::set_color(const color& color) noexcept
{
    set_center_color(color);
    set_outline_color(color);
}

void polygon::set_center_color(const color& color) noexcept
{
    get_vertices()[0].color = static_cast<glm::vec4>(color);
    update();
}

void polygon::set_outline_color(const color& color) noexcept
{
    for(std::uint32_t i{}; i < std::size(m_points); ++i)
    {
        set_point_color(i, color);
    }
}

void polygon::set_point_color(std::uint32_t point, const color& color) noexcept
{
    get_vertices()[point + 1].color = static_cast<glm::vec4>(color);
    update();
}

void polygon::init(std::vector<glm::vec2> points, const color& color)
{
    m_points = std::move(points);

    const auto indices{get_indices()};
    for(std::uint32_t i{1}; i < std::size(m_points); ++i)
    {
        const auto current{indices.subspan((i - 1) * 3, 3)};

        current[0] = 0;
        current[1] = i;
        current[2] = i + 1;
    }

    const auto last_triangle{indices.subspan((std::size(m_points) - 1) * 3, 3)};
    last_triangle[0] = 0;
    last_triangle[1] = 1; //Loop on the first point
    last_triangle[2] = static_cast<std::uint32_t>(std::size(m_points));

    const glm::vec4 native_color{color};
    const auto vertices{get_vertices()};

    vertices[0].color = native_color;
    for(std::uint32_t i{}; i < std::size(m_points); ++i)
    {
        vertices[i + 1].position = glm::vec3{m_points[i], 0.0f};
        vertices[i + 1].color = native_color;
    }

    update();
}

tilemap::tilemap(std::uint32_t width, std::uint32_t height, std::uint32_t tile_width, std::uint32_t tile_height)
:renderable{width * height * 6, width * height * 4}
,m_width{width}
,m_height{height}
,m_tile_width{tile_width}
,m_tile_height{tile_height}
{
    init();
}

tilemap::tilemap(std::uint32_t width, std::uint32_t height, const tileset& tileset)
:renderable{width * height * 6, width * height * 4}
,m_width{width}
,m_height{height}
,m_tile_width{tileset.tile_width()}
,m_tile_height{tileset.tile_height()}
{
    init();
    set_texture(tileset.texture());
}

void tilemap::set_color(std::uint32_t row, std::uint32_t col, const color& color) noexcept
{
    const auto vertices{get_vertices().subspan((row * m_width + col) * 4, 4)};

    vertices[0].color = static_cast<glm::vec4>(color);
    vertices[1].color = static_cast<glm::vec4>(color);
    vertices[2].color = static_cast<glm::vec4>(color);
    vertices[3].color = static_cast<glm::vec4>(color);

    update();
}

void tilemap::set_texture_coords(std::uint32_t row, std::uint32_t col, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2) noexcept
{
    set_relative_texture_coords(row, col,
                                static_cast<float>(x1) / texture()->width(),
                                static_cast<float>(y1) / texture()->height(),
                                static_cast<float>(x2) / texture()->width(),
                                static_cast<float>(y2) / texture()->height());
}

void tilemap::set_texture_rect(std::uint32_t row, std::uint32_t col, std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) noexcept
{
    set_texture_coords(row, col, x, y, x + width, y + height);
}

void tilemap::set_texture_rect(std::uint32_t row, std::uint32_t col, const tileset::texture_rect& rect) noexcept
{
    const auto vertices{get_vertices().subspan((row * m_width + col) * 4, 4)};

    vertices[0].texture_coord = rect.top_left;
    vertices[1].texture_coord = rect.top_right;
    vertices[2].texture_coord = rect.bottom_right;
    vertices[3].texture_coord = rect.bottom_left;

    update();
}

void tilemap::set_relative_texture_coords(std::uint32_t row, std::uint32_t col, float x1, float y1, float x2, float y2) noexcept
{
    const auto vertices{get_vertices().subspan((row * m_width + col) * 4, 4)};

    vertices[0].texture_coord = glm::vec2{x1, y1};
    vertices[1].texture_coord = glm::vec2{x2, y1};
    vertices[2].texture_coord = glm::vec2{x2, y2};
    vertices[3].texture_coord = glm::vec2{x1, y2};

    update();
}

void tilemap::set_relative_texture_rect(std::uint32_t row, std::uint32_t col, float x, float y, float width, float height) noexcept
{
    set_relative_texture_coords(row, col, x, y, x + width, y + height);
}

void tilemap::init()
{
    const auto vertices{get_vertices()};
    const auto indices{get_indices()};

    for(std::uint32_t j{}; j < m_height; ++j)
    {
        for(std::uint32_t i{}; i < m_width; ++i)
        {
            const auto current_vertices{vertices.subspan((j * m_width + i) * 4, 4)};
            current_vertices[0].position = glm::vec3{i * m_tile_width, j * m_tile_height, 0.0f};
            current_vertices[1].position = glm::vec3{(i + 1) * m_tile_width, j * m_tile_height, 0.0f};
            current_vertices[2].position = glm::vec3{(i + 1) * m_tile_width, (j + 1) * m_tile_height, 0.0f};
            current_vertices[3].position = glm::vec3{i * m_tile_width, (j + 1) * m_tile_height, 0.0f};
            current_vertices[0].color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
            current_vertices[1].color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
            current_vertices[2].color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
            current_vertices[3].color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};

            const auto shift{(j * m_width + i) * 4};
            const auto current_indices{indices.subspan((j * m_width + i) * 6, 6)};
            current_indices[0] = shift + 0;
            current_indices[1] = shift + 1;
            current_indices[2] = shift + 2;
            current_indices[3] = shift + 2;
            current_indices[4] = shift + 3;
            current_indices[5] = shift + 0;
        }
    }
}

}
