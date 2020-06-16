#ifndef SWELL_STREAM_HPP_INCLUDED
#define SWELL_STREAM_HPP_INCLUDED

#include "config.hpp"

#include <mutex>

#include "physical_device.hpp"

namespace swl
{

class application;
class mixer;

class stream
{
    static std::mutex mutex;

public:
    stream() = default;
    stream(application& application, const physical_device& physical_device, mixer& mixer);
    ~stream();
    stream(const stream&) = delete;
    stream& operator=(const stream&) = delete;
    stream(stream&& other) noexcept;
    stream& operator=(stream&& other) noexcept;

    void start();
    void stop();
    bool active();

private:
    void* m_stream{};
    mixer* m_mixer{};
};

}

#endif
