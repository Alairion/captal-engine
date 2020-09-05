#include "state.hpp"

#include <algorithm>
#include <cassert>

namespace cpt
{

void state::entered(state_stack& stack [[maybe_unused]])
{

}

void state::raised(state_stack& stack [[maybe_unused]])
{

}

void state::fell(state_stack& stack [[maybe_unused]])
{

}

void state::leaved(state_stack& stack [[maybe_unused]])
{

}

state_stack::state_stack(handle initial_state)
{
    push(std::move(initial_state));
}

void state_stack::push(handle state)
{
    value_type& new_top{*state};

    if(!std::empty(m_states))
    {
        value_type& last_top{*m_states.back()};

        m_states.emplace_back(std::move(state));

        new_top.entered(*this);
        last_top.fell(*this);
        new_top.raised(*this);
    }
    else
    {
        m_states.emplace_back(std::move(state));

        new_top.entered(*this);
        new_top.raised(*this);
    }
}

void state_stack::insert_above(pointer position, handle state)
{
    if(is_top(position))
    {
        push(std::move(state));
    }
    else
    {
        const auto it{find(position) + 1};

        value_type& new_state{*state};

        m_states.insert(it, std::move(state));
        new_state.entered(*this);
    }
}

void state_stack::insert_below(const_pointer position, handle state)
{
    const auto it{find(position)};

    value_type& new_state{*state};

    m_states.insert(it, std::move(state));
    new_state.entered(*this);
}

state_stack::handle state_stack::pop()
{
    assert(!std::empty(m_states) && "cpt::state_stack::pop called on empty state_stack");

    handle output{std::move(m_states.back())};
    m_states.pop_back();

    output->leaved(*this);

    if(!std::empty(m_states))
    {
        m_states.back()->raised(*this);
    }

    return output;
}

state_stack::handle state_stack::remove(pointer position)
{
    if(is_top(position))
    {
        return pop();
    }
    else
    {
        const auto it{find(position)};

        handle output{std::move(*it)};
        m_states.erase(it);

        return output;
    }
}

state_stack::handle state_stack::remove_above(pointer position)
{
    assert(!is_top(position) && "cpt::state_stack::remove_above called on top state");

    const auto it{find(position) + 1};

    if(is_top(it->get()))
    {
        return pop();
    }
    else
    {
        handle output{std::move(*it)};
        m_states.erase(it);

        return output;
    }
}

state_stack::handle state_stack::remove_below(const_pointer position)
{
    assert(m_states.front().get() != position && "cpt::state_stack::remove_above called on bottom state");

    const auto it{find(position) - 1};

    handle output{std::move(*it)};
    m_states.erase(it);

    return output;
}

void state_stack::clear()
{
    m_states.clear();
}

void state_stack::reset(handle initial_state)
{
    clear();
    push(std::move(initial_state));
}

void state_stack::pop_until(pointer state)
{
    while(!is_top(state))
    {
        pop();
    }
}

void state_stack::raise(pointer state)
{
    const auto it{find(state)};

    value_type& new_top{*state};
    value_type& last_top{*m_states.back()};

    m_states.emplace_back(std::move(*it));
    m_states.erase(it);

    last_top.fell(*this);
    new_top.raised(*this);
}

void state_stack::update(float elapsed_time)
{
    for(auto&& state : m_states)
    {
        state->update(*this, elapsed_time);
    }

    for(auto&& callback : m_post_update_callbacks)
    {
        callback(*this);
    }

    m_post_update_callbacks.clear();
}

void state_stack::add_post_update_callback(post_update_callback_type callback)
{
    m_post_update_callbacks.emplace_back(std::move(callback));
}

std::vector<state_stack::handle>::iterator state_stack::find(const_pointer state) noexcept
{
    const auto find_state = [state](const handle& other)
    {
        return state == other.get();
    };

    const auto it{std::find_if(std::begin(m_states), std::end(m_states), find_state)};
    assert(it != std::end(m_states) && "state_stack does not contains specified state");

    return it;
}

std::vector<state_stack::handle>::const_iterator state_stack::find(const_pointer state) const noexcept
{
    const auto find_state = [state](const handle& other)
    {
        return state == other.get();
    };

    const auto it{std::find_if(std::begin(m_states), std::end(m_states), find_state)};
    assert(it != std::end(m_states) && "state_stack does not contains specified state");

    return it;
}

}
