#ifndef CAPTAL_FRAMED_BUFFER_HPP_INCLUDED
#define CAPTAL_FRAMED_BUFFER_HPP_INCLUDED

#include "config.hpp"

#include <vector>
#include <cstring>

#include <tephra/buffer.hpp>

#include "engine.hpp"

namespace cpt
{

enum class buffer_part_type
{
    uniform = 0,
    index = 1,
    vertex = 2,
};

struct buffer_part
{
    buffer_part_type type{};
    std::uint64_t size{};
};

class CAPTAL_API framed_buffer
{
public:
    framed_buffer() = default;
    framed_buffer(std::vector<buffer_part> parts);
    ~framed_buffer() = default;
    framed_buffer(const framed_buffer&) = delete;
    framed_buffer& operator=(const framed_buffer&) = delete;
    framed_buffer(framed_buffer&&) noexcept = default;
    framed_buffer& operator=(framed_buffer&&) noexcept = default;

    void upload();

    template<typename T>
    T& get(std::size_t index) noexcept
    {
        return *reinterpret_cast<T*>(std::data(m_data) + compute_offset(index));
    }

    template<typename T>
    const T& get(std::size_t index) const noexcept
    {
        return *reinterpret_cast<const T*>(std::data(m_data) + compute_offset(index));
    }

    std::uint64_t compute_offset(std::size_t index) const noexcept
    {
        const auto align_up = [](std::uint64_t offset, std::uint64_t alignment) noexcept -> std::uint64_t
        {
            return (offset + alignment - 1) & ~(alignment - 1);
        };

        const std::uint64_t uniform_alignment{engine::instance().graphics_device().limits().min_uniform_buffer_alignment};

        std::uint64_t offset{};
        for(std::size_t i{}; i < index; ++i)
        {
            if(m_parts[i].type == buffer_part_type::uniform)
            {
                offset = align_up(offset, uniform_alignment) + m_parts[i].size;
            }
            else
            {
                offset += m_parts[i].size;
            }
        }

        return offset;
    }

    tph::buffer& buffer() noexcept
    {
        return m_device_buffer;
    }

    const tph::buffer& buffer() const noexcept
    {
        return m_device_buffer;
    }

    std::uint64_t size() const noexcept
    {
        return m_size;
    }

private:
    struct staging_buffer
    {
        tph::buffer buffer{};
        bool available{true};
    };

private:
    std::uint64_t m_size{};
    std::vector<buffer_part> m_parts{};
    std::vector<std::uint8_t> m_data{}; //cpu local copy for host manipulation
    std::vector<staging_buffer> m_stagings{}; //cpu-gpu-shared buffers, stay alive waiting for transfers
    tph::buffer m_device_buffer{}; //the gpu buffer
};

}

#endif
