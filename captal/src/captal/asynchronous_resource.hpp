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
        m_resources.emplace_back(std::forward<T>(resource));
    }

    void reserve(std::size_t size)
    {
        m_resources.reserve(size);
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
