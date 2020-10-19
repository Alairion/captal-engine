#ifndef CAPTAL_STORAGE_BUFFER_HPP_INCLUDED
#define CAPTAL_STORAGE_BUFFER_HPP_INCLUDED

#include "config.hpp"

#include <memory>

#include <tephra/buffer.hpp>

#include "asynchronous_resource.hpp"

namespace cpt
{

class CAPTAL_API storage_buffer : public asynchronous_resource
{
public:
    storage_buffer() = default;
    explicit storage_buffer(std::uint64_t size, tph::buffer_usage usage = tph::buffer_usage{});

    ~storage_buffer() = default;
    storage_buffer(const storage_buffer&) = delete;
    storage_buffer& operator=(const storage_buffer&) = delete;
    storage_buffer(storage_buffer&&) noexcept = default;
    storage_buffer& operator=(storage_buffer&&) noexcept = default;

    std::uint64_t size() const noexcept
    {
        return m_buffer.size();
    }

    tph::buffer& get_buffer() noexcept
    {
        return m_buffer;
    }

    const tph::buffer& get_buffer() const noexcept
    {
        return m_buffer;
    }

#ifdef CAPTAL_DEBUG
    void set_name(std::string_view name);
#else
    void set_name(std::string_view name [[maybe_unused]]) const noexcept
    {

    }
#endif

private:
    tph::buffer m_buffer{};
};

using storage_buffer_ptr = std::shared_ptr<storage_buffer>;
using storage_buffer_weak_ptr = std::weak_ptr<storage_buffer>;

template<typename... Args>
storage_buffer_ptr make_storage_buffer(Args&&... args)
{
    return std::make_shared<storage_buffer>(std::forward<Args>(args)...);
}

}

#endif
