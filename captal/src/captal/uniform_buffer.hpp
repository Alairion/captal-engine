#ifndef CAPTAL_UNIFORM_BUFFER_HPP_INCLUDED
#define CAPTAL_UNIFORM_BUFFER_HPP_INCLUDED

#include "config.hpp"

#include <vector>
#include <cstring>
#include <memory>

#include <tephra/buffer.hpp>

#include "asynchronous_resource.hpp"
#include "signal.hpp"
#include "memory_transfer.hpp"

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

class CAPTAL_API uniform_buffer : public asynchronous_resource
{
public:
    uniform_buffer() = default;
    explicit uniform_buffer(std::vector<buffer_part> parts);

    ~uniform_buffer() = default;
    uniform_buffer(const uniform_buffer&) = delete;
    uniform_buffer& operator=(const uniform_buffer&) = delete;
    uniform_buffer(uniform_buffer&&) noexcept = default;
    uniform_buffer& operator=(uniform_buffer&&) noexcept = default;

    void upload(tph::command_buffer& command_buffer, transfer_ended_signal& signal);

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

    std::uint64_t compute_offset(std::size_t index) const noexcept;

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

#ifdef CAPTAL_DEBUG
    void set_name(std::string_view name);
#else
    void set_name(std::string_view name [[maybe_unused]]) const noexcept
    {

    }
#endif

private:
    struct staging_buffer
    {
        tph::buffer buffer{};
        bool available{true};
        cpt::scoped_connection connection{};
    };

private:
    std::uint64_t m_size{};
    std::vector<buffer_part> m_parts{};
    std::vector<std::uint8_t> m_data{}; //cpu local copy for host manipulation
    std::vector<staging_buffer> m_stagings{}; //cpu-gpu-shared buffers, stay alive waiting for transfers
    tph::buffer m_device_buffer{}; //the gpu buffer

#ifdef CAPTAL_DEBUG
    std::string m_name{};
#endif
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
