#ifndef CAPTAL_UNIFORM_BUFFER_HPP_INCLUDED
#define CAPTAL_UNIFORM_BUFFER_HPP_INCLUDED

#include "config.hpp"

#include "framed_buffer.hpp"

namespace cpt
{

class uniform_buffer
{
public:
    template<typename T>
    uniform_buffer(std::uint32_t set, std::uint32_t binding, T&& data)
    :m_buffer{{buffer_part{buffer_part_type::uniform, sizeof(data)}}}
    ,m_set{set}
    ,m_binding{binding}
    {

    }

    template<typename T>
    T& get() noexcept
    {
        return m_buffer.get<T>(0);
    }

    template<typename T>
    const T& get() const noexcept
    {
        return m_buffer.get<T>(0);
    }

    template<typename T>
    void set(T&& data) noexcept(std::is_nothrow_assignable<T&, decltype(data)>::value)
    {
        m_buffer.get<T>(0) = std::forward<T>(data);
    }

    framed_buffer& buffer() noexcept
    {
        return m_buffer;
    }

    const framed_buffer& buffer() const noexcept
    {
        return m_buffer;
    }

    std::uint32_t set() const noexcept
    {
        return m_set;
    }

    std::uint32_t binding() const noexcept
    {
        return m_binding;
    }

    void upload() noexcept
    {
        m_buffer.upload();
    }

private:
    framed_buffer m_buffer;
    std::uint32_t m_set{};
    std::uint32_t m_binding{};
};

}

#endif
