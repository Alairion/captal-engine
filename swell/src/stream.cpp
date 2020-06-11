#include "stream.hpp"

#include <stdexcept>
#include <iostream>

#include <portaudio.h>

#include "mixer.hpp"

namespace swl
{

namespace impl
{

int callback(const void*, void* output, unsigned long frame_count, const PaStreamCallbackTimeInfo*, unsigned long, void* user_data)
{
    auto& mixer{*static_cast<swl::mixer*>(user_data)};

    try
    {
        mixer.next(static_cast<float*>(output), static_cast<std::size_t>(frame_count));
    }
    catch(const std::exception& e)
    {
        std::cerr << "Audio stream aborted. " << e.what() << std::endl;

        return paAbort;
    }

    return mixer.is_ok() ? paContinue : paAbort;
}

}

stream::stream(application& application [[maybe_unused]], const physical_device& physical_device, mixer& mixer)
{
    PaStreamParameters output_info{};
    output_info.device = physical_device.id();
    output_info.channelCount = mixer.channel_count();
    output_info.sampleFormat = paFloat32;
    output_info.suggestedLatency = physical_device.default_low_output_latency().count();

    if(auto err{Pa_OpenStream(&m_stream, nullptr, &output_info, static_cast<double>(mixer.sample_rate()), 0, 0, &impl::callback, &mixer)}; err != paNoError)
        throw std::runtime_error{"Can not open audio stream (" + std::string{Pa_GetErrorText(err)} + "; Host: " + std::to_string(Pa_GetLastHostErrorInfo()->hostApiType) + "; Error: " + std::to_string(Pa_GetLastHostErrorInfo()->errorCode) + "; Decription: " +  Pa_GetLastHostErrorInfo()->errorText + ")"};
}

stream::~stream()
{
    if(m_stream)
        Pa_CloseStream(m_stream);
}

stream::stream(stream&& other) noexcept
:m_stream{std::exchange(other.m_stream, nullptr)}
{

}

stream& stream::operator=(stream&& other) noexcept
{
    m_stream = std::exchange(other.m_stream, m_stream);

    return *this;
}

void stream::start()
{
    if(Pa_IsStreamActive(m_stream))
    {
        stop();
    }

    if(auto err = Pa_StartStream(m_stream); err != paNoError)
        throw std::runtime_error{"Can not start playing audio stream. (" + std::string{Pa_GetErrorText(err)} + ")"};
}

void stream::stop()
{
    if(auto err = Pa_StopStream(m_stream); err != paNoError)
        throw std::runtime_error{"Can not stop audio stream. (" + std::string{Pa_GetErrorText(err)} + ")"};
}

bool stream::active()
{
    return Pa_IsStreamActive(m_stream) == 1;
}

}
