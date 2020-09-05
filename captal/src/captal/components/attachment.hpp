#ifndef CAPTAL_COMPONENTS_ATTACHMENT_HPP_INCLUDED
#define CAPTAL_COMPONENTS_ATTACHMENT_HPP_INCLUDED

#include "../config.hpp"

#include <concepts>
#include <optional>

namespace cpt::components::impl
{

template<typename T>
class basic_attachement
{
public:
    using value_type = T;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;

public:
    basic_attachement() = default;

    template<typename... Args> requires std::constructible_from<value_type, Args...>
    explicit basic_attachement(Args&&... args) noexcept(std::is_nothrow_constructible_v<value_type, Args...>)
    :m_attachment{std::in_place, std::forward<Args>(args)...}
    {

    }

    ~basic_attachement() = default;
    basic_attachement(const basic_attachement&) = delete;
    basic_attachement& operator=(const basic_attachement&) = delete;
    basic_attachement(basic_attachement&&) noexcept = default;
    basic_attachement& operator=(basic_attachement&&) noexcept = default;

    template<typename... Args> requires std::constructible_from<value_type, Args...>
    void attach(Args&&... args) noexcept(std::is_nothrow_constructible_v<value_type, Args...>)
    {
        m_attachment.emplace(std::forward<Args>(args)...);
    }

    void detach() noexcept
    {
        m_attachment.reset();
    }

    reference attachment() noexcept
    {
        return m_attachment.value();
    }

    const_reference attachment() const noexcept
    {
        return m_attachment.value();
    }

    bool has_attachment() const noexcept
    {
        return m_attachment.has_value();
    }

    void swap(basic_attachement& other) noexcept(m_attachment.swap(other.m_attachment))
    {
        m_attachment.swap(other.m_attachment);
    }

    explicit operator bool() const noexcept
    {
        return m_attachment.has_value();
    }

    reference operator*() noexcept
    {
        return m_attachment.value();
    }

    const_reference operator*() const noexcept
    {
        return m_attachment.value();
    }

    pointer operator->() noexcept
    {
        return &m_attachment.value();
    }

    const_pointer operator->() const noexcept
    {
        return &m_attachment.value();
    }

private:
    std::optional<value_type> m_attachment{};
};

}

#endif
