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
        return *m_attachment;
    }

    const view& attachment() const noexcept
    {
        return *m_attachment;
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
        return *m_attachment;
    }

    const view& operator*() const noexcept
    {
        return *m_attachment;
    }

    view* operator->() noexcept
    {
        return &(*m_attachment);
    }

    const view* operator->() const noexcept
    {
        return &(*m_attachment);
    }

private:
    std::optional<view> m_attachment{};
};

}

#endif
