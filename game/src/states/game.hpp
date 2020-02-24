#ifndef MY_PROJECT_STATES_GAME_HPP_INCLUDED
#define MY_PROJECT_STATES_GAME_HPP_INCLUDED

#include "../config.hpp"

#include <captal/state.hpp>
#include <captal/render_window.hpp>
#include <captal/text.hpp>

#include <entt/entity/registry.hpp>

namespace mpr
{

namespace states
{

class game : public cpt::state
{
public:
    game(cpt::render_window_ptr window);
    ~game() = default;
    game(const game&) = delete;
    game& operator=(const game&) = delete;
    game(game&&) noexcept = delete;
    game& operator=(game&&) noexcept = delete;

protected:
    virtual void on_enter(cpt::state_stack& stack) override;
    virtual void on_leave(cpt::state_stack& stack) override;
    virtual void on_update(cpt::state_stack& stack, float elapsed_time) override;

private:
    cpt::render_window_ptr m_window{};
    entt::registry m_world{};
    entt::entity m_view_entity{};
    std::vector<sigslot::scoped_connection> m_connections{};
};

}

}

#endif
