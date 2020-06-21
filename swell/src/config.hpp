#ifndef SWELL_CONFIG_HPP_INCLUDED
#define SWELL_CONFIG_HPP_INCLUDED

#include <chrono>

#include <captal_foundation/base.hpp>
#include <captal_foundation/enum_operations.hpp>

#define SWELL_API CAPTAL_FOUNDATION_API

namespace swl
{

using namespace cpt::foundation;

using time_type = std::chrono::duration<double>;

}

#endif
