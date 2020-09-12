#ifndef CAPTAL_COMPONENTS_CAMERA_HPP_INCLUDED
#define CAPTAL_COMPONENTS_CAMERA_HPP_INCLUDED

#include "../config.hpp"

#include <optional>

#include "../view.hpp"

namespace cpt::components
{

class camera
{
public:
    camera() = default;

    template<typename... Args>
    explicit camera(Args&&... args) noexcept(std::is_nothrow_constructible_v<view, Args...>)
    :m_attachment{std::in_place, std::forward<Args>(args)...}
    {

    }

    ~camera() = default;
    camera(const camera&) = delete;
    camera& operator=(const camera&) = delete;
    camera(camera&&) noexcept = default;
    camera& operator=(camera&&) noexcept = default;

    template<typename... Args>
    view& attach(Args&&... args) noexcept(std::is_nothrow_constructible_v<view, Args...>)
    {
        return m_attachment.emplace(std::forward<Args>(args)...);
    }

    void detach() noexcept
    {
        m_attachment.reset();
    }

    view& attachment() noexcept
    {
        return m_attachment.value();
    }

    const view& attachment() const noexcept
    {
        return m_attachment.value();
    }

    bool has_attachment() const noexcept
    {
        return m_attachment.has_value();
    }

    void swap(camera& other) noexcept
    {
        m_attachment.swap(other.m_attachment);
    }

    explicit operator bool() const noexcept
    {
        return m_attachment.has_value();
    }

    view& operator*() noexcept
    {
        return m_attachment.value();
    }

    const view& operator*() const noexcept
    {
        return m_attachment.value();
    }

    view* operator->() noexcept
    {
        return &m_attachment.value();
    }

    const view* operator->() const noexcept
    {
        return &m_attachment.value();
    }

private:
    std::optional<view> m_attachment{};
};

}

#endif
