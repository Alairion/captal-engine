#ifndef CAPTAL_UNIFORM_BUFFER_HPP_INCLUDED
#define CAPTAL_UNIFORM_BUFFER_HPP_INCLUDED

#include "config.hpp"

#include <variant>

#include "framed_buffer.hpp"
#include "texture.hpp"

namespace cpt
{

using uniform_binding = std::variant<framed_buffer_ptr, texture_ptr>;

enum class uniform_binding_type : std::size_t
{
    buffer = 0,
    texture = 1
};

uniform_binding_type get_uniform_binding_type(const uniform_binding& binding)
{
    return static_cast<uniform_binding_type>(binding.index());
}

}

#endif
