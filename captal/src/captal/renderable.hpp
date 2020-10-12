#ifndef CAPTAL_RENDERABLE_HPP_INCLUDED
#define CAPTAL_RENDERABLE_HPP_INCLUDED

#include "config.hpp"

#include <unordered_map>
#include <span>
#include <numbers>

#include "asynchronous_resource.hpp"
#include "uniform_buffer.hpp"
#include "binding.hpp"
#include "color.hpp"
#include "view.hpp"
#include "vertex.hpp"
#include "texture.hpp"

namespace cpt
{

class CAPTAL_API renderable
{
public:
    struct uniform_data
    {
        mat4f model{};
    };

public:
    renderable() = default;
    explicit renderable(std::uint32_t vertex_count);
    explicit renderable(std::uint32_t index_count, std::uint32_t vertex_count);

    virtual ~renderable() = default;
    renderable(const renderable&) = delete;
    renderable& operator=(const renderable&) = delete;
    renderable(renderable&&) noexcept = default;
    renderable& operator=(renderable&&) noexcept = default;

    void set_indices(std::span<const std::uint32_t> indices) noexcept;
    void set_vertices(std::span<const vertex> vertices) noexcept;
    void set_texture(texture_ptr texture) noexcept;
    void set_view(cpt::view& view);

    void move(const vec3f& relative) noexcept
    {
        m_position += relative;
        update();
    }

    void move_to(const vec3f& position) noexcept
    {
        m_position = position;
        update();
    }

    void set_origin(const vec3f& origin) noexcept
    {
        m_origin = origin;
        update();
    }

    void move_origin(const vec3f& relative) noexcept
    {
        m_origin += relative;
        update();
    }

    void rotate(float angle) noexcept
    {
        m_rotation = std::fmod(m_rotation + angle, std::numbers::pi_v<float> * 2.0f);
        update();
    }

    void set_rotation(float angle) noexcept
    {
        m_rotation = std::fmod(angle, std::numbers::pi_v<float> * 2.0f);
        update();
    }

    void scale(const vec3f& scale) noexcept
    {
        m_scale *= scale;
        update();
    }

    void set_scale(const vec3f& scale) noexcept
    {
        m_scale = scale;
        update();
    }

    void hide() noexcept
    {
        m_hidden = true;
    }

    void show() noexcept
    {
        m_hidden = false;
    }

    void update() noexcept
    {
        m_need_upload = true;
    }

    void upload();
    void draw(tph::command_buffer& buffer);

    cpt::binding& add_binding(std::uint32_t index, cpt::binding binding);
    void set_binding(std::uint32_t index, cpt::binding new_binding);

    const vec3f& position() const noexcept
    {
        return m_position;
    }

    const vec3f& origin() const noexcept
    {
        return m_origin;
    }

    const vec3f& scale() const noexcept
    {
        return m_scale;
    }

    float rotation() const noexcept
    {
        return m_rotation;
    }

    bool hidden() const noexcept
    {
        return m_hidden;
    }

    std::span<std::uint32_t> get_indices() noexcept
    {
        assert(m_index_count > 0 && "cpt::renderable::get_indices called on renderable with no index buffer");

        return std::span{&m_buffer->get<std::uint32_t>(1), static_cast<std::size_t>(m_index_count)};
    }

    std::span<const std::uint32_t> get_indices() const noexcept
    {
        assert(m_index_count > 0 && "cpt::renderable::get_indices called on renderable with no index buffer");

        return std::span{&m_buffer->get<std::uint32_t>(1), static_cast<std::size_t>(m_index_count)};
    }

    std::span<vertex> get_vertices() noexcept
    {
        return std::span{&m_buffer->get<vertex>(m_index_count > 0 ? 2 : 1), static_cast<std::size_t>(m_vertex_count)};
    }

    std::span<const vertex> get_vertices() const noexcept
    {
        return std::span{&m_buffer->get<vertex>(m_index_count > 0 ? 2 : 1), static_cast<std::size_t>(m_vertex_count)};
    }

    const descriptor_set_ptr& set() const noexcept
    {
        return m_current_set;
    }

    const descriptor_set_ptr& set(const cpt::view& view) const noexcept
    {
        return m_descriptor_sets.at(view.resource().get());
    }

    const texture_ptr& texture() const noexcept
    {
        return m_texture;
    }

    const cpt::binding& binding(std::uint32_t index) const
    {
        return m_bindings.at(index);
    }

    bool has_binding(std::uint32_t index) const
    {
        return m_bindings.find(index) != std::end(m_bindings);
    }

    const std::unordered_map<std::uint32_t, cpt::binding>& bindings() const noexcept
    {
        return m_bindings;
    }

    asynchronous_resource_ptr resource() const noexcept
    {
        return m_buffer;
    }

private:
    uniform_buffer_ptr m_buffer{};
    texture_ptr m_texture{};
    std::unordered_map<std::uint32_t, cpt::binding> m_bindings{};
    std::unordered_map<const void*, descriptor_set_ptr> m_descriptor_sets{};
    descriptor_set_ptr m_current_set{};

    std::uint32_t m_index_count{};
    std::uint32_t m_vertex_count{};
    std::uint64_t m_index_offset{};
    std::uint64_t m_vertex_offset{};

    vec3f m_position{};
    vec3f m_origin{};
    vec3f m_scale{1.0f};
    float m_rotation{};

    bool m_hidden{};
    bool m_need_upload{true};
    bool m_need_descriptor_update{};
};

class CAPTAL_API sprite final : public renderable
{
public:
    sprite() = default;
    explicit sprite(std::uint32_t width, std::uint32_t height, const color& color = colors::white);
    explicit sprite(texture_ptr texture);
    explicit sprite(std::uint32_t width, std::uint32_t height, texture_ptr texture);

    ~sprite() = default;
    sprite(const sprite&) = delete;
    sprite& operator=(const sprite&) = delete;
    sprite(sprite&&) noexcept = default;
    sprite& operator=(sprite&&) noexcept = default;

    void set_color(const color& color) noexcept;

    void set_texture_coords(std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2) noexcept;
    void set_texture_rect(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) noexcept;
    void set_relative_texture_coords(float x1, float y1, float x2, float y2) noexcept;
    void set_relative_texture_rect(float x, float y, float width, float height) noexcept;
    void set_spritesheet_coords(std::uint32_t x, std::uint32_t y) noexcept;

    void resize(std::uint32_t width, std::uint32_t height) noexcept;

    std::uint32_t width() const noexcept
    {
        return m_width;
    }

    std::uint32_t height() const noexcept
    {
        return m_height;
    }

private:
    void init(const color& color);

private:
    std::uint32_t m_width{};
    std::uint32_t m_height{};
};

class CAPTAL_API polygon final : public renderable
{
public:
    polygon() = default;
    explicit polygon(std::vector<vec2f> points, const color& color = colors::white);

    ~polygon() = default;
    polygon(const polygon&) = delete;
    polygon& operator=(const polygon&) = delete;
    polygon(polygon&&) noexcept = default;
    polygon& operator=(polygon&&) noexcept = default;

    void set_color(const color& color) noexcept;
    void set_center_color(const color& color) noexcept;
    void set_outline_color(const color& color) noexcept;
    void set_point_color(std::uint32_t point, const color& color) noexcept;

    void set_texture_coords(std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2) noexcept;
    void set_texture_rect(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) noexcept;

    void set_relative_texture_coords(float x1, float y1, float x2, float y2) noexcept;
    void set_relative_texture_rect(float x, float y, float width, float height) noexcept;

    std::span<const vec2f> points() const noexcept
    {
        return m_points;
    }

private:
    void init(std::vector<vec2f> points, const color& color);

private:
    std::vector<vec2f> m_points{};
};

class CAPTAL_API tilemap final : public renderable
{
public:
    tilemap() = default;
    explicit tilemap(std::uint32_t width, std::uint32_t height, std::uint32_t tile_width, std::uint32_t tile_height);
    explicit tilemap(std::uint32_t width, std::uint32_t height, const tileset& tileset);

    ~tilemap() = default;
    tilemap(const tilemap&) = delete;
    tilemap& operator=(const tilemap&) = delete;
    tilemap(tilemap&&) noexcept = default;
    tilemap& operator=(tilemap&&) noexcept = default;

    void set_color(std::uint32_t row, std::uint32_t col, const color& color) noexcept;

    void set_texture_coords(std::uint32_t row, std::uint32_t col, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2) noexcept;
    void set_texture_rect(std::uint32_t row, std::uint32_t col, std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) noexcept;
    void set_texture_rect(std::uint32_t row, std::uint32_t col, const tileset::texture_rect& rect) noexcept;

    void set_relative_texture_coords(std::uint32_t row, std::uint32_t col, float x1, float y1, float x2, float y2) noexcept;
    void set_relative_texture_rect(std::uint32_t row, std::uint32_t col, float x, float y, float width, float height) noexcept;

    std::uint32_t width() const noexcept
    {
        return m_width;
    }

    std::uint32_t height() const noexcept
    {
        return m_height;
    }

    std::uint32_t tile_width() const noexcept
    {
        return m_width;
    }

    std::uint32_t tile_height() const noexcept
    {
        return m_height;
    }

private:
    void init();

private:
    std::uint32_t m_width{};
    std::uint32_t m_height{};
    std::uint32_t m_tile_width{};
    std::uint32_t m_tile_height{};
};

}

#endif
