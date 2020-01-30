#ifndef SWELL_STREAM_HPP_INCLUDED
#define SWELL_STREAM_HPP_INCLUDED

#include "config.hpp"

#include "physical_device.hpp"

struct PaStreamCallbackTimeInfo;

namespace swl
{

class application;
class mixer;

namespace impl
{

int callback(const void*, void* output, unsigned long, const PaStreamCallbackTimeInfo*, unsigned long, void* user_data);

}

class stream
{
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
    void* m_stream;
};

}

#endif
