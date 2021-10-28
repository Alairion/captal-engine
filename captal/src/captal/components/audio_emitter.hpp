//MIT License
//
//Copyright (c) 2021 Alexy Pellegrini
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#ifndef CAPTAL_COMPONENTS_AUDIO_EMITTER_HPP_INCLUDED
#define CAPTAL_COMPONENTS_AUDIO_EMITTER_HPP_INCLUDED

#include "../config.hpp"

#include <optional>

#include "../sound.hpp"

namespace cpt::components
{

class audio_emitter
{
public:
    audio_emitter() = default;

    template<typename... Args>
    explicit audio_emitter(Args&&... args) noexcept(std::is_nothrow_constructible_v<sound, Args...>)
    :m_attachment{std::in_place, std::forward<Args>(args)...}
    {

    }

    ~audio_emitter() = default;
    audio_emitter(const audio_emitter&) = delete;
    audio_emitter& operator=(const audio_emitter&) = delete;
    audio_emitter(audio_emitter&&) noexcept = default;
    audio_emitter& operator=(audio_emitter&&) noexcept = default;

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

    void swap(audio_emitter& other) noexcept
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
