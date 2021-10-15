//MIT License
//
//Copyright (c) 2021 Alexy Pellegrini
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

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

using transfer_ended_signal = cpt::signal<>;

struct memory_transfer_info
{
    tph::command_buffer& buffer;
    transfer_ended_signal& signal;
    asynchronous_resource_keeper& keeper;
};

class CAPTAL_API memory_transfer_scheduler
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
    std::size_t clean_threads();

private:
    struct thread_transfer_buffer
    {
        tph::command_buffer buffer{};
        transfer_ended_signal signal{};
        asynchronous_resource_keeper keeper{};
        std::size_t parent{no_parent};
        bool begin{};
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
    };

private:
    transfer_buffer& next_buffer();
    transfer_buffer& add_buffer();
    std::size_t buffer_index(const transfer_buffer& buffer) const noexcept;
    void reset_buffer(transfer_buffer& buffer);
    void reset_thread_buffer(thread_transfer_buffer& data);
    std::vector<std::reference_wrapper<tph::command_buffer>> secondary_buffers(std::size_t parent);

    thread_transfer_pool& get_transfer_pool(std::thread::id thread);
    thread_transfer_buffer& next_thread_buffer(thread_transfer_pool& pool, std::thread::id thread);
    thread_transfer_buffer& add_thread_buffer(thread_transfer_pool& pool, std::thread::id thread);

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
