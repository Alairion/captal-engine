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
        return *m_attachment;
    }

    const_reference attachment() const noexcept
    {
        return *m_attachment;
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
        return *m_attachment;
    }

    const_reference operator*() const noexcept
    {
        return *m_attachment;
    }

    pointer operator->() noexcept
    {
        return &(*m_attachment);
    }

    const_pointer operator->() const noexcept
    {
        return &(*m_attachment);
    }

private:
    std::optional<value_type> m_attachment{};
};

}

#endif
