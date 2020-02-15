#ifndef CAPTAL_VIEW_HPP_INCLUDED
#define CAPTAL_VIEW_HPP_INCLUDED

#include "config.hpp"

#include <vector>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "asynchronous_resource.hpp"
#include "framed_buffer.hpp"
#include "uniform_buffer.hpp"
#include "render_technique.hpp"
#include "render_window.hpp"
#include "render_texture.hpp"

namespace cpt
{

enum class view_type
{
    orthographic = 0,
};

class CAPTAL_API view : public asynchronous_resource
{
public:
    struct uniform_data
    {
        glm::vec4 position{};
        glm::mat4 view{1.0f};
        glm::mat4 projection{1.0f};
    };

public:
    view();
    view(render_target_ptr target, const render_technique_info& info = render_technique_info{});
    view(render_target_ptr target, render_technique_ptr technique);
    ~view() = default;
    view(const view&) = delete;
    view& operator=(const view&) = delete;
    view(view&&) noexcept = default;
    view& operator=(view&&) noexcept = default;

    void set_viewport(float x, float y, float width, float height, float min_depth, float max_depth) noexcept
    {
        m_viewport = tph::viewport{x, y, width, height, min_depth, max_depth};
    }

    void set_viewport(const tph::viewport& viewport) noexcept
    {
        m_viewport = viewport;
    }

    void set_scissor(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) noexcept
    {
        m_scissor = tph::scissor{x, y, width, height};
    }

    void set_scissor(const tph::scissor& scissor) noexcept
    {
        m_scissor = scissor;
    }

    void resize(float witdh, float height) noexcept
    {
        m_size = glm::vec2{witdh, height};
        m_need_upload = true;
    }

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

    void set_origin(const glm::vec3& origin) noexcept
    {
        m_origin = origin;
        m_need_upload = true;
    }

    void set_origin(float x, float y, float z = 0.0f) noexcept
    {
        m_origin = glm::vec3{x, y, z};
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

    void set_rotation(float angle) noexcept
    {
        m_rotation = angle;
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
        m_scale *= scale;
        m_need_upload = true;
    }

    void set_render_technique(render_technique_ptr technique) noexcept
    {
        m_render_technique = std::move(technique);
    }

    void set_target(render_target_ptr target, const render_technique_info& info = render_technique_info{});
    void set_target(render_target_ptr target, render_technique_ptr technique);
    void fit_to(const render_texture_ptr& window);
    void fit_to(const render_window_ptr& texture);

    void update();
    void upload();

    const tph::viewport& viewport() const noexcept
    {
        return m_viewport;
    }

    const tph::scissor& scissor() const noexcept
    {
        return m_scissor;
    }

    const glm::vec3& position() const noexcept
    {
        return m_position;
    }

    const glm::vec3& origin() const noexcept
    {
        return m_origin;
    }

    const glm::vec2& size() const noexcept
    {
        return m_size;
    }

    float width() const noexcept
    {
        return m_size.x;
    }

    float height() const noexcept
    {
        return m_size.y;
    }

    float scale() const noexcept
    {
        return m_scale;
    }

    float rotation() const noexcept
    {
        return m_rotation;
    }

    view_type type() const noexcept
    {
        return m_type;
    }

    const render_target_ptr& target() const noexcept
    {
        return m_target;
    }

    tph::buffer& buffer() noexcept
    {
        return m_buffer.buffer();
    }

    const tph::buffer& buffer() const noexcept
    {
        return m_buffer.buffer();
    }

    template<typename T>
    void add_uniform_buffer(std::uint32_t binding, const T& data)
    {
        m_uniform_buffers.emplace_back(binding, data);
        m_uniform_buffers.back().upload();
        m_need_descriptor_update = true;
    }

    cpt::uniform_buffer& uniform_buffer(std::uint32_t binding) noexcept
    {
        return *std::find_if(std::begin(m_uniform_buffers), std::end(m_uniform_buffers), [binding](const cpt::uniform_buffer& buffer)
        {
            return buffer.binding() == binding;
        });
    }

    const cpt::uniform_buffer& uniform_buffer(std::uint32_t binding) const noexcept
    {
        return *std::find_if(std::begin(m_uniform_buffers), std::end(m_uniform_buffers), [binding](const cpt::uniform_buffer& buffer)
        {
            return buffer.binding() == binding;
        });
    }

    template<typename T>
    void set_uniform(std::uint32_t binding, const T& data) noexcept
    {
        uniform_buffer(binding).set(data);
    }

    std::vector<cpt::uniform_buffer>& uniform_buffers() noexcept
    {
        return m_uniform_buffers;
    }

    const std::vector<cpt::uniform_buffer>& uniform_buffers() const noexcept
    {
        return m_uniform_buffers;
    }

    bool need_descriptor_update(bool new_value = false) noexcept
    {
        return std::exchange(m_need_descriptor_update, new_value);
    }

    const render_technique_ptr& render_technique() const noexcept
    {
        return m_render_technique;
    }

private:
    tph::viewport m_viewport{};
    tph::scissor m_scissor{};
    glm::vec3 m_position{};
    glm::vec3 m_origin{};
    glm::vec2 m_size{};
    float m_scale{1.0f};
    float m_rotation{};
    view_type m_type{};

    render_target_ptr m_target{};
    framed_buffer m_buffer{};
    bool m_need_upload{true};
    std::vector<cpt::uniform_buffer> m_uniform_buffers{};
    bool m_need_descriptor_update{};
    render_technique_ptr m_render_technique{};
};

using view_ptr = std::shared_ptr<view>;

template<typename... Args>
view_ptr make_view(Args&&... args)
{
    return std::make_shared<view>(std::forward<Args>(args)...);
}

}

#endif
