#ifndef MY_PROJECT_STATES_SPLASH_SCREEN_HPP_INCLUDED
#define MY_PROJECT_STATES_SPLASH_SCREEN_HPP_INCLUDED

#include "../config.hpp"

#include <captal/physics.hpp>

namespace mpr
{

struct controller
{
public:
    controller(const cpt::physical_body_ptr& player_body)
    :m_body{cpt::make_physical_body(player_body->world(), cpt::physical_body_type::kinematic)}
    ,m_joint{cpt::make_physical_constraint(cpt::pin_joint, m_body, player_body, glm::vec2{}, player_body->mass_center())}
    {

    }

    ~controller() = default;
    controller(const controller&) = default;
    controller& operator=(const controller&) = default;
    controller(controller&&) noexcept = default;
    controller& operator=(controller&&) noexcept = default;

    const cpt::physical_body_ptr& body() const noexcept
    {
        return m_body;
    }

    const cpt::physical_constraint_ptr& joint() const noexcept
    {
        return m_joint;
    }

private:
    cpt::physical_body_ptr m_body{};
    cpt::physical_constraint_ptr m_joint{};
};

}

#endif
