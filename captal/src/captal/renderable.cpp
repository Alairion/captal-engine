#include "renderable.hpp"

#include <cassert>

#include <tephra/commands.hpp>

#include "engine.hpp"

namespace cpt
{

static std::array<buffer_part, 2> compute_buffer_parts(std::uint32_t vertex_count)
{
    return std::array<buffer_part, 2>
    {
        buffer_part{buffer_part_type::uniform, sizeof(basic_renderable::uniform_data)},
        buffer_part{buffer_part_type::vertex, vertex_count * sizeof(vertex)},
    };
}

static std::array<buffer_part, 3> compute_buffer_parts(std::uint32_t vertex_count, std::uint32_t index_count)
{
    return std::array<buffer_part, 3>
    {
        buffer_part{buffer_part_type::uniform, sizeof(basic_renderable::uniform_data)},
        buffer_part{buffer_part_type::vertex, vertex_count * sizeof(vertex)},
        buffer_part{buffer_part_type::index, index_count * sizeof(std::uint32_t)},
    };
}

basic_renderable::basic_renderable(std::uint32_t vertex_count, std::uint32_t uniform_index)
:m_vertex_count{vertex_count}
,m_uniform_index{uniform_index}
{
    auto buffer{make_uniform_buffer(compute_buffer_parts(vertex_count))};
    m_buffer = buffer.get();

    m_bindings.set(m_uniform_index, std::move(buffer));
}

basic_renderable::basic_renderable(std::uint32_t vertex_count, std::uint32_t index_count, std::uint32_t uniform_index)
:m_vertex_count{vertex_count}
,m_index_count{index_count}
,m_uniform_index{uniform_index}
{
    auto buffer{make_uniform_buffer(compute_buffer_parts(vertex_count, index_count))};
    m_buffer = buffer.get();

    m_bindings.set(m_uniform_index, std::move(buffer));
}

void basic_renderable::set_vertices(std::span<const vertex> vertices) noexcept
{
    assert(std::size(vertices) == m_vertex_count && "cpt::basic_renderable::set_vertices called with a wrong number of vertices.");

    std::memcpy(&m_buffer->get<vertex>(1), std::data(vertices), std::size(vertices) * sizeof(vertex));

    m_upload_vertices = true;
}

void basic_renderable::set_indices(std::span<const std::uint32_t> indices) noexcept
{
    assert(m_index_count > 0 && "cpt::basic_renderable::set_indices called on a basic_renderable without index buffer.");
    assert(std::size(indices) == m_index_count && "cpt::basic_renderable::set_indices called with a wrong number of indices.");

    std::memcpy(&m_buffer->get<std::uint32_t>(2), std::data(indices), std::size(indices) * sizeof(std::uint32_t));

    m_upload_indices = true;
}

void basic_renderable::reset(std::uint32_t vertex_count)
{
    auto buffer{make_uniform_buffer(compute_buffer_parts(vertex_count))};

    m_buffer = buffer.get();
    m_vertex_count = vertex_count;
    m_upload_model = true;

    m_bindings.set(m_uniform_index, std::move(buffer));
}

void basic_renderable::reset(std::uint32_t vertex_count, std::uint32_t index_count)
{
    auto buffer{make_uniform_buffer(compute_buffer_parts(vertex_count, index_count))};

    m_buffer = buffer.get();
    m_vertex_count = vertex_count;
    m_index_count = index_count;
    m_upload_model = true;

    m_bindings.set(m_uniform_index, std::move(buffer));
}

void basic_renderable::bind(frame_render_info info, cpt::view& view)
{
    const auto& layout{view.render_technique()->layout()};

    const auto write_set = [this, &layout](descriptor_set_data& data)
    {
        const auto to_bind{layout->bindings(render_layout::renderable_index)};

        std::vector<tph::descriptor_write> writes{};
        writes.reserve(std::size(to_bind));

        for(auto&& binding : to_bind)
        {
            const auto local{m_bindings.try_get(binding.binding)};

            if(local)
            {
                writes.emplace_back(make_descriptor_write(data.set->set(), binding.binding, *local));
                data.to_keep.emplace_back(get_binding_resource(*local));
            }
            else
            {
                const auto fallback{layout->default_binding(render_layout::renderable_index, binding.binding)};
                assert(fallback && "cpt::basic_renderable::bind can not find any suitable binding, neither the renderable nor the render layout have a binding for specified index.");

                writes.emplace_back(make_descriptor_write(data.set->set(), binding.binding, *fallback));
                data.to_keep.emplace_back(get_binding_resource(*fallback));
            }
        }

        tph::write_descriptors(engine::instance().renderer(), writes);
    };

    auto it{m_sets.find(layout)};

    if(it == std::end(m_sets)) //New layout
    {
        it = m_sets.emplace(layout, descriptor_set_data{layout->make_set(render_layout::renderable_index), std::vector<asynchronous_resource_ptr>{}, m_descriptors_epoch}).first;

        write_set(it->second);
    }
    else if(it->second.epoch < m_descriptors_epoch) //Already known layout but not up to date
    {
        it->second.set.reset();
        it->second.set = layout->make_set(1);
        it->second.to_keep.clear();
        it->second.epoch = m_descriptors_epoch;

        write_set(it->second);
    }

    auto buffer{m_buffer->get_buffer()};

    if(m_index_count > 0)
    {
        tph::cmd::bind_index_buffer(info.buffer, buffer.buffer, buffer.offset + m_buffer->part_offset(2), tph::index_type::uint32);
    }

    tph::cmd::bind_vertex_buffer (info.buffer, buffer.buffer, buffer.offset + m_buffer->part_offset(1));
    tph::cmd::bind_descriptor_set(info.buffer, 1, it->second.set->set(), layout->pipeline_layout());

    m_push_constants.push(info.buffer, layout, render_layout::renderable_index);

    info.keeper.keep(std::begin(it->second.to_keep), std::end(it->second.to_keep));
    info.keeper.keep(it->second.set);
}

void basic_renderable::draw(frame_render_info info)
{
    if(m_index_count > 0)
    {
        tph::cmd::draw_indexed(info.buffer, m_index_count, 1, 0, 0, 0);
    }
    else
    {
        tph::cmd::draw(info.buffer, m_vertex_count, 1, 0, 0);
    }
}

void basic_renderable::draw(frame_render_info info, cpt::view& view)
{
    bind(info, view);
    draw(info);
}

void basic_renderable::upload(memory_transfer_info info)
{
    bool keep{};

    if(std::exchange(m_upload_model, false))
    {
        m_buffer->get<uniform_data>(0).model = cpt::model(m_position, m_rotation, vec3f{0.0f, 0.0f, 1.0f}, m_scale, m_origin);
        m_buffer->upload(0);

        keep = true;
    }

    if(std::exchange(m_upload_vertices, false))
    {
        m_buffer->upload(1);

        keep = true;
    }

    if(std::exchange(m_upload_indices, false))
    {
        m_buffer->upload(2);

        keep = true;
    }

    if(keep)
    {
        info.keeper.keep(get_binding_resource(m_bindings.get(m_uniform_index)));
    }
}

void basic_renderable::set_binding(std::uint32_t index, cpt::binding binding)
{
    assert(index != m_uniform_index && "cpt::basic_renderable::set_binding must never be called with index == uniform_index.");

    m_bindings.set(index, std::move(binding));
    ++m_descriptors_epoch;
}

sprite::sprite(std::uint32_t width, std::uint32_t height, const color& color)
:basic_renderable{4, 6, 0}
,m_width{width}
,m_height{height}
{
    init(color);
}

sprite::sprite(texture_ptr texture, const color& color)
:basic_renderable{4, 6, 0}
,m_width{texture->width()}
,m_height{texture->height()}
{
    init(color);
    set_texture(std::move(texture));
}

sprite::sprite(std::uint32_t width, std::uint32_t height, texture_ptr texture, const color& color)
:basic_renderable{4, 6, 0}
,m_width{width}
,m_height{height}
{
    init(color);
    set_texture(std::move(texture));
}

void sprite::set_texture(texture_ptr texture)
{
    set_binding(1, std::move(texture));
}

void sprite::set_color(const color& color) noexcept
{
    const auto vertices{basic_renderable::vertices()};

    vertices[0].color = static_cast<vec4f>(color);
    vertices[1].color = static_cast<vec4f>(color);
    vertices[2].color = static_cast<vec4f>(color);
    vertices[3].color = static_cast<vec4f>(color);
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
    const auto vertices{basic_renderable::vertices()};

    vertices[0].texture_coord = vec2f{x1, y1};
    vertices[1].texture_coord = vec2f{x2, y1};
    vertices[2].texture_coord = vec2f{x2, y2};
    vertices[3].texture_coord = vec2f{x1, y2};
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

    const auto vertices{basic_renderable::vertices()};

    vertices[0].position = vec3f{0.0f, 0.0f, 0.0f};
    vertices[1].position = vec3f{static_cast<float>(width), 0.0f, 0.0f};
    vertices[2].position = vec3f{static_cast<float>(width), static_cast<float>(height), 0.0f};
    vertices[3].position = vec3f{0.0f, static_cast<float>(height), 0.0f};
}

void sprite::init(const color& color)
{
    set_indices(std::array<std::uint32_t, 6>{0, 1, 2, 2, 3, 0});
    resize(m_width, m_height);
    set_color(color);
    set_relative_texture_coords(0.0f, 0.0f, 1.0f, 1.0f);
}

polygon::polygon(std::vector<vec2f> points, const color& color)
:basic_renderable{static_cast<std::uint32_t>(std::size(points) + 1), static_cast<std::uint32_t>(std::size(points) * 3), 0}
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
    vertices()[0].color = static_cast<vec4f>(color);
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
    vertices()[point + 1].color = static_cast<vec4f>(color);
}

void polygon::init(std::vector<vec2f> points, const color& color)
{
    m_points = std::move(points);

    const auto indices{basic_renderable::indices()};
    for(std::uint32_t i{1}; i < std::size(m_points); ++i)
    {
        const auto current{indices.subspan((i - 1) * 3)};

        current[0] = 0;
        current[1] = i;
        current[2] = i + 1;
    }

    const auto last_triangle{indices.subspan((std::size(m_points) - 1) * 3)};
    last_triangle[0] = 0;
    last_triangle[1] = 1; //Loop on the first point
    last_triangle[2] = static_cast<std::uint32_t>(std::size(m_points));

    const vec4f native_color{color};
    const auto  vertices    {basic_renderable::vertices()};

    vertices[0].color = native_color;
    for(std::uint32_t i{}; i < std::size(m_points); ++i)
    {
        vertices[i + 1].position = vec3f{m_points[i], 0.0f};
        vertices[i + 1].color = native_color;
    }
}

tilemap::tilemap(std::uint32_t width, std::uint32_t height, std::uint32_t tile_width, std::uint32_t tile_height)
:basic_renderable{width * height * 4, width * height * 6, 0}
,m_width{width}
,m_height{height}
,m_tile_width{tile_width}
,m_tile_height{tile_height}
{
    init();
}

tilemap::tilemap(std::uint32_t width, std::uint32_t height, const tileset& tileset)
:basic_renderable{width * height * 4, width * height * 6, 0}
,m_width{width}
,m_height{height}
,m_tile_width{tileset.tile_width()}
,m_tile_height{tileset.tile_height()}
{
    init();
    set_texture(tileset.texture());
}

void tilemap::set_texture(texture_ptr texture)
{
    set_binding(1, std::move(texture));
}

void tilemap::set_color(std::uint32_t row, std::uint32_t col, const color& color) noexcept
{
    const auto vertices{basic_renderable::vertices().subspan((row * m_width + col) * 4)};

    vertices[0].color = static_cast<vec4f>(color);
    vertices[1].color = static_cast<vec4f>(color);
    vertices[2].color = static_cast<vec4f>(color);
    vertices[3].color = static_cast<vec4f>(color);
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
    const auto vertices{basic_renderable::vertices().subspan((row * m_width + col) * 4)};

    vertices[0].texture_coord = rect.top_left;
    vertices[1].texture_coord = vec2f{rect.bottom_right.x(), rect.top_left.y()};
    vertices[2].texture_coord = rect.bottom_right;
    vertices[3].texture_coord = vec2f{rect.top_left.x(), rect.bottom_right.y()};
}

void tilemap::set_relative_texture_coords(std::uint32_t row, std::uint32_t col, float x1, float y1, float x2, float y2) noexcept
{
    const auto vertices{basic_renderable::vertices().subspan((row * m_width + col) * 4)};

    vertices[0].texture_coord = vec2f{x1, y1};
    vertices[1].texture_coord = vec2f{x2, y1};
    vertices[2].texture_coord = vec2f{x2, y2};
    vertices[3].texture_coord = vec2f{x1, y2};
}

void tilemap::set_relative_texture_rect(std::uint32_t row, std::uint32_t col, float x, float y, float width, float height) noexcept
{
    set_relative_texture_coords(row, col, x, y, x + width, y + height);
}

void tilemap::init()
{
    const auto vertices{basic_renderable::vertices()};
    const auto indices {basic_renderable::indices()};

    for(std::uint32_t j{}; j < m_height; ++j)
    {
        for(std::uint32_t i{}; i < m_width; ++i)
        {
            const auto current_vertices{vertices.subspan((j * m_width + i) * 4)};
            current_vertices[0].position = vec3f{static_cast<float>(i * m_tile_width), static_cast<float>(j * m_tile_height), 0.0f};
            current_vertices[1].position = vec3f{static_cast<float>((i + 1) * m_tile_width), static_cast<float>(j * m_tile_height), 0.0f};
            current_vertices[2].position = vec3f{static_cast<float>((i + 1) * m_tile_width), static_cast<float>((j + 1) * m_tile_height), 0.0f};
            current_vertices[3].position = vec3f{static_cast<float>(i * m_tile_width), static_cast<float>((j + 1) * m_tile_height), 0.0f};
            current_vertices[0].color = vec4f{1.0f, 1.0f, 1.0f, 1.0f};
            current_vertices[1].color = vec4f{1.0f, 1.0f, 1.0f, 1.0f};
            current_vertices[2].color = vec4f{1.0f, 1.0f, 1.0f, 1.0f};
            current_vertices[3].color = vec4f{1.0f, 1.0f, 1.0f, 1.0f};

            const auto shift{(j * m_width + i) * 4};
            const auto current_indices{indices.subspan((j * m_width + i) * 6)};
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
