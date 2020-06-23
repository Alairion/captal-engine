#ifndef CAPTAL_COMPONENTS_PHYSICAL_BODY_HPP_INCLUDED
#define CAPTAL_COMPONENTS_PHYSICAL_BODY_HPP_INCLUDED

#include "../config.hpp"

#include <vector>

#include "../physics.hpp"

namespace cpt
{

namespace components
{

class CAPTAL_API physical_body
{
public:
    using value_type = cpt::physical_body;

public:
    physical_body() = default;

    explicit physical_body(physical_body_ptr attachment)
    :m_attachment{std::move(attachment)}
    {

    }

    ~physical_body() = default;
    physical_body(const physical_body&) = default;
    physical_body& operator=(const physical_body&) = default;
    physical_body(physical_body&&) noexcept = default;
    physical_body& operator=(physical_body&&) noexcept = default;

    void attach(physical_body_ptr attachment) noexcept
    {
        m_attachment = std::move(attachment);
    }

    void attach(physical_shape_ptr attachment)
    {
        assert(attachment->body() == m_attachment && "cpt::component::physical_body::attach can only attach shape that belong to its attachment.");

        m_shapes.emplace_back(std::move(attachment));
    }

    template<typename... Args>
    const physical_shape_ptr& add_shape(Args&&... args)
    {
        return m_shapes.emplace_back(make_physical_shape(m_attachment, std::forward<Args>(args)...));
    }

    void detach() noexcept
    {
        m_shapes.clear();
        m_attachment.reset();
    }

    void detach(std::size_t index)
    {
        m_shapes.erase(std::begin(m_shapes) + index);
    }

    void detach(const physical_shape_ptr& shape)
    {
        m_shapes.erase(std::find(std::begin(m_shapes), std::end(m_shapes), shape));
    }

    const physical_body_ptr& attachment() const noexcept
    {
        return m_attachment;
    }

    const physical_shape_ptr& shape(std::size_t index) const noexcept
    {
        return m_shapes[index];
    }

    cpt::physical_body* operator->() const noexcept
    {
        return m_attachment.get();
    }

private:
    physical_body_ptr m_attachment{};
    std::vector<physical_shape_ptr> m_shapes{};
};

}

}

#endif
