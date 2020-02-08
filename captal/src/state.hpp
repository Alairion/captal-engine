#ifndef CAPTAL_STATE_HPP_INCLUDED
#define CAPTAL_STATE_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <vector>

namespace cpt
{

class state_stack;

class CAPTAL_API state
{
public:
    state() noexcept = default;
    virtual ~state() = default;
    state(const state&) = delete;
    state& operator=(const state&) = delete;
    state(state&& other) noexcept = delete;
    state& operator=(state&& other) noexcept = delete;

    void enter(state_stack& stack)
    {
        on_enter(stack);
    }

    void leave(state_stack& stack)
    {
        on_leave(stack);
    }

    void update(state_stack& stack, float elapsed_time)
    {
        on_update(stack, elapsed_time);
    }

protected:
    virtual void on_enter(state_stack& stack) = 0;
    virtual void on_leave(state_stack& stack) = 0;
    virtual void on_update(state_stack& stack, float elapsed_time) = 0;
};

using state_ptr = std::shared_ptr<state>;

template<typename T, typename... Args>
std::shared_ptr<T> make_state(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

class CAPTAL_API state_stack
{
public:
    state_stack() = default;
    state_stack(state_ptr initial_state);
    ~state_stack() = default;
    state_stack(const state_stack&) = delete;
    state_stack& operator=(const state_stack&) = delete;
    state_stack(state_stack&& other) noexcept = default;
    state_stack& operator=(state_stack&& other) noexcept = default;

    void push(state_ptr state);
    state_ptr pop();
    void reset(state_ptr initial_state);

    void pop_until(state_ptr state);
    void pop_until(state* state);
    void raise(state_ptr state);
    void raise(state* state);

    void update(float elapsed_time);

    bool is_top(const state* state) const;
    bool is_top(state_ptr state) const;
    state_ptr current() const;

    bool empty() const noexcept
    {
        return std::empty(m_states);
    }

private:
    std::vector<state_ptr> m_states{};
};

}

#endif
