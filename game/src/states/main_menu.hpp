#ifndef MY_PROJECT_STATES_MAIN_MENU_HPP_INCLUDED
#define MY_PROJECT_STATES_MAIN_MENU_HPP_INCLUDED

#include "../config.hpp"

#include <captal/state.hpp>
#include <captal/render_window.hpp>
#include <captal/text.hpp>

#include <entt/entity/registry.hpp>

namespace mpr
{

namespace states
{

class main_menu : public cpt::state
{
public:
    main_menu(cpt::render_window_ptr window);
    ~main_menu() = default;
    main_menu(const main_menu&) = delete;
    main_menu& operator=(const main_menu&) = delete;
    main_menu(main_menu&&) noexcept = delete;
    main_menu& operator=(main_menu&&) noexcept = delete;

protected:
    virtual void on_enter(cpt::state_stack& stack) override;
    virtual void on_leave(cpt::state_stack& stack) override;
    virtual void on_update(cpt::state_stack& stack, float elapsed_time) override;

private:
    bool above_play_button(std::int32_t x, std::int32_t y) const noexcept;

private:
    cpt::render_window_ptr m_window{};
    entt::registry m_world{};
    entt::entity m_view_entity{};
    entt::entity m_text_entity{};
    cpt::font m_font{};
    cpt::text_ptr m_text{};
    std::vector<sigslot::scoped_connection> m_connections{};
    bool m_play_button_pressed{};
};

}

}

#endif
