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

#ifndef CAPTAL_UNIFORM_BUFFER_HPP_INCLUDED
#define CAPTAL_UNIFORM_BUFFER_HPP_INCLUDED

#include "config.hpp"

#include <vector>
#include <cstring>
#include <memory>

#include <tephra/buffer.hpp>

#include "asynchronous_resource.hpp"
#include "signal.hpp"
#include "buffer_pool.hpp"

namespace cpt
{

enum class buffer_part_type : std::uint32_t
{
    uniform = 0,
    index = 1,
    vertex = 2,
};

struct buffer_part
{
    buffer_part_type type{};
    std::uint64_t size{};
};

class CAPTAL_API uniform_buffer final : public asynchronous_resource
{
public:
    struct buffer_info
    {
        tph::buffer&  buffer;
        std::uint64_t offset{};
    };

private:
    struct buffer_part_info
    {
        std::uint64_t offset{};
        std::uint64_t size{};
    };

public:
    uniform_buffer() = default;
    explicit uniform_buffer(std::span<const buffer_part> parts);

    ~uniform_buffer() = default;
    uniform_buffer(const uniform_buffer&) = delete;
    uniform_buffer& operator=(const uniform_buffer&) = delete;
    uniform_buffer(uniform_buffer&&) noexcept = default;
    uniform_buffer& operator=(uniform_buffer&&) noexcept = default;

    void upload();
    void upload(std::size_t index);

    template<typename T>
    T& get(std::size_t index) noexcept
    {
        return *std::launder(reinterpret_cast<T*>(m_buffer.map(m_parts[index].offset)));
    }

    template<typename T>
    const T& get(std::size_t index) const noexcept
    {
        return *std::launder(reinterpret_cast<const T*>(m_buffer.map(m_parts[index].offset)));
    }

    std::uint64_t part_offset(std::size_t index) const noexcept
    {
        return m_parts[index].offset;
    }

    std::uint64_t part_size(std::size_t index) const noexcept
    {
        return m_parts[index].size;
    }

    buffer_info get_buffer() noexcept
    {
        return buffer_info{m_buffer.heap().buffer(), m_buffer.offset()};
    }

    std::uint64_t size() const noexcept
    {
        return m_buffer.size();
    }

private:
    static std::vector<buffer_part_info> compute_part_info(std::span<const buffer_part> parts);

private:
    std::vector<buffer_part_info> m_parts{};
    buffer_heap_chunk m_buffer{};
};

using uniform_buffer_ptr = std::shared_ptr<uniform_buffer>;
using uniform_buffer_weak_ptr = std::weak_ptr<uniform_buffer>;

template<typename... Args>
uniform_buffer_ptr make_uniform_buffer(Args&&... args)
{
    return std::make_shared<uniform_buffer>(std::forward<Args>(args)...);
}

}

#endif
