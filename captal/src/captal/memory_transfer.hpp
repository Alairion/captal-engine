#ifndef CAPTAL_ENGINE_HPP_INCLUDED
#define CAPTAL_ENGINE_HPP_INCLUDED

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

using transfer_ended_signal = cpt::signal<>;

struct memory_transfer_info
{
    tph::command_buffer& buffer;
    transfer_ended_signal& signal;
    asynchronous_resource_keeper& keeper;
};

class memory_transfer_scheduler
{
public:
    explicit memory_transfer_scheduler(tph::renderer& renderer) noexcept;
    ~memory_transfer_scheduler();
    memory_transfer_scheduler(const memory_transfer_scheduler&) = delete;
    memory_transfer_scheduler& operator=(const memory_transfer_scheduler&) = delete;
    memory_transfer_scheduler(memory_transfer_scheduler&& other) noexcept = delete;
    memory_transfer_scheduler& operator=(memory_transfer_scheduler&& other) noexcept = delete;


    memory_transfer_info begin_transfer();


    void flush_transfers();

private:
    struct transfer_buffer
    {
        tph::command_buffer buffer{};
        tph::fence fence{};
        transfer_ended_signal signal{};
        asynchronous_resource_keeper keeper{};
        bool begin{};
    };

    struct transfer_pool
    {
        tph::command_pool pool{};
        std::promise<void> exit_promise{};
        std::future<void> exit_future{};
        std::vector<transfer_buffer> buffers{};
    };

private:
    transfer_buffer& next_buffer(transfer_pool& pool, std::thread::id thread);
    transfer_buffer& add_buffer(transfer_pool& pool);
    void reset(transfer_buffer& data);

private:
    tph::renderer* m_renderer{};
    std::unordered_map<std::thread::id, transfer_pool> m_pools{};
    std::mutex m_mutex{};
};

}

#endif
