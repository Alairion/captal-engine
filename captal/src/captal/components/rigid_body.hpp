#ifndef CAPTAL_COMPONENTS_RIGID_BODY_HPP_INCLUDED
#define CAPTAL_COMPONENTS_RIGID_BODY_HPP_INCLUDED

#include "../config.hpp"

#include <optional>
#include <vector>

#include "../physics.hpp"

namespace cpt::components
{

class rigid_body : private std::vector<physical_shape>
{
    using parent_type = std::vector<physical_shape>;

public:
    using value_type = parent_type::value_type;
    using size_type = parent_type::size_type;
    using difference_type = parent_type::difference_type;
    using pointer = parent_type::pointer;
    using const_pointer = parent_type::const_pointer;
    using reference = parent_type::reference;
    using const_reference = parent_type::const_reference;
    using iterator = parent_type::iterator;
    using const_iterator = parent_type::const_iterator;
    using reverse_iterator = parent_type::reverse_iterator;
    using const_reverse_iterator = parent_type::const_reverse_iterator;

public:
    rigid_body() = default;

    template<typename... Args> requires std::constructible_from<physical_body, Args...>
    explicit rigid_body(Args&&... args) noexcept(std::is_nothrow_constructible_v<physical_body, Args...>)
    :m_attachment{std::in_place, std::forward<Args>(args)...}
    {

    }

    ~rigid_body() = default;
    rigid_body(const rigid_body&) = delete;
    rigid_body& operator=(const rigid_body&) = delete;
    rigid_body(rigid_body&&) noexcept = default;
    rigid_body& operator=(rigid_body&&) noexcept = default;

    template<typename... Args> requires std::constructible_from<physical_body, Args...>
    physical_body& attach(Args&&... args) noexcept(std::is_nothrow_constructible_v<physical_body, Args...>)
    {
        return m_attachment.emplace(std::forward<Args>(args)...);
    }

    template<typename... Args> requires std::constructible_from<physical_shape, physical_body&, Args...>
    physical_shape& attach_shape(Args&&... args) noexcept(std::is_nothrow_constructible_v<physical_shape, physical_body&, Args...>)
    {
        return emplace_back(attachment(), std::forward<Args>(args)...);
    }

    void detach() noexcept
    {
        clear();
        m_attachment.reset();
    }

    void detach_shape(const_iterator position) noexcept
    {
        erase(position);
    }

    void detach_shapes() noexcept
    {
        clear();
    }

    physical_body& attachment() noexcept
    {
        return m_attachment.value();
    }

    const physical_body& attachment() const noexcept
    {
        return m_attachment.value();
    }

    bool has_attachment() const noexcept
    {
        return m_attachment.has_value();
    }

    void swap(rigid_body& other) noexcept
    {
        parent_type::swap(other);
        m_attachment.swap(other.m_attachment);
    }

    explicit operator bool() const noexcept
    {
        return m_attachment.has_value();
    }

    physical_body& operator*() noexcept
    {
        return m_attachment.value();
    }

    const physical_body& operator*() const noexcept
    {
        return m_attachment.value();
    }

    physical_body* operator->() noexcept
    {
        return &m_attachment.value();
    }

    const physical_body* operator->() const noexcept
    {
        return &m_attachment.value();
    }

    using parent_type::operator[];
    using parent_type::front;
    using parent_type::back;
    using parent_type::data;

    using parent_type::empty;
    using parent_type::size;
    using parent_type::max_size;
    using parent_type::reserve;
    using parent_type::capacity;
    using parent_type::shrink_to_fit;

    using parent_type::begin;
    using parent_type::cbegin;
    using parent_type::end;
    using parent_type::cend;
    using parent_type::rbegin;
    using parent_type::crbegin;
    using parent_type::rend;
    using parent_type::crend;

private:
    std::optional<physical_body> m_attachment{};
};

}

#endif
