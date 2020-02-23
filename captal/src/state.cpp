#include "state.hpp"

#include <algorithm>

namespace cpt
{

state_stack::state_stack(state_ptr initial_state)
{
    push(initial_state);
}

void state_stack::push(state_ptr state)
{
    m_states.push_back(std::move(state));
    m_states.back()->enter(*this);
}

state_ptr state_stack::pop()
{
    state_ptr output{m_states.back()};
    m_states.pop_back();

    output->leave(*this);
    return output;
}

void state_stack::reset(state_ptr initial_state)
{
    while(!std::empty(m_states))
        pop();

    push(std::move(initial_state));
}

void state_stack::pop_until(const state_ptr& state)
{
    while(!is_top(state))
        pop();
}

void state_stack::pop_until(state* state)
{
    while(!is_top(state))
        pop();
}

void state_stack::raise(const state_ptr& state)
{
    const auto it{std::find(std::begin(m_states), std::end(m_states), state)};
    state_ptr last_top{m_states.back()};

    m_states.push_back(*it);
    m_states.erase(it);

    last_top->leave(*this);
    m_states.back()->enter(*this);
}

void state_stack::raise(state* state)
{
    const auto find_state = [state](const state_ptr& other) -> bool
    {
        return state == other.get();
    };

    const auto it{std::find_if(std::begin(m_states), std::end(m_states), find_state)};
    state_ptr last_top{m_states.back()};

    m_states.push_back(*it);
    m_states.erase(it);

    last_top->leave(*this);
    m_states.back()->enter(*this);
}

void state_stack::update(float elapsed_time)
{
    for(auto&& state : m_states)
        state->update(*this, elapsed_time);

    for(auto&& callback : m_post_update_callbacks)
        callback(*this);

    m_post_update_callbacks.clear();
}

void state_stack::add_post_update_callback(post_update_callback_type callback)
{
    m_post_update_callbacks.push_back(std::move(callback));
}

bool state_stack::is_top(const state* state) const noexcept
{
    return m_states.back().get() == state;
}

bool state_stack::is_top(const state_ptr& state) const noexcept
{
    return m_states.back() == state;
}

const state_ptr& state_stack::current() const noexcept
{
    return m_states.back();
}



}
