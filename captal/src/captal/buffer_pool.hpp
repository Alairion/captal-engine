#ifndef CAPTAL_BUFFER_POOL_HPP_INCLUDED
#define CAPTAL_BUFFER_POOL_HPP_INCLUDED

#include "config.hpp"

#include <tephra/buffer.hpp>

#include "signal.hpp"
#include "memory_transfer.hpp"

namespace cpt
{

class buffer_heap;

class CAPTAL_API buffer_heap_chunk
{
    friend class buffer_heap;

private:
    explicit buffer_heap_chunk(buffer_heap* parent, std::uint64_t offset, std::uint64_t size) noexcept;

public:
    constexpr buffer_heap_chunk() = default;
    ~buffer_heap_chunk();
    buffer_heap_chunk(const buffer_heap_chunk&) = delete;
    buffer_heap_chunk& operator=(const buffer_heap_chunk&) = delete;
    buffer_heap_chunk(buffer_heap_chunk&& other) noexcept;
    buffer_heap_chunk& operator=(buffer_heap_chunk&& other) noexcept;

    void upload(std::uint64_t offset = 0, std::uint64_t size = std::numeric_limits<std::uint64_t>::max());

    void* map(std::uint64_t offset = 0) noexcept;
    const void* map(std::uint64_t offset = 0) const noexcept;

    buffer_heap& heap() noexcept
    {
        return *m_parent;
    }

    const buffer_heap& heap() const noexcept
    {
        return *m_parent;
    }

    std::uint64_t offset() const noexcept
    {
        return m_offset;
    }

    std::uint64_t size() const noexcept
    {
        return m_size;
    }

private:
    buffer_heap* m_parent{};
    std::uint64_t m_offset{};
    std::uint64_t m_size{};
};

class CAPTAL_API buffer_heap
{
    friend class buffer_heap_chunk;
    friend class buffer_pool;

private:
    struct range
    {
        std::uint64_t offset{};
        std::uint64_t size{};
    };

public:
    explicit buffer_heap(std::uint64_t size, tph::buffer_usage usage);
    ~buffer_heap();
    buffer_heap(const buffer_heap&) = delete;
    buffer_heap& operator=(const buffer_heap&) = delete;
    buffer_heap(buffer_heap&& other) noexcept = delete;
    buffer_heap& operator=(buffer_heap&& other) noexcept = delete;

    std::optional<buffer_heap_chunk> try_allocate(std::uint64_t size, std::uint64_t alignment);
    buffer_heap_chunk allocate_first(std::uint64_t size);

    tph::buffer& buffer() noexcept
    {
        return m_device_data;
    }

    const tph::buffer& buffer() const noexcept
    {
        return m_device_data;
    }

    std::uint64_t size() const noexcept
    {
        return m_size;
    }

    void* map() noexcept
    {
        return m_local_map;
    }

    const void* map() const noexcept
    {
        return m_local_map;
    }

    std::uint64_t free_space() const noexcept
    {
        return m_free_space;
    }

    std::size_t allocation_count() const noexcept
    {
        return m_allocation_count;
    }

#ifdef CAPTAL_DEBUG
    void set_name(std::string_view name);
#else
    void set_name(std::string_view name [[maybe_unused]]) const noexcept
    {

    }
#endif

private:
    bool begin_upload(tph::command_buffer& command_buffer);
    void end_upload(tph::command_buffer& command_buffer, transfer_ended_signal& signal);

    void register_upload(std::uint64_t offset, std::uint64_t size) noexcept;
    void unregister_chunk(const buffer_heap_chunk& chunk) noexcept;

private:
    struct staging_buffer
    {
        tph::buffer buffer{};
        std::uint32_t used{}; //only 4 bits are used
        std::array<cpt::scoped_connection, 4> connection{};
    };

private:
    tph::buffer m_local_data{};
    tph::buffer m_device_data{};
    std::vector<staging_buffer> m_stagings{};
    std::uint64_t m_size{};
    void* m_local_map{};
    std::atomic<std::uint64_t> m_free_space{};
    std::atomic<std::size_t> m_allocation_count{};
    std::vector<range> m_ranges{};
    std::mutex m_mutex{};

    std::vector<tph::buffer_copy> m_upload_ranges{};
    std::size_t m_current_staging{};
    std::uint32_t m_current_mask{};
    std::uint32_t m_current_mask_index{};
    std::mutex m_upload_mutex{};

#ifdef CAPTAL_DEBUG
    std::string m_name{};
#endif
};

class CAPTAL_API buffer_pool
{
public:
    explicit buffer_pool(tph::buffer_usage pool_usage, std::uint64_t pool_size = 1024 * 1024);
    ~buffer_pool() = default;
    buffer_pool(const buffer_pool&) = delete;
    buffer_pool& operator=(const buffer_pool&) = delete;
    buffer_pool(buffer_pool&&) noexcept = delete;
    buffer_pool& operator=(buffer_pool&&) noexcept = delete;

    buffer_heap_chunk allocate(std::uint64_t size, std::uint64_t alignment);
    void upload(memory_transfer_info info);
    void upload();
    void clean();

#ifdef CAPTAL_DEBUG
    void set_name(std::string_view name);
#else
    void set_name(std::string_view name [[maybe_unused]]) const noexcept
    {

    }
#endif

private:
    tph::buffer_usage m_pool_usage{};
    std::uint64_t m_pool_size{};

    std::vector<bool> m_to_end{};
    std::vector<std::unique_ptr<buffer_heap>> m_heaps{};
    mutable std::mutex m_mutex{};

#ifdef CAPTAL_DEBUG
    std::string m_name{};
#endif
};

}

#endif
