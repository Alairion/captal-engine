#include "stream.hpp"

#include <stdexcept>

#include <portaudio.h>

#include "mixer.hpp"

namespace swl
{

namespace impl
{

int callback(const void*, void* output, unsigned long, const PaStreamCallbackTimeInfo*, unsigned long, void* user_data)
{
    const auto data{static_cast<mixer*>(user_data)->next()};

    std::copy(std::begin(data), std::end(data), static_cast<float*>(output));

    return static_cast<mixer*>(user_data)->is_ok() ? paContinue : paAbort;
}

}

stream::stream(application& application [[maybe_unused]], const physical_device& physical_device, mixer& mixer)
{
    PaStreamParameters output_info{};
    output_info.device = physical_device.id();
    output_info.channelCount = mixer.channel_count();
    output_info.sampleFormat = paFloat32;
    output_info.suggestedLatency = static_cast<double>(mixer.frame_per_buffer()) / static_cast<double>(mixer.sample_rate());

    if(auto err{Pa_OpenStream(&m_stream, nullptr, &output_info, static_cast<double>(mixer.sample_rate()), static_cast<std::uint32_t>(mixer.frame_per_buffer()), 0, &impl::callback, &mixer)}; err != paNoError)
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
