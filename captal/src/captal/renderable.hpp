#ifndef CAPTAL_RENDERABLE_HPP_INCLUDED
#define CAPTAL_RENDERABLE_HPP_INCLUDED

#include "config.hpp"

#include <unordered_map>
#include <map>
#include <span>
#include <concepts>
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

template<typename T>
concept renderable = requires(T r, const T cr,
                              cpt::view view, const cpt::view cview,
                              std::uint32_t i, vec3f vec, float f, cpt::binding bind, tph::shader_stage stages,
                              memory_transfer_info info, tph::command_buffer cb, asynchronous_resource_keeper keeper)
{
    r.bind(cb, view);
    r.draw(cb);
    r.upload(info);
    r.keep(keeper);

    r.set_binding(i, bind);

    r.set_push_constant(stages, i, f);

    r.move(vec);
    r.move_to(vec);
    r.set_origin(vec);
    r.move_origin(vec);
    r.rotate(f);
    r.set_rotation(f);
    r.scale(vec);
    r.set_scale(vec);
    r.hide();
    r.show();

    {cr.binding(i)}         -> std::convertible_to<const cpt::binding&>;
    {cr.try_get_binding(i)} -> std::convertible_to<optional_ref<const cpt::binding>>;
    {cr.has_binding(i)}     -> std::convertible_to<bool>;

    {cr. template get_push_constant<float>(stages, i)}     -> std::convertible_to<const float&>;
    {cr. template try_get_push_constant<float>(stages, i)} -> std::convertible_to<optional_ref<const float>>;
    {cr.has_push_constant(stages, i)}                      -> std::convertible_to<bool>;

    {cr.position()} -> std::convertible_to<const vec3f&>;
    {cr.origin()}   -> std::convertible_to<const vec3f&>;
    {cr.scale()}    -> std::convertible_to<const vec3f&>;
    {cr.rotation()} -> std::convertible_to<float>;
    {cr.hidden()}   -> std::convertible_to<bool>;

    {r.vertices()}   -> std::convertible_to<std::span<vertex>>;
    {cr.vertices()}  -> std::convertible_to<std::span<const vertex>>;
    {cr.cvertices()} -> std::convertible_to<std::span<const vertex>>;

    {r.indices()}   -> std::convertible_to<std::span<std::uint32_t>>;
    {cr.indices()}  -> std::convertible_to<std::span<const std::uint32_t>>;
    {cr.cindices()} -> std::convertible_to<std::span<const std::uint32_t>>;
};

class CAPTAL_API basic_renderable
{
public:
    struct uniform_data
    {
        mat4f model{};
    };

protected:
    basic_renderable() = default;
    explicit basic_renderable(std::uint32_t vertex_count);
    explicit basic_renderable(std::uint32_t vertex_count, std::uint32_t index_count);

    ~basic_renderable() = default;
    basic_renderable(const basic_renderable&) = delete;
    basic_renderable& operator=(const basic_renderable&) = delete;
    basic_renderable(basic_renderable&&) noexcept = default;
    basic_renderable& operator=(basic_renderable&&) noexcept = default;

    void set_vertices(std::span<const vertex> vertices) noexcept;
    void set_indices(std::span<const std::uint32_t> indices) noexcept;

public:
    void bind(tph::command_buffer& command_buffer, cpt::view& view);
    void draw(tph::command_buffer& command_buffer);
    void upload(memory_transfer_info& info);
    void keep(asynchronous_resource_keeper& keeper);

    void set_binding(std::uint32_t index, cpt::binding binding);

    template<typename T>
    void set_push_constant(tph::shader_stage stages, std::uint32_t offset, T&& value)
    {
        return m_push_constants.set(stages, offset, std::forward<T>(value));
    }

    void move(const vec3f& relative) noexcept
    {
        m_position += relative;
        m_upload_model = true;
    }

    void move_to(const vec3f& position) noexcept
    {
        m_position = position;
        m_upload_model = true;
    }

    void set_origin(const vec3f& origin) noexcept
    {
        m_origin = origin;
        m_upload_model = true;
    }

    void move_origin(const vec3f& relative) noexcept
    {
        m_origin += relative;
        m_upload_model = true;
    }

    void rotate(float angle) noexcept
    {
        m_rotation = std::fmod(m_rotation + angle, std::numbers::pi_v<float> * 2.0f);
        m_upload_model = true;
    }

    void set_rotation(float angle) noexcept
    {
        m_rotation = std::fmod(angle, std::numbers::pi_v<float> * 2.0f);
        m_upload_model = true;
    }

    void scale(const vec3f& scale) noexcept
    {
        m_scale *= scale;
        m_upload_model = true;
    }

    void set_scale(const vec3f& scale) noexcept
    {
        m_scale = scale;
        m_upload_model = true;
    }

    void hide() noexcept
    {
        m_hidden = true;
    }

    void show() noexcept
    {
        m_hidden = false;
    }

    const cpt::binding& binding(std::uint32_t index) const
    {
        return m_bindings.at(index);
    }

    optional_ref<const cpt::binding> try_get_binding(std::uint32_t index) const
    {
        const auto it{m_bindings.find(index)};

        if(it != std::end(m_bindings))
        {
            return it->second;
        }

        return nullref;
    }

    bool has_binding(std::uint32_t index) const
    {
        return m_bindings.find(index) != std::end(m_bindings);
    }

    template<typename T>
    const T& get_push_constant(tph::shader_stage stages, std::uint32_t offset) const
    {
        return m_push_constants.get<T>(stages, offset);
    }

    template<typename T>
    optional_ref<const T> try_get_push_constant(tph::shader_stage stages, std::uint32_t offset) const
    {
        return m_push_constants.try_get<T>(stages, offset);
    }

    bool has_push_constant(tph::shader_stage stages, std::uint32_t offset) const
    {
        return m_push_constants.has(stages, offset);
    }

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

    std::span<vertex> vertices() noexcept
    {
        m_upload_vertices = true;

        return std::span{&m_buffer->get<vertex>(1), static_cast<std::size_t>(m_vertex_count)};
    }

    std::span<const vertex> vertices() const noexcept
    {
        return std::span{&m_buffer->get<const vertex>(1), static_cast<std::size_t>(m_vertex_count)};
    }

    std::span<const vertex> cvertices() const noexcept
    {
        return std::span{&m_buffer->get<const vertex>(1), static_cast<std::size_t>(m_vertex_count)};
    }

    std::span<std::uint32_t> indices() noexcept
    {
        assert(m_index_count > 0 && "cpt::basic_renderable::get_indices called on basic_renderable with no index buffer");

        m_upload_indices = true;

        return std::span{&m_buffer->get<std::uint32_t>(2), static_cast<std::size_t>(m_index_count)};
    }

    std::span<const std::uint32_t> indices() const noexcept
    {
        assert(m_index_count > 0 && "cpt::basic_renderable::get_indices called on basic_renderable with no index buffer");

        return std::span{&m_buffer->get<const std::uint32_t>(2), static_cast<std::size_t>(m_index_count)};
    }

    std::span<const std::uint32_t> cindices() const noexcept
    {
        assert(m_index_count > 0 && "cpt::basic_renderable::get_indices called on basic_renderable with no index buffer");

        return std::span{&m_buffer->get<const std::uint32_t>(2), static_cast<std::size_t>(m_index_count)};
    }

private:
    struct descriptor_set_data
    {
        descriptor_set_ptr set{};
        std::uint32_t epoch{};
    };

    using descriptor_set_map = std::map<render_layout_weak_ptr, descriptor_set_data, std::owner_less<render_layout_weak_ptr>>;

private:
    std::unordered_map<std::uint32_t, cpt::binding> m_bindings{};
    push_constants_buffer m_push_constants{};
    descriptor_set_map m_sets{};
    uniform_buffer* m_buffer{};

    std::uint32_t m_vertex_count{};
    std::uint32_t m_index_count{};
    std::uint32_t m_descriptors_epoch{};

    vec3f m_position{};
    vec3f m_origin{};
    vec3f m_scale{1.0f};
    float m_rotation{};
    bool  m_hidden{};

    bool m_upload_model{true};
    bool m_upload_indices{true};
    bool m_upload_vertices{true};
};

class CAPTAL_API sprite final : public basic_renderable
{
public:
    sprite() = default;
    explicit sprite(std::uint32_t width, std::uint32_t height, const color& color = colors::white);
    explicit sprite(texture_ptr texture, const color& color = colors::white);
    explicit sprite(std::uint32_t width, std::uint32_t height, texture_ptr texture, const color& color = colors::white);

    ~sprite() = default;
    sprite(const sprite&) = delete;
    sprite& operator=(const sprite&) = delete;
    sprite(sprite&&) noexcept = default;
    sprite& operator=(sprite&&) noexcept = default;

    void set_texture(texture_ptr texture);
    void set_color(const color& color) noexcept;

    void set_texture_coords(std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2) noexcept;
    void set_texture_rect(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) noexcept;

    void set_relative_texture_coords(float x1, float y1, float x2, float y2) noexcept;
    void set_relative_texture_rect(float x, float y, float width, float height) noexcept;

    void set_spritesheet_coords(std::uint32_t x, std::uint32_t y) noexcept;

    void resize(std::uint32_t width, std::uint32_t height) noexcept;

    const texture_ptr& texture() const
    {
        return std::get<texture_ptr>(binding(1));
    }

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

class CAPTAL_API polygon final : public basic_renderable
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

class CAPTAL_API tilemap final : public basic_renderable
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

    void set_texture(texture_ptr texture);
    void set_color(std::uint32_t row, std::uint32_t col, const color& color) noexcept;

    void set_texture_coords(std::uint32_t row, std::uint32_t col, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2) noexcept;
    void set_texture_rect(std::uint32_t row, std::uint32_t col, std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) noexcept;
    void set_texture_rect(std::uint32_t row, std::uint32_t col, const tileset::texture_rect& rect) noexcept;

    void set_relative_texture_coords(std::uint32_t row, std::uint32_t col, float x1, float y1, float x2, float y2) noexcept;
    void set_relative_texture_rect(std::uint32_t row, std::uint32_t col, float x, float y, float width, float height) noexcept;

    const texture_ptr& texture() const
    {
        return std::get<texture_ptr>(binding(1));
    }

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
