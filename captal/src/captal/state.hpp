#ifndef CAPTAL_STATE_HPP_INCLUDED
#define CAPTAL_STATE_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <vector>
#include <functional>
#include <cassert>

namespace cpt
{

template<typename StateT>
class state_stack;

class state // A simple generic state type, usefull if nothing more is needed
{
public:
    state() noexcept = default;
    virtual ~state() = default;
    state(const state&) = delete;
    state& operator=(const state&) = delete;
    state(state&& other) noexcept = delete;
    state& operator=(state&& other) noexcept = delete;

    virtual void entered(state_stack<state>& stack [[maybe_unused]])
    {

    }

    virtual void raised(state_stack<state>& stack [[maybe_unused]])
    {

    }

    virtual void fell(state_stack<state>& stack [[maybe_unused]])
    {

    }

    virtual void leaved(state_stack<state>& stack [[maybe_unused]])
    {

    }

    virtual void update(state_stack<state>& stack, float elapsed_time) = 0;
};

template<typename T>
concept is_state = requires(T t, state_stack<T>& stack, float elapsed_time)
{
    t.entered(stack);
    t.raised(stack);
    t.fell(stack);
    t.leaved(stack);
    t.update(stack, elapsed_time);
};

template<typename StateT>
class state_stack
{
    static_assert(is_state<StateT>, "is_state constraints are not satisfied by StateT");

public:
    using value_type = StateT;
    using handle = std::unique_ptr<value_type>;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using post_update_callback_type = std::function<void(state_stack& stack)>;

public:
    state_stack() = default;

    state_stack(handle initial_state)
    {
        push(std::move(initial_state));
    }

    ~state_stack() = default;
    state_stack(const state_stack&) = delete;
    state_stack& operator=(const state_stack&) = delete;
    state_stack(state_stack&& other) noexcept = default;
    state_stack& operator=(state_stack&& other) noexcept = default;

    void push(handle state)
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

    void insert_above(pointer position, handle state)
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

    void insert_below(const_pointer position, handle state)
    {
        const auto it{find(position)};

        value_type& new_state{*state};

        m_states.insert(it, std::move(state));
        new_state.entered(*this);
    }

    handle pop()
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

    handle remove(pointer position)
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

    handle remove_above(pointer position)
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

    handle remove_below(const_pointer position)
    {
        assert(is_bottom(position) && "cpt::state_stack::remove_above called on bottom state");

        const auto it{find(position) - 1};

        handle output{std::move(*it)};
        m_states.erase(it);

        return output;
    }

    void clear() noexcept
    {
        m_states.clear();
    }

    void reset(handle initial_state)
    {
        clear();
        push(std::move(initial_state));
    }

    void raise(pointer state)
    {
        const auto it{find(state)};

        value_type& new_top{*state};
        value_type& last_top{*m_states.back()};

        m_states.emplace_back(std::move(*it));
        m_states.erase(it);

        last_top.fell(*this);
        new_top.raised(*this);
    }

    void pop_until(pointer state)
    {
        while(!is_top(state))
        {
            pop();
        }
    }

    void update(float elapsed_time)
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

    void add_post_update_callback(post_update_callback_type callback)
    {
        m_post_update_callbacks.emplace_back(std::move(callback));
    }

    pointer neighbour_above(const_pointer state) noexcept
    {
        if(is_top(state))
        {
            return nullptr;
        }
        else
        {
            return (find(state) + 1)->get();
        }
    }

    const_pointer neighbour_above(const_pointer state) const noexcept
    {
        if(is_top(state))
        {
            return nullptr;
        }
        else
        {
            return (find(state) + 1)->get();
        }
    }

    pointer neighbour_below(const_pointer state) noexcept
    {
        if(is_bottom(state))
        {
            return nullptr;
        }
        else
        {
            return (find(state) - 1)->get();
        }
    }

    const_pointer neighbour_below(const_pointer state) const noexcept
    {
        if(is_bottom(state))
        {
            return nullptr;
        }
        else
        {
            return (find(state) - 1)->get();
        }
    }

    bool is_top(const_pointer state) const noexcept
    {
        return m_states.back().get() == state;
    }

    bool is_bottom(const_pointer state) const noexcept
    {
        return m_states.front().get() == state;
    }

    pointer top() noexcept
    {
        return m_states.back().get();
    }

    const_pointer top() const noexcept
    {
        return m_states.back().get();
    }

    bool empty() const noexcept
    {
        return std::empty(m_states);
    }

    std::size_t size() const noexcept
    {
        return std::size(m_states);
    }

private:
    typename std::vector<handle>::iterator find(const_pointer state) noexcept
    {
        const auto find_state = [state](const handle& other)
        {
            return state == other.get();
        };

        const auto it{std::find_if(std::begin(m_states), std::end(m_states), find_state)};
        assert(it != std::end(m_states) && "state_stack does not contains specified state");

        return it;
    }

    typename std::vector<handle>::const_iterator find(const_pointer state) const noexcept
    {
        const auto find_state = [state](const handle& other)
        {
            return state == other.get();
        };

        const auto it{std::find_if(std::begin(m_states), std::end(m_states), find_state)};
        assert(it != std::end(m_states) && "state_stack does not contains specified state");

        return it;
    }

private:
    std::vector<handle> m_states{};
    std::vector<post_update_callback_type> m_post_update_callbacks{};
};

}

#endif
