#include "stream.hpp"

#include <stdexcept>
#include <iostream>

#include <portaudio.h>

#include "mixer.hpp"

namespace swl
{

static stream_callback_flags make_flags(PaStreamCallbackFlags flags)
{
    switch(flags)
    {
        case paOutputUnderflow: return stream_callback_flags::underflow;
        case paOutputOverflow:  return stream_callback_flags::overflow;
        case paInputUnderflow:  return stream_callback_flags::underflow;
        case paInputOverflow:   return stream_callback_flags::overflow;
        default: return stream_callback_flags::none;
    }
}

static int internal_output_callback(const void*, void* output, unsigned long frame_count, const PaStreamCallbackTimeInfo* time, PaStreamCallbackFlags flags, void* user_data)
{
    try
    {
        stream::output_callback& callback{*static_cast<stream::output_callback*>(user_data)};

        return static_cast<int>(callback(static_cast<std::size_t>(frame_count), output, seconds{time->currentTime}, make_flags(flags)));
    }
    catch(const std::exception& e)
    {
        std::cerr << "Audio stream aborted. " << e.what() << std::endl;
        return paAbort;
    }
}

static int internal_input_callback(const void* input, void*, unsigned long frame_count, const PaStreamCallbackTimeInfo* time, PaStreamCallbackFlags flags, void* user_data)
{
    try
    {
        stream::input_callback& callback{*static_cast<stream::input_callback*>(user_data)};

        return static_cast<int>(callback(static_cast<std::size_t>(frame_count), input, seconds{time->currentTime}, make_flags(flags)));
    }
    catch(const std::exception& e)
    {
        std::cerr << "Audio stream aborted. " << e.what() << std::endl;
        return paAbort;
    }
}

stream::stream(application& application [[maybe_unused]], const physical_device& device, const stream_info& info, output_callback callback)
:m_info{info}
,m_output_callback{std::make_unique<output_callback>(std::move(callback))}
{
    PaStreamParameters output_info{};
    output_info.device           = device.id();
    output_info.sampleFormat     = static_cast<PaSampleFormat>(info.format);
    output_info.channelCount     = info.channel_count;
    output_info.suggestedLatency = info.latency.count();

    std::lock_guard lock{mutex}; //Pa_OpenStream (and Pa_CloseStream) are not guaranteed to be thread safe
    if(auto err{Pa_OpenStream(&m_stream, nullptr, &output_info, static_cast<double>(info.sample_rate), 0, 0, &internal_output_callback, m_output_callback.get())}; err != paNoError)
        throw std::runtime_error{"Can not open audio stream (" + std::string{Pa_GetErrorText(err)} + "; Host: " + std::to_string(Pa_GetLastHostErrorInfo()->hostApiType) + "; Error: " + std::to_string(Pa_GetLastHostErrorInfo()->errorCode) + "; Decription: " +  Pa_GetLastHostErrorInfo()->errorText + ")"};

    const PaStreamInfo* new_info{Pa_GetStreamInfo(m_stream)};
    m_info.latency     = seconds{new_info->outputLatency};
    m_info.sample_rate = static_cast<std::uint32_t>(new_info->sampleRate);
}

stream::stream(application& application [[maybe_unused]], const physical_device& device, const stream_info& info, input_callback callback)
:m_info{info}
,m_input_callback{std::make_unique<input_callback>(std::move(callback))}
{
    PaStreamParameters input_info{};
    input_info.device           = device.id();
    input_info.sampleFormat     = static_cast<PaSampleFormat>(info.format);
    input_info.channelCount     = info.channel_count;
    input_info.suggestedLatency = info.latency.count();

    std::lock_guard lock{mutex}; //Pa_OpenStream (and Pa_CloseStream) are not guaranteed to be thread safe
    if(auto err{Pa_OpenStream(&m_stream, &input_info, nullptr, static_cast<double>(info.sample_rate), 0, 0, &internal_input_callback, m_output_callback.get())}; err != paNoError)
        throw std::runtime_error{"Can not open audio stream (" + std::string{Pa_GetErrorText(err)} + "; Host: " + std::to_string(Pa_GetLastHostErrorInfo()->hostApiType) + "; Error: " + std::to_string(Pa_GetLastHostErrorInfo()->errorCode) + "; Decription: " +  Pa_GetLastHostErrorInfo()->errorText + ")"};

    const PaStreamInfo* new_info{Pa_GetStreamInfo(m_stream)};
    m_info.latency     = seconds{new_info->outputLatency};
    m_info.sample_rate = static_cast<std::uint32_t>(new_info->sampleRate);
}

stream::~stream()
{
    if(m_stream)
    {
        std::lock_guard lock{mutex};
        Pa_CloseStream(m_stream);
    }
}

stream::stream(stream&& other) noexcept
:m_stream{std::exchange(other.m_stream, nullptr)}
,m_info{other.m_info}
,m_output_callback{std::move(other.m_output_callback)}
{

}

stream& stream::operator=(stream&& other) noexcept
{
    m_stream = std::exchange(other.m_stream, m_stream);
    m_info = other.m_info;
    m_output_callback = std::move(other.m_output_callback);

    return *this;
}

void stream::start()
{
    if(!active())
    {
        if(auto err = Pa_StartStream(m_stream); err != paNoError)
            throw std::runtime_error{"Can not start playing audio stream. (" + std::string{Pa_GetErrorText(err)} + ")"};
    }
}

void stream::stop()
{
    if(auto err = Pa_StopStream(m_stream); err != paNoError)
        throw std::runtime_error{"Can not stop audio stream. (" + std::string{Pa_GetErrorText(err)} + ")"};
}

bool stream::active() const noexcept
{
    return Pa_IsStreamActive(m_stream) == 1;
}

seconds stream::time() const noexcept
{
    return seconds{Pa_GetStreamTime(m_stream)};
}

bool is_input_format_supported(application& application [[maybe_unused]], physical_device& device, const stream_info& info)
{
    PaStreamParameters input_info{};
    input_info.device           = device.id();
    input_info.sampleFormat     = static_cast<PaSampleFormat>(info.format);
    input_info.channelCount     = info.channel_count;
    input_info.suggestedLatency = info.latency.count();

    return Pa_IsFormatSupported(&input_info, nullptr, static_cast<double>(info.sample_rate)) == paFormatIsSupported;
}

bool is_output_format_supported(application& application [[maybe_unused]], physical_device& device, const stream_info& info)
{
    PaStreamParameters output_info{};
    output_info.device           = device.id();
    output_info.sampleFormat     = static_cast<PaSampleFormat>(info.format);
    output_info.channelCount     = info.channel_count;
    output_info.suggestedLatency = info.latency.count();

    return Pa_IsFormatSupported(nullptr, &output_info, static_cast<double>(info.sample_rate)) == paFormatIsSupported;
}

}
