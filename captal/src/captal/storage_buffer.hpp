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

#ifndef CAPTAL_STORAGE_BUFFER_HPP_INCLUDED
#define CAPTAL_STORAGE_BUFFER_HPP_INCLUDED

#include "config.hpp"

#include <memory>

#include <tephra/buffer.hpp>

#include "asynchronous_resource.hpp"

namespace cpt
{

class CAPTAL_API storage_buffer final : public asynchronous_resource
{
public:
    storage_buffer() = default;
    explicit storage_buffer(std::uint64_t size, tph::buffer_usage usage = tph::buffer_usage{});

    ~storage_buffer() = default;
    storage_buffer(const storage_buffer&) = delete;
    storage_buffer& operator=(const storage_buffer&) = delete;
    storage_buffer(storage_buffer&&) noexcept = default;
    storage_buffer& operator=(storage_buffer&&) noexcept = default;

    std::uint64_t size() const noexcept
    {
        return m_buffer.size();
    }

    tph::buffer& get_buffer() noexcept
    {
        return m_buffer;
    }

    const tph::buffer& get_buffer() const noexcept
    {
        return m_buffer;
    }

#ifdef CAPTAL_DEBUG
    void set_name(std::string_view name);
#else
    void set_name(std::string_view name [[maybe_unused]]) const noexcept
    {

    }
#endif

private:
    tph::buffer m_buffer{};
};

using storage_buffer_ptr = std::shared_ptr<storage_buffer>;
using storage_buffer_weak_ptr = std::weak_ptr<storage_buffer>;

template<typename... Args>
storage_buffer_ptr make_storage_buffer(Args&&... args)
{
    return std::make_shared<storage_buffer>(std::forward<Args>(args)...);
}

}

#endif
