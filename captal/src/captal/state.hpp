#ifndef CAPTAL_STATE_HPP_INCLUDED
#define CAPTAL_STATE_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <vector>
#include <functional>

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

    virtual void entered(state_stack& stack);
    virtual void raised(state_stack& stack);
    virtual void fell(state_stack& stack);
    virtual void leaved(state_stack& stack);

    virtual void update(state_stack& stack, float elapsed_time) = 0;
};

class CAPTAL_API state_stack
{
public:
    using value_type = cpt::state;
    using handle = std::unique_ptr<value_type>;
    using pointer = cpt::state*;
    using const_pointer = const cpt::state*;
    using reference = cpt::state&;
    using const_reference = const cpt::state&;
    using post_update_callback_type = std::function<void(state_stack& stack)>;

public:
    state_stack() = default;
    state_stack(handle initial_state);
    ~state_stack() = default;
    state_stack(const state_stack&) = delete;
    state_stack& operator=(const state_stack&) = delete;
    state_stack(state_stack&& other) noexcept = default;
    state_stack& operator=(state_stack&& other) noexcept = default;

    void push(handle state);
    void insert_above(pointer position, handle state);
    void insert_below(const_pointer position, handle state);

    handle pop();
    handle remove(pointer position);
    handle remove_above(pointer position);
    handle remove_below(const_pointer position);

    void clear();
    void reset(handle initial_state);

    void raise(pointer state);
    void pop_until(pointer state);

    void update(float elapsed_time);
    void add_post_update_callback(post_update_callback_type callback);

    bool is_top(const_pointer state) const noexcept
    {
        return m_states.back().get() == state;
    }

    cpt::state& top() noexcept
    {
        return *m_states.back();
    }

    const cpt::state& top() const noexcept
    {
        return *m_states.back();
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
    std::vector<handle>::iterator find(const_pointer state) noexcept;
    std::vector<handle>::const_iterator find(const_pointer state) const noexcept;

private:
    std::vector<handle> m_states{};
    std::vector<post_update_callback_type> m_post_update_callbacks{};
};

}

#endif
