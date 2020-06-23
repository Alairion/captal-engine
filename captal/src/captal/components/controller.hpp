#ifndef CAPTAL_COMPONENTS_CONTROLLER_HPP_INCLUDED
#define CAPTAL_COMPONENTS_CONTROLLER_HPP_INCLUDED

#include "../config.hpp"

#include <vector>

#include "../physics.hpp"

namespace cpt
{

namespace components
{

struct CAPTAL_API controller
{
public:
    using value_type = cpt::physical_body;

public:
    controller() = default;

    explicit controller(cpt::physical_body_weak_ptr controlled)
    :m_controlled{std::move(controlled)}
    ,m_body{cpt::make_physical_body(m_controlled.lock()->world(), physical_body_type::kinematic)}
    {

    }

    ~controller() = default;
    controller(const controller&) = default;
    controller& operator=(const controller&) = default;
    controller(controller&&) noexcept = default;
    controller& operator=(controller&&) noexcept = default;

    void attach(cpt::physical_body_weak_ptr controlled)
    {
        m_controlled = std::move(controlled);
        m_body = cpt::make_physical_body(m_controlled.lock()->world(), physical_body_type::kinematic);
        m_constraints.clear();
    }

    void attach(cpt::physical_constraint_ptr constraint)
    {
        assert((constraint->bodies_ptr().first == m_controlled.lock() || constraint->bodies_ptr().second == m_controlled.lock()) && "cpt::component::physical_body::attach can only attach constraint links its attachment and its body.");
        assert((constraint->bodies_ptr().first == m_body || constraint->bodies_ptr().second == m_body) && "cpt::component::physical_body::attach can only attach constraint links its attachment and its body.");

        m_constraints.emplace_back(std::move(constraint));
    }

    template<typename DisambiguationTag, typename... Args>
    const cpt::physical_constraint_ptr& add_constraint(DisambiguationTag tag, Args&&... args)
    {
        return m_constraints.emplace_back(make_physical_constraint(tag, m_controlled.lock(), m_body, std::forward<Args>(args)...));
    }

    void detach()
    {
        m_constraints.clear();
        m_body.reset();
        m_controlled.reset();
    }

    void detach(std::size_t index)
    {
        m_constraints.erase(std::begin(m_constraints) + index);
    }

    void detach(const cpt::physical_constraint_ptr& constraint)
    {
        m_constraints.erase(std::find(std::begin(m_constraints), std::end(m_constraints), constraint));
    }

    const cpt::physical_body_weak_ptr& controlled() const noexcept
    {
        return m_controlled;
    }

    const cpt::physical_body_ptr& body() const noexcept
    {
        return m_body;
    }

    const cpt::physical_constraint_ptr& constraint(std::size_t index) const noexcept
    {
        return m_constraints[index];
    }

    cpt::physical_body* operator->() const noexcept
    {
        return m_body.get();
    }

private:
    cpt::physical_body_weak_ptr m_controlled{};
    cpt::physical_body_ptr m_body{};
    std::vector<cpt::physical_constraint_ptr> m_constraints{};
};

}

}

#endif
