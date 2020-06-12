#ifndef CAPTAL_ASYNCHRONOUS_RESOURCE_HPP_INCLUDED
#define CAPTAL_ASYNCHRONOUS_RESOURCE_HPP_INCLUDED

#include "config.hpp"

#include <memory>

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

template<typename... Args>
asynchronous_resource_ptr make_asynchronous_resource(Args&&... args)
{
    return std::make_shared<asynchronous_resource>(std::forward<Args>(args)...);
}

}

#endif
