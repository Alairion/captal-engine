#ifndef CAPTAL_BINDING_HPP_INCLUDED
#define CAPTAL_BINDING_HPP_INCLUDED

#include "config.hpp"

#include <variant>

#include "uniform_buffer.hpp"
#include "texture.hpp"
#include "storage_buffer.hpp"

namespace cpt
{

using binding = std::variant<uniform_buffer_ptr, texture_ptr, storage_buffer_ptr>;

enum class binding_type : std::uint32_t
{
    uniform_buffer = 0,
    texture = 1,
    storage_buffer = 2
};

inline binding_type get_binding_type(const binding& binding) noexcept
{
    return static_cast<binding_type>(binding.index());
}

}

#endif
