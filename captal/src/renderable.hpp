#ifndef CAPTAL_RENDERABLE_HPP_INCLUDED
#define CAPTAL_RENDERABLE_HPP_INCLUDED

#include "config.hpp"

#include <unordered_map>

#include <glm/mat4x4.hpp>

#include "asynchronous_resource.hpp"
#include "framed_buffer.hpp"
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
        float shininess{};
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

    void set_indices(const std::vector<std::uint16_t>& indices) noexcept;
    void set_vertices(const std::vector<vertex>& vertices) noexcept;

    void set_render_technique(render_technique_ptr technique) noexcept;
    void set_texture(texture_ptr texture) noexcept;
    void set_normal_map(texture_ptr texture) noexcept;
    void set_height_map(texture_ptr texture) noexcept;
    void set_specular_map(texture_ptr texture) noexcept;
    void set_view(const view_ptr& view);

    void move_to(const glm::vec3& position) noexcept
    {
        m_position = position;
        m_need_upload = true;
    }

    void move_to(float x, float y, float z = 1.0f) noexcept
    {
        m_position = glm::vec3{x, y, z};
        m_need_upload = true;
    }

    void set_origin(const glm::vec3& origin) noexcept
    {
        m_origin = origin;
        m_need_upload = true;
    }

    void set_origin(float x, float y, float z = 1.0f) noexcept
    {
        m_origin += glm::vec3{x, y, z};
        m_need_upload = true;
    }

    void move_origin(const glm::vec3& relative) noexcept
    {
        m_origin += relative;
        m_need_upload = true;
    }

    void move_origin(float x, float y, float z = 0.0f) noexcept
    {
        m_origin += glm::vec3{x, y, z};
        m_need_upload = true;
    }

    void move(const glm::vec3& relative) noexcept
    {
        m_position += relative;
        m_need_upload = true;
    }

    void move(float x, float y, float z = 0.0f) noexcept
    {
        m_position += glm::vec3{x, y, z};
        m_need_upload = true;
    }

    void set_rotation(float angle) noexcept
    {
        m_rotation = std::fmod(angle, pi<float> * 2.0f);
        m_need_upload = true;
    }

    void rotate(float angle) noexcept
    {
        m_rotation = std::fmod(m_rotation + angle, pi<float> * 2.0f);
        m_need_upload = true;
    }

    void set_scale(float scale) noexcept
    {
        m_scale = scale;
        m_need_upload = true;
    }

    void scale(float scale) noexcept
    {
        m_scale += scale;
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

    void set_shininess(float shininess) noexcept
    {
        m_shininess = shininess;
    }

    void update();
    void upload();

    void draw(tph::command_buffer& buffer);

    bool hidden() const noexcept
    {
        return m_hidden;
    }

    const glm::vec3& position() const noexcept
    {
        return m_position;
    }

    float x() const noexcept
    {
        return m_position.x;
    }

    float y() const noexcept
    {
        return m_position.y;
    }

    float z() const noexcept
    {
        return m_position.z;
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

    float shininess() const noexcept
    {
        return m_shininess;
    }

    std::uint32_t index_count() const noexcept
    {
        return m_index_count;
    }

    std::uint32_t vertex_count() const noexcept
    {
        return m_vertex_count;
    }

    std::uint16_t* get_indices() noexcept
    {
        return &m_buffer.get<std::uint16_t>(m_index_count > 0 ? 2 : 1);
    }

    const std::uint16_t* get_indices() const noexcept
    {
        return &m_buffer.get<const std::uint16_t>(m_index_count > 0 ? 2 : 1);
    }

    vertex* get_vertices() noexcept
    {
        return &m_buffer.get<vertex>(m_index_count > 0 ? 2 : 1);
    }

    const vertex* get_vertices() const noexcept
    {
        return &m_buffer.get<const vertex>(m_index_count > 0 ? 2 : 1);
    }

    const render_technique_ptr& technique() const noexcept
    {
        return m_render_technique;
    }

    const descriptor_set_ptr& set(view_ptr view) const noexcept
    {
        return m_descriptor_sets.at(view);
    }

    const texture_ptr& texture() const noexcept
    {
        return m_texture;
    }

    const texture_ptr& normal_map() const noexcept
    {
        return m_normal_map;
    }

    const texture_ptr& height_map() const noexcept
    {
        return m_height_map;
    }

    const texture_ptr& specular_map() const noexcept
    {
        return m_specular_map;
    }

private:
    std::uint32_t m_index_count{};
    std::uint32_t m_vertex_count{};

    glm::vec3 m_position{};
    glm::vec3 m_origin{};
    float m_scale{1.0f};
    float m_rotation{};
    bool m_hidden{};
    float m_shininess{};

    framed_buffer m_buffer{};
    bool m_need_upload{true};

    render_technique_ptr m_render_technique{};
    texture_ptr m_texture{};
    texture_ptr m_normal_map{};
    texture_ptr m_height_map{};
    texture_ptr m_specular_map{};
    std::unordered_map<view_ptr, descriptor_set_ptr> m_descriptor_sets{};

    descriptor_set* m_current_set{};
    bool m_need_descriptor_update{};
};

using renderable_ptr = std::shared_ptr<renderable>;

template<typename... Args>
renderable_ptr make_renderable(Args&&... args)
{
    return std::make_shared<renderable>(std::forward<Args>(args)...);
}

}

#endif
