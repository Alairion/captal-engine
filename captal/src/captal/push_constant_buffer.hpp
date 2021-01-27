#ifndef CAPTAL_PUSH_CONSTANT_BUFFER_HPP_INCLUDED
#define CAPTAL_PUSH_CONSTANT_BUFFER_HPP_INCLUDED

#include "config.hpp"

#include <array>
#include <vector>
#include <variant>

namespace cpt
{

class push_constants_buffer
{
public:
    static constexpr std::size_t sbo_size{24};

private:
    using static_buffer_type = std::array<std::uint32_t, sbo_size / 4>;
    using dynamic_buffer_type = std::vector<std::uint32_t>;

public:
    push_constants_buffer() = default;
    ~push_constants_buffer() = default;
    push_constants_buffer(const push_constants_buffer&) = delete;
    push_constants_buffer& operator=(const push_constants_buffer&) = delete;
    push_constants_buffer(push_constants_buffer&& other) noexcept = default;
    push_constants_buffer& operator=(push_constants_buffer&& other) noexcept = default;

    template<typename T>
    void set_push_constant(std::size_t index, T&& value) noexcept
    {
        get_push_constant<T>(index) = std::forward<T>(value);
    }

    template<typename T>
    T& get_push_constant(std::size_t index) noexcept
    {
        const auto range{m_render_technique->ranges()[index]};
        assert(range.size == sizeof(T) && "Size of T does not match range size.");

        return *std::launder(reinterpret_cast<T*>(std::data(m_push_constant_buffer) + range.offset / 4u));
    }

    template<typename T>
    const T& get_push_constant(std::size_t index) const noexcept
    {
        const auto range{m_render_technique->ranges()[index]};
        assert(range.size == sizeof(T) && "Size of T does not match range size.");

        return *std::launder(reinterpret_cast<const T*>(std::data(m_push_constant_buffer) + range.offset / 4u));
    }

private:
    std::uint32_t*

private:
    std::variant<static_buffer_type, dynamic_buffer_type> m_data{};
};

}

#endif
