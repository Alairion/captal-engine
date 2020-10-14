#ifndef CAPTAL_UNIFORM_BUFFER_HPP_INCLUDED
#define CAPTAL_UNIFORM_BUFFER_HPP_INCLUDED

#include "config.hpp"

#include <vector>
#include <cstring>
#include <memory>

#include <tephra/buffer.hpp>

#include "asynchronous_resource.hpp"
#include "signal.hpp"

namespace cpt
{

enum class buffer_part_type : std::uint32_t
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

class CAPTAL_API uniform_buffer : public std::enable_shared_from_this<uniform_buffer>, public asynchronous_resource
{
public:
    uniform_buffer() = default;

    explicit uniform_buffer(std::vector<buffer_part> parts);

    template<typename T>
    explicit uniform_buffer(T&& data)
    :uniform_buffer{{buffer_part{buffer_part_type::uniform, sizeof(T)}}}
    {
        get<T>(0) = std::forward<T>(data);
    }

    ~uniform_buffer();
    uniform_buffer(const uniform_buffer&) = delete;
    uniform_buffer& operator=(const uniform_buffer&) = delete;
    uniform_buffer(uniform_buffer&&) noexcept = default;
    uniform_buffer& operator=(uniform_buffer&&) noexcept = default;

    void upload();

    template<typename T>
    T& get(std::size_t index) noexcept
    {
        return *std::launder(reinterpret_cast<T*>(std::data(m_data) + compute_offset(index)));
    }

    template<typename T>
    const T& get(std::size_t index) const noexcept
    {
        return *std::launder(reinterpret_cast<const T*>(std::data(m_data) + compute_offset(index)));
    }

    std::uint64_t compute_offset(std::size_t index) const noexcept
    {
        const std::uint64_t uniform_alignment{uniform_alignement()};

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

    tph::buffer& get_buffer() noexcept
    {
        return m_device_buffer;
    }

    const tph::buffer& get_buffer() const noexcept
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
        cpt::scoped_connection connection{};
    };

private:
    static std::uint64_t uniform_alignement();

private:
    std::uint64_t m_size{};
    std::vector<buffer_part> m_parts{};
    std::vector<std::uint8_t> m_data{}; //cpu local copy for host manipulation
    std::vector<staging_buffer> m_stagings{}; //cpu-gpu-shared buffers, stay alive waiting for transfers
    tph::buffer m_device_buffer{}; //the gpu buffer
};

using uniform_buffer_ptr = std::shared_ptr<uniform_buffer>;
using uniform_buffer_weak_ptr = std::weak_ptr<uniform_buffer>;

template<typename... Args>
uniform_buffer_ptr make_uniform_buffer(Args&&... args)
{
    return std::make_shared<uniform_buffer>(std::forward<Args>(args)...);
}

}

#endif
