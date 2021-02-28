#ifndef CAPTAL_BINDING_HPP_INCLUDED
#define CAPTAL_BINDING_HPP_INCLUDED

#include "config.hpp"

#include <variant>
#include <vector>
#include <cassert>

#include <tephra/descriptor.hpp>

#include "uniform_buffer.hpp"
#include "texture.hpp"
#include "storage_buffer.hpp"

namespace cpt
{

using binding = std::variant<texture_ptr, uniform_buffer_ptr, storage_buffer_ptr>;

enum class binding_type : std::uint32_t
{
    texture = 0,
    uniform_buffer = 1,
    storage_buffer = 2
};

inline binding_type get_binding_type(const binding& binding) noexcept
{
    return static_cast<binding_type>(binding.index());
}

inline asynchronous_resource_ptr get_binding_resource(const binding& binding) noexcept
{
    return std::visit([](auto&& altenative) -> asynchronous_resource_ptr
    {
        return altenative;
    }, binding);
}

inline tph::descriptor_write make_descriptor_write(tph::descriptor_set& set, std::uint32_t binding, const cpt::binding& data) noexcept
{
    if(get_binding_type(data) == binding_type::uniform_buffer)
    {
        auto buffer{std::get<uniform_buffer_ptr>(data)->get_buffer()};

        const tph::descriptor_buffer_info info{buffer.buffer, buffer.offset, std::get<uniform_buffer_ptr>(data)->size()};

        return tph::descriptor_write{set, binding, 0, tph::descriptor_type::uniform_buffer, info};
    }
    else if(get_binding_type(data) == binding_type::texture)
    {
        const tph::descriptor_texture_info info{std::get<texture_ptr>(data)->get_texture(), tph::texture_layout::shader_read_only_optimal};

        return tph::descriptor_write{set, binding, 0, tph::descriptor_type::image_sampler, info};
    }
    else
    {
        const tph::descriptor_buffer_info info{std::get<storage_buffer_ptr>(data)->get_buffer(), 0, std::get<storage_buffer_ptr>(data)->size()};

        return tph::descriptor_write{set, binding, 0, tph::descriptor_type::storage_buffer, info};
    }
}

class binding_buffer
{
public:
    binding_buffer() = default;
    ~binding_buffer() = default;
    binding_buffer(const binding_buffer&) = delete;
    binding_buffer& operator=(const binding_buffer&) = delete;
    binding_buffer(binding_buffer&& other) noexcept = default;
    binding_buffer& operator=(binding_buffer&& other) noexcept = default;

    void set(std::uint32_t index, binding value)
    {
        assure(index);

        m_bindings[index] = std::move(value);
    }

    const binding& get(std::uint32_t index) const noexcept
    {
        assert(has(index) && "cpt::binding_buffer::get index out of range.");

        return m_bindings[index];
    }

    optional_ref<const binding> try_get(std::uint32_t index) const noexcept
    {
        if(has(index))
        {
            return m_bindings[index];
        }

        return nullref;
    }

    bool has(std::uint32_t index) const noexcept
    {
        return index < std::size(m_bindings) || get_binding_resource(m_bindings[index]) != nullptr;
    }

private:
    void assure(std::size_t index)
    {
        if(index >= std::size(m_bindings))
        {
            m_bindings.resize(index + 1);
        }
    }

private:
    std::vector<binding> m_bindings{};
};

}

#endif
