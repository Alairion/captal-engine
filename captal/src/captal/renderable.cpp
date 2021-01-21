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

basic_renderable::basic_renderable(std::uint32_t vertex_count)
:m_vertex_count{vertex_count}
{
    auto buffer{make_uniform_buffer(compute_buffer_parts(vertex_count))};
    m_buffer = buffer.get();

    m_bindings.emplace(0, std::move(buffer));
}

basic_renderable::basic_renderable(std::uint32_t vertex_count, std::uint32_t index_count)
:m_vertex_count{vertex_count}
,m_index_count{index_count}
{
    auto buffer{make_uniform_buffer(compute_buffer_parts(vertex_count, index_count))};
    m_buffer = buffer.get();

    m_bindings.emplace(0, std::move(buffer));
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

void basic_renderable::bind_view(cpt::view& view)
{
    const auto write_set = [this](cpt::view& view, const descriptor_set_ptr& set)
    {
        const auto has_binding = [](cpt::view& view, std::uint32_t binding)
        {
            for(auto&& layout_binding : view.render_technique()->bindings())
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
                auto buffer{std::get<uniform_buffer_ptr>(data)->get_buffer()};
                const tph::descriptor_buffer_info info{buffer.buffer, buffer.offset, std::get<uniform_buffer_ptr>(data)->size()};

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

        auto  buffer {m_buffer->get_buffer()};
        auto& texture{m_texture->get_texture()};

        std::vector<tph::descriptor_write> writes{};
        writes.reserve(3 + std::size(m_bindings) + std::size(view.bindings()));

        writes.emplace_back(set->set(), 0, 0, tph::descriptor_type::uniform_buffer, tph::descriptor_buffer_info {view.get_buffer(), 0, sizeof(view::uniform_data)});
        writes.emplace_back(set->set(), 1, 0, tph::descriptor_type::uniform_buffer, tph::descriptor_buffer_info {buffer.buffer, buffer.offset, sizeof(uniform_data)});
        writes.emplace_back(set->set(), 2, 0, tph::descriptor_type::image_sampler,  tph::descriptor_texture_info{texture, tph::texture_layout::shader_read_only_optimal});

        for(auto&& [binding, data] : m_bindings)
        {
            if(has_binding(view, binding))
            {
                writes.emplace_back(write(set, binding, data));
            }
        }

        for(auto&& [binding, data] : view.m_bindings)
        {
            if(has_binding(view, binding))
            {
                writes.emplace_back(write(set, binding, data));
            }
        }

        tph::write_descriptors(engine::instance().renderer(), writes);
    };

    auto it{m_descriptor_sets.find(view.resource().get())};

    if(it == std::end(m_descriptor_sets)) //New view
    {
        const auto [new_item, success] = m_descriptor_sets.emplace(view.resource().get(), view.render_technique()->make_set());

        it = new_item;
        write_set(view, it->second);
    }
    else if(m_need_descriptor_update || view.need_descriptor_update()) //Already known view check
    {
        it->second = view.render_technique()->make_set();
        write_set(view, it->second);
    }

    m_current_set = it->second;
    m_need_descriptor_update = false;
}

/* this is a cool effect :)
template<arithmetic T>
mat<T, 4, 4> rotate_and_scale(const vec<T, 3>& translation, T angle, const vec<T, 3>& axis, const vec<T, 3>& factor, const vec<T, 3>& origin)
{
    return translate(translation - origin) * (rotate(angle, axis) + translate(origin)) * scale(factor);
}*/

void basic_renderable::upload(memory_transfer_info& info)
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
        info.keeper.keep(m_buffer);
    }
}

void basic_renderable::draw(tph::command_buffer& command_buffer)
{
    const auto buffer{m_buffer->get_buffer()};

    if(m_index_count > 0)
    {
        tph::cmd::bind_vertex_buffer (command_buffer, buffer.buffer, buffer.offset + m_buffer->part_offset(1));
        tph::cmd::bind_index_buffer  (command_buffer, buffer.buffer, buffer.offset + m_buffer->part_offset(2), tph::index_type::uint32);
        tph::cmd::bind_descriptor_set(command_buffer, 1, m_set->set(), m_set->pool().technique().pipeline_layout());

        tph::cmd::draw_indexed(command_buffer, m_index_count, 1, 0, 0, 0);
    }
    else
    {
        tph::cmd::bind_vertex_buffer (command_buffer, buffer.buffer, buffer.offset + m_buffer->part_offset(1));
        tph::cmd::bind_descriptor_set(command_buffer, 1, m_set->set(), m_set->pool().technique().pipeline_layout());

        tph::cmd::draw(command_buffer, m_vertex_count, 1, 0, 0);
    }
}

void basic_renderable::keep(asynchronous_resource_keeper& keeper)
{
    for(const auto& [index, binding] : m_bindings)
    {
        keeper.keep(get_binding_resource(binding));
    }
}

cpt::binding& basic_renderable::add_binding(std::uint32_t index, cpt::binding binding)
{
    auto [it, success] = m_bindings.try_emplace(index, std::move(binding));
    assert(success && "cpt::view::add_binding called with already used binding.");

    m_need_descriptor_update = true;

    return it->second;
}

void basic_renderable::set_binding(std::uint32_t index, cpt::binding new_binding)
{
    m_bindings.at(index) = std::move(new_binding);
    m_need_descriptor_update = true;
}

sprite::sprite(std::uint32_t width, std::uint32_t height, const color& color)
:renderable{4, 6}
,m_width{width}
,m_height{height}
{
    init(color);
}

sprite::sprite(texture_ptr texture)
:renderable{4, 6}
,m_width{texture->width()}
,m_height{texture->height()}
{
    init(colors::white);
    set_texture(std::move(texture));
}

sprite::sprite(std::uint32_t width, std::uint32_t height, texture_ptr texture)
:renderable{4, 6}
,m_width{width}
,m_height{height}
{
    init(colors::white);
    set_texture(std::move(texture));
}

void sprite::set_texture(texture_ptr texture) noexcept
{
    m_texture = std::move(texture);
    m_need_descriptor_update = true;
}

void sprite::set_color(const color& color) noexcept
{
    const auto vertices{renderable::vertices()};

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
    const auto vertices{renderable::vertices()};

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

    const auto vertices{renderable::vertices()};

    vertices[0].position = vec3f{0.0f, 0.0f, 0.0f};
    vertices[1].position = vec3f{static_cast<float>(m_width), 0.0f, 0.0f};
    vertices[2].position = vec3f{static_cast<float>(m_width), static_cast<float>(m_height), 0.0f};
    vertices[3].position = vec3f{0.0f, static_cast<float>(m_height), 0.0f};
}

void sprite::init(const color& color)
{
    set_indices(std::array<std::uint32_t, 6>{0, 1, 2, 2, 3, 0});
    resize(m_width, m_height);
    set_color(color);
    set_relative_texture_coords(0.0f, 0.0f, 1.0f, 1.0f);
}

polygon::polygon(std::vector<vec2f> points, const color& color)
:renderable{static_cast<std::uint32_t>(std::size(points) + 1), static_cast<std::uint32_t>(std::size(points) * 3)}
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

    const auto indices{renderable::indices()};
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
    const auto  vertices    {renderable::vertices()};

    vertices[0].color = native_color;
    for(std::uint32_t i{}; i < std::size(m_points); ++i)
    {
        vertices[i + 1].position = vec3f{m_points[i], 0.0f};
        vertices[i + 1].color = native_color;
    }
}

tilemap::tilemap(std::uint32_t width, std::uint32_t height, std::uint32_t tile_width, std::uint32_t tile_height)
:renderable{width * height * 4, width * height * 6}
,m_width{width}
,m_height{height}
,m_tile_width{tile_width}
,m_tile_height{tile_height}
{
    init();
}

tilemap::tilemap(std::uint32_t width, std::uint32_t height, const tileset& tileset)
:renderable{width * height * 4, width * height * 6}
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
    const auto vertices{renderable::vertices().subspan((row * m_width + col) * 4)};

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
    const auto vertices{renderable::vertices().subspan((row * m_width + col) * 4)};

    vertices[0].texture_coord = rect.top_left;
    vertices[1].texture_coord = vec2f{rect.bottom_right.x(), rect.top_left.y()};
    vertices[2].texture_coord = rect.bottom_right;
    vertices[3].texture_coord = vec2f{rect.top_left.x(), rect.bottom_right.y()};
}

void tilemap::set_relative_texture_coords(std::uint32_t row, std::uint32_t col, float x1, float y1, float x2, float y2) noexcept
{
    const auto vertices{renderable::vertices().subspan((row * m_width + col) * 4)};

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
    const auto vertices{renderable::vertices()};
    const auto indices {renderable::indices()};

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
