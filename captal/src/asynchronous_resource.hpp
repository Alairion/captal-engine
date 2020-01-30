#ifndef CAPTAL_ASYNCHRONOUS_RESOURCE_HPP_INCLUDED
#define CAPTAL_ASYNCHRONOUS_RESOURCE_HPP_INCLUDED

#include "config.hpp"

namespace cpt
{

class asynchronous_resource
{
public:
    asynchronous_resource() noexcept = default;
    virtual ~asynchronous_resource() = default;
    asynchronous_resource(const asynchronous_resource&) = default;
    asynchronous_resource& operator=(const asynchronous_resource&) = default;
    asynchronous_resource(asynchronous_resource&& other) noexcept = default;
    asynchronous_resource& operator=(asynchronous_resource&& other) noexcept = default;
};

}

#endif
