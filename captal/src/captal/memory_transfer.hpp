#ifndef CAPTAL_MEMORY_TRANSFER_SCHEDULER_HPP_INCLUDED
#define CAPTAL_MEMORY_TRANSFER_SCHEDULER_HPP_INCLUDED

#include "config.hpp"

#include <unordered_map>
#include <future>

#include <tephra/renderer.hpp>
#include <tephra/commands.hpp>
#include <tephra/synchronization.hpp>

#include "asynchronous_resource.hpp"
#include "signal.hpp"

namespace cpt
{

using transfer_ended_signal = cpt::signal_st<>;

struct memory_transfer_info
{
    tph::command_buffer& buffer;
    transfer_ended_signal& signal;
    asynchronous_resource_keeper& keeper;
};

class memory_transfer_scheduler
{
    static constexpr std::size_t no_parent{std::numeric_limits<std::size_t>::max()};

public:
    explicit memory_transfer_scheduler(tph::renderer& renderer) noexcept;
    ~memory_transfer_scheduler();
    memory_transfer_scheduler(const memory_transfer_scheduler&) = delete;
    memory_transfer_scheduler& operator=(const memory_transfer_scheduler&) = delete;
    memory_transfer_scheduler(memory_transfer_scheduler&& other) noexcept = delete;
    memory_transfer_scheduler& operator=(memory_transfer_scheduler&& other) noexcept = delete;

    memory_transfer_info begin_transfer();
    memory_transfer_info begin_transfer(std::thread::id thread);
    void submit_transfers();

private:
    struct thread_transfer_buffer
    {
        tph::command_buffer buffer{};
        transfer_ended_signal signal{};
        asynchronous_resource_keeper keeper{};
        std::size_t parent{no_parent};
        bool begin{};
        bool submitted{};
    };

    struct thread_transfer_pool
    {
        tph::command_pool pool{};
        std::promise<void> exit_promise{};
        std::future<void> exit_future{};
        std::vector<thread_transfer_buffer> buffers{};
    };

    struct transfer_buffer
    {
        tph::command_buffer buffer{};
        tph::fence fence{};
        bool begin{};
        bool submitted{};
    };

private:
    thread_transfer_pool& get_pool(std::thread::id thread);

    thread_transfer_buffer& next_thread_buffer(thread_transfer_pool& pool, std::thread::id thread);
    thread_transfer_buffer& add_thread_buffer(thread_transfer_pool& pool, std::thread::id thread);
    bool check_thread_buffer(thread_transfer_buffer& data);
    void reset_thread_buffer(thread_transfer_buffer& data, std::thread::id thread);

private:
    tph::renderer* m_renderer{};
    std::unordered_map<std::thread::id, thread_transfer_pool> m_thread_pools{};
    tph::command_pool m_pool{};
    std::vector<transfer_buffer> m_buffers{};
    std::mutex m_mutex{};
    bool m_begin{};
};

}

#endif