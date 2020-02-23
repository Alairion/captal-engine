#ifndef MY_PROJECT_STATES_SPLASH_SCREEN_HPP_INCLUDED
#define MY_PROJECT_STATES_SPLASH_SCREEN_HPP_INCLUDED

#include "../config.hpp"

#include <captal/state.hpp>
#include <captal/render_window.hpp>
#include <captal/text.hpp>

#include <entt/entity/registry.hpp>

namespace mpr
{

namespace states
{

class splash_screen : public cpt::state
{
public:
    splash_screen(cpt::render_window_ptr window);
    ~splash_screen() = default;
    splash_screen(const splash_screen&) = delete;
    splash_screen& operator=(const splash_screen&) = delete;
    splash_screen(splash_screen&&) noexcept = delete;
    splash_screen& operator=(splash_screen&&) noexcept = delete;

protected:
    virtual void on_enter(cpt::state_stack& stack) override;
    virtual void on_leave(cpt::state_stack& stack) override;
    virtual void on_update(cpt::state_stack& stack, float elapsed_time) override;

private:
    cpt::render_window_ptr m_window{};
    entt::registry m_world{};
    entt::entity m_text_entity{};
    entt::entity m_view_entity{};
    cpt::font m_font{};
    cpt::text_ptr m_text{};
    sigslot::connection m_resize_connection{};
    float m_time{};
};

}

}

#endif
