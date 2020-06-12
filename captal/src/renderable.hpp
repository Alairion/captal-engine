#ifndef CAPTAL_RENDERABLE_HPP_INCLUDED
#define CAPTAL_RENDERABLE_HPP_INCLUDED

#include "config.hpp"

#include <unordered_map>

#include <glm/mat4x4.hpp>

#include "asynchronous_resource.hpp"
#include "framed_buffer.hpp"
#include "uniform_buffer.hpp"
#include "color.hpp"
#include "view.hpp"
#include "vertex.hpp"
#include "texture.hpp"

namespace cpt
{

class CAPTAL_API renderable : public asynchronous_resource
{
public:
    struct uniform_data
    {
        glm::mat4 model{};
    };

public:
    renderable() = default;
    renderable(std::uint32_t vertex_count);
    renderable(std::uint32_t index_count, std::uint32_t vertex_count);
    virtual ~renderable() = default;
    renderable(const renderable&) = delete;
    renderable& operator=(const renderable&) = delete;
    renderable(renderable&&) noexcept = default;
    renderable& operator=(renderable&&) noexcept = default;

    void set_indices(const std::vector<std::uint32_t>& indices) noexcept;
    void set_vertices(const std::vector<vertex>& vertices) noexcept;
    void set_texture(texture_ptr texture) noexcept;
    void set_view(const view_ptr& view);

    void move(const glm::vec3& relative) noexcept
    {
        m_position += relative;
        m_need_upload = true;
    }

    void move_to(const glm::vec3& position) noexcept
    {
        m_position = position;
        m_need_upload = true;
    }

    void set_origin(const glm::vec3& origin) noexcept
    {
        m_origin = origin;
        m_need_upload = true;
    }

    void move_origin(const glm::vec3& relative) noexcept
    {
        m_origin += relative;
        m_need_upload = true;
    }

    void rotate(float angle) noexcept
    {
        m_rotation = std::fmod(m_rotation + angle, pi<float> * 2.0f);
        m_need_upload = true;
    }

    void set_rotation(float angle) noexcept
    {
        m_rotation = std::fmod(angle, pi<float> * 2.0f);
        m_need_upload = true;
    }

    void scale(float scale) noexcept
    {
        m_scale += scale;
        m_need_upload = true;
    }

    void set_scale(float scale) noexcept
    {
        m_scale = scale;
        m_need_upload = true;
    }

    void hide() noexcept
    {
        m_hidden = true;
    }

    void show() noexcept
    {
        m_hidden = false;
    }

    void update();
    void upload();

    void draw(tph::command_buffer& buffer);

    const glm::vec3& position() const noexcept
    {
        return m_position;
    }

    const glm::vec3& origin() const noexcept
    {
        return m_origin;
    }

    float scale() const noexcept
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

    std::uint32_t index_count() const noexcept
    {
        return m_index_count;
    }

    std::uint32_t vertex_count() const noexcept
    {
        return m_vertex_count;
    }

    std::uint32_t* get_indices() noexcept
    {
        assert(m_index_count > 0 && "cpt::renderable::get_indices called on renderable with no index buffer");

        return &m_buffer.get<std::uint32_t>(1);
    }

    const std::uint32_t* get_indices() const noexcept
    {
        assert(m_index_count > 0 && "cpt::renderable::get_indices called on renderable with no index buffer");

        return &m_buffer.get<const std::uint32_t>(1);
    }

    vertex* get_vertices() noexcept
    {
        return &m_buffer.get<vertex>(m_index_count > 0 ? 2 : 1);
    }

    const vertex* get_vertices() const noexcept
    {
        return &m_buffer.get<const vertex>(m_index_count > 0 ? 2 : 1);
    }

    const descriptor_set_ptr& set() const noexcept
    {
        return m_current_set;
    }

    const descriptor_set_ptr& set(const view_ptr& view) const noexcept
    {
        return m_descriptor_sets.at(view.get());
    }

    const texture_ptr& texture() const noexcept
    {
        return m_texture;
    }

    template<typename T>
    cpt::uniform_binding& add_uniform_binding(std::uint32_t binding, T&& data)
    {
        auto [it, success] = m_uniform_bindings.try_emplace(binding, cpt::uniform_binding{std::forward<T>(data)});
        assert(success && "cpt::view::add_uniform_buffer called with already used binding.");

        m_need_descriptor_update = true;
        return it->second;
    }

    cpt::uniform_binding& uniform_binding(std::uint32_t binding)
    {
        return m_uniform_bindings.at(binding);
    }

    const cpt::uniform_binding& uniform_binding(std::uint32_t binding) const
    {
        return m_uniform_bindings.at(binding);
    }

    template<typename T>
    void set_uniform(std::uint32_t binding, T&& data)
    {
        uniform_binding(binding) = std::forward<T>(data);
        m_need_descriptor_update = true;
    }

    bool has_binding(std::uint32_t binding) const
    {
        return m_uniform_bindings.find(binding) != std::end(m_uniform_bindings);
    }

    std::unordered_map<std::uint32_t, cpt::uniform_binding>& uniform_bindings() noexcept
    {
        return m_uniform_bindings;
    }

    const std::unordered_map<std::uint32_t, cpt::uniform_binding>& uniform_bindings() const noexcept
    {
        return m_uniform_bindings;
    }

private:
    std::uint32_t m_index_count{};
    std::uint32_t m_vertex_count{};

    glm::vec3 m_position{};
    glm::vec3 m_origin{};
    float m_scale{1.0f};
    float m_rotation{};
    bool m_hidden{};

    framed_buffer m_buffer{};
    bool m_need_upload{true};

    texture_ptr m_texture{};
    std::unordered_map<std::uint32_t, cpt::uniform_binding> m_uniform_bindings{};

    std::unordered_map<const view*, descriptor_set_ptr> m_descriptor_sets{};
    descriptor_set_ptr m_current_set{};
    bool m_need_descriptor_update{};
};

using renderable_ptr = std::shared_ptr<renderable>;
using renderable_weak_ptr = std::weak_ptr<renderable>;

template<typename... Args>
renderable_ptr make_renderable(Args&&... args)
{
    return std::make_shared<renderable>(std::forward<Args>(args)...);
}

class CAPTAL_API sprite : public renderable
{
public:
    sprite() = default;
    sprite(std::uint32_t width, std::uint32_t height, const color& color = colors::white);
    sprite(texture_ptr texture);
    ~sprite() = default;
    sprite(const sprite&) = delete;
    sprite& operator=(const sprite&) = delete;
    sprite(sprite&&) noexcept = default;
    sprite& operator=(sprite&&) noexcept = default;

    void set_color(const color& color) noexcept;

    void set_texture_coords(std::int32_t x1, std::int32_t y1, std::uint32_t x2, std::uint32_t y2) noexcept;
    void set_texture_rect(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) noexcept;

    void set_relative_texture_coords(float x1, float y1, float x2, float y2) noexcept;
    void set_relative_texture_rect(float x, float y, float width, float height) noexcept;

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

using sprite_ptr = std::shared_ptr<sprite>;
using sprite_weak_ptr = std::weak_ptr<sprite>;

template<typename... Args>
sprite_ptr make_sprite(Args&&... args)
{
    return std::make_shared<sprite>(std::forward<Args>(args)...);
}

class CAPTAL_API circle : public renderable
{
public:
    circle() = default;
    circle(float radius, const color& color = colors::white);
    circle(float radius, std::uint32_t point_count = 0, const color& color = colors::white);

    ~circle() = default;
    circle(const circle&) = delete;
    circle& operator=(const circle&) = delete;
    circle(circle&&) noexcept = default;
    circle& operator=(circle&&) noexcept = default;

    void set_color(const color& color) noexcept;
    void set_center_color(const color& color) noexcept;
    void set_outline_color(const color& color) noexcept;
    void set_point_color(std::uint32_t point, const color& color) noexcept;

    void set_texture_coords(std::int32_t x1, std::int32_t y1, std::uint32_t x2, std::uint32_t y2) noexcept;
    void set_texture_rect(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) noexcept;

    void set_relative_texture_coords(float x1, float y1, float x2, float y2) noexcept;
    void set_relative_texture_rect(float x, float y, float width, float height) noexcept;

    void resize(float radius);

private:
    void init(const color& color);

private:
    float m_radius{};
    std::uint32_t m_point_count{};
};

using circle_ptr = std::shared_ptr<circle>;
using circle_weak_ptr = std::weak_ptr<circle>;

template<typename... Args>
circle_ptr make_circle(Args&&... args)
{
    return std::make_shared<circle>(std::forward<Args>(args)...);
}

class CAPTAL_API tilemap : public renderable
{
public:
    tilemap() = default;
    tilemap(std::uint32_t width, std::uint32_t height, std::uint32_t tile_width, std::uint32_t tile_height);
    tilemap(std::uint32_t width, std::uint32_t height, const tileset& tileset);

    ~tilemap() = default;
    tilemap(const tilemap&) = delete;
    tilemap& operator=(const tilemap&) = delete;
    tilemap(tilemap&&) noexcept = default;
    tilemap& operator=(tilemap&&) noexcept = default;

    void set_color(std::uint32_t row, std::uint32_t col, const color& color) noexcept;

    void set_texture_coords(std::uint32_t row, std::uint32_t col, std::int32_t x1, std::int32_t y1, std::uint32_t x2, std::uint32_t y2) noexcept;
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

using tilemap_ptr = std::shared_ptr<tilemap>;
using tilemap_weak_ptr = std::weak_ptr<tilemap>;

template<typename... Args>
tilemap_ptr make_tilemap(Args&&... args)
{
    return std::make_shared<tilemap>(std::forward<Args>(args)...);
}

}

#endif
