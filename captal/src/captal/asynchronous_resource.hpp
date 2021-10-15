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

#ifndef CAPTAL_ASYNCHRONOUS_RESOURCE_HPP_INCLUDED
#define CAPTAL_ASYNCHRONOUS_RESOURCE_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <vector>

namespace cpt
{

class CAPTAL_API asynchronous_resource
{
public:
    asynchronous_resource() noexcept = default;
    virtual ~asynchronous_resource() = default;

protected:
    asynchronous_resource(const asynchronous_resource&) = default;
    asynchronous_resource& operator=(const asynchronous_resource&) = default;
    asynchronous_resource(asynchronous_resource&& other) noexcept = default;
    asynchronous_resource& operator=(asynchronous_resource&& other) noexcept = default;
};

using asynchronous_resource_ptr = std::shared_ptr<asynchronous_resource>;
using asynchronous_resource_weak_ptr = std::weak_ptr<asynchronous_resource>;

class CAPTAL_API asynchronous_resource_keeper
{
public:
    asynchronous_resource_keeper() = default;
    ~asynchronous_resource_keeper() = default;
    asynchronous_resource_keeper(const asynchronous_resource_keeper&) = delete;
    asynchronous_resource_keeper& operator=(const asynchronous_resource_keeper&) = delete;
    asynchronous_resource_keeper(asynchronous_resource_keeper&&) noexcept = default;
    asynchronous_resource_keeper& operator=(asynchronous_resource_keeper&&) noexcept = default;

    template<typename T>
    void keep(T&& resource)
    {
        static_assert(!std::is_pointer_v<T>, "cpt::asynchronous_resource_keeper::keep called with raw pointer.");

        m_resources.emplace_back(std::forward<T>(resource));
    }

    template<std::input_iterator InputIt>
    void keep(InputIt begin, InputIt end)
    {
        static_assert(!std::is_pointer_v<typename std::iterator_traits<InputIt>::value_type>, "cpt::asynchronous_resource_keeper::keep called with raw pointer.");

        m_resources.insert(std::end(m_resources), begin, end);
    }

    void reserve(std::size_t size)
    {
        m_resources.reserve(std::size(m_resources) + size);
    }

    void clear() noexcept
    {
        m_resources.clear();
    }

private:
    std::vector<asynchronous_resource_ptr> m_resources{};
};

}

#endif
