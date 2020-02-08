#ifndef CAPTAL_COMPONENTS_AUDIO_EMITER_HPP_INCLUDED
#define CAPTAL_COMPONENTS_AUDIO_EMITER_HPP_INCLUDED

#include "../config.hpp"

#include "../sound.hpp"

namespace cpt
{

namespace components
{

class audio_emiter
{
public:
    audio_emiter() = default;
    audio_emiter(sound_ptr attachment)
    :m_attachment{std::move(attachment)}
    {

    }

    ~audio_emiter() = default;
    audio_emiter(const audio_emiter&) = default;
    audio_emiter& operator=(const audio_emiter&) = default;
    audio_emiter(audio_emiter&&) noexcept = default;
    audio_emiter& operator=(audio_emiter&&) noexcept = default;

    void attach(sound_ptr attachment) noexcept
    {
        m_attachment = std::move(attachment);
    }

    void detach() noexcept
    {
        m_attachment = nullptr;
    }

    const sound_ptr& attachment() const noexcept
    {
        return m_attachment;
    }

private:
    sound_ptr m_attachment{};
};

}

}

#endif
