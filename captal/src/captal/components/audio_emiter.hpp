#ifndef CAPTAL_COMPONENTS_AUDIO_EMITER_HPP_INCLUDED
#define CAPTAL_COMPONENTS_AUDIO_EMITER_HPP_INCLUDED

#include "../config.hpp"

#include <optional>

#include "../sound.hpp"

namespace cpt::components
{

class audio_emiter
{
public:
    audio_emiter() = default;

    template<typename... Args>
    explicit audio_emiter(Args&&... args) noexcept(std::is_nothrow_constructible_v<sound, Args...>)
    :m_attachment{std::in_place, std::forward<Args>(args)...}
    {

    }

    ~audio_emiter() = default;
    audio_emiter(const audio_emiter&) = delete;
    audio_emiter& operator=(const audio_emiter&) = delete;
    audio_emiter(audio_emiter&&) noexcept = default;
    audio_emiter& operator=(audio_emiter&&) noexcept = default;

    template<typename... Args>
    sound& attach(Args&&... args) noexcept(std::is_nothrow_constructible_v<sound, Args...>)
    {
        return m_attachment.emplace(std::forward<Args>(args)...);
    }

    void detach() noexcept
    {
        m_attachment.reset();
    }

    sound& attachment() noexcept
    {
        return *m_attachment;
    }

    const sound& attachment() const noexcept
    {
        return *m_attachment;
    }

    bool has_attachment() const noexcept
    {
        return m_attachment.has_value();
    }

    void swap(audio_emiter& other) noexcept
    {
        m_attachment.swap(other.m_attachment);
    }

    explicit operator bool() const noexcept
    {
        return m_attachment.has_value();
    }

    sound& operator*() noexcept
    {
        return *m_attachment;
    }

    const sound& operator*() const noexcept
    {
        return *m_attachment;
    }

    sound* operator->() noexcept
    {
        return &(*m_attachment);
    }

    const sound* operator->() const noexcept
    {
        return &(*m_attachment);
    }

private:
    std::optional<sound> m_attachment{};
};

}

#endif
