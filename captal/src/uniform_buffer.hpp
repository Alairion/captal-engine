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
    uniform_buffer(std::uint32_t binding, const T& data)
    :m_buffer{{buffer_part{buffer_part_type::uniform, sizeof(data)}}}
    ,m_binding{binding}
    {
        m_buffer.get<T>(0) = data;
    }

    ~uniform_buffer() = default;
    uniform_buffer(const uniform_buffer&) = delete;
    uniform_buffer& operator=(const uniform_buffer&) = delete;
    uniform_buffer(uniform_buffer&&) noexcept = default;
    uniform_buffer& operator=(uniform_buffer&&) noexcept = default;

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
    void set(const T& data) noexcept
    {
        m_buffer.get<T>(0) = data;
    }

    framed_buffer& buffer() noexcept
    {
        return m_buffer;
    }

    const framed_buffer& buffer() const noexcept
    {
        return m_buffer;
    }

    std::uint32_t binding() const noexcept
    {
        return m_binding;
    }

    void upload()
    {
        m_buffer.upload();
    }

private:
    framed_buffer m_buffer;
    std::uint32_t m_binding{};
};

}

#endif
