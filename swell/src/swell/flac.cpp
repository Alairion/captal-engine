#include "flac.hpp"

#include <cassert>

#include <FLAC/stream_decoder.h>

namespace swl
{

static FLAC__StreamDecoder* decoder_cast(void* decoder)
{
    return static_cast<FLAC__StreamDecoder*>(decoder);
}

void flac_reader::flac_deleter::operator()(void* decoder)
{
    FLAC__stream_decoder_delete(decoder_cast(decoder));
}

static float get_factor(std::uint32_t bits_per_sample) noexcept
{
    return 1.0f / static_cast<float>(1u << (bits_per_sample - 1u));
}

static FLAC__StreamDecoderReadStatus stream_read(const FLAC__StreamDecoder* decoder [[maybe_unused]], FLAC__byte* buffer, std::size_t* bytes, void* userdata)
{
    auto& stream{*static_cast<impl::flac_context*>(userdata)->stream};

    if(*bytes > 0)
    {
        *bytes = static_cast<std::size_t>(stream.read(reinterpret_cast<char*>(buffer), static_cast<std::streamsize>(*bytes)).gcount());
        if(*bytes == 0)
        {
            return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        }

        if(stream.fail() && !stream.eof())
        {
            return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
        }

        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }

    return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
}

static FLAC__StreamDecoderSeekStatus stream_seek(const FLAC__StreamDecoder* decoder [[maybe_unused]], FLAC__uint64 position, void* userdata)
{
    auto& stream{*static_cast<impl::flac_context*>(userdata)->stream};

    stream.clear();
    if(!stream.seekg(position, std::ios_base::beg))
    {
        return FLAC__STREAM_DECODER_SEEK_STATUS_UNSUPPORTED;
    }

    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

static FLAC__StreamDecoderTellStatus stream_tell(const FLAC__StreamDecoder* decoder [[maybe_unused]], FLAC__uint64* output_position, void* userdata)
{
    auto& stream{*static_cast<impl::flac_context*>(userdata)->stream};

    if(const auto position{stream.tellg()}; position != -1)
    {
        *output_position = static_cast<std::uint64_t>(position);

        return FLAC__STREAM_DECODER_TELL_STATUS_OK;
    }

    if(!stream)
    {
        return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
    }

    return FLAC__STREAM_DECODER_TELL_STATUS_UNSUPPORTED;
}

static FLAC__StreamDecoderLengthStatus stream_length(const FLAC__StreamDecoder* decoder [[maybe_unused]], FLAC__uint64* output_size, void* userdata)
{
    auto& stream{*static_cast<impl::flac_context*>(userdata)->stream};

    const auto current_position{stream.tellg()};

    if(current_position == -1)
    {
        return FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED;
    }

    *output_size = static_cast<std::uint64_t>(stream.seekg(0, std::ios_base::end).tellg());

    if(!stream.seekg(current_position, std::ios_base::beg))
    {
        return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
    }

    return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

static FLAC__bool stream_eof(const FLAC__StreamDecoder* decoder [[maybe_unused]], void* userdata)
{
    auto& stream{*static_cast<impl::flac_context*>(userdata)->stream};

    return static_cast<FLAC__bool>(stream.eof());
}

static FLAC__StreamDecoderReadStatus memory_read(const FLAC__StreamDecoder* decoder [[maybe_unused]], FLAC__byte* buffer, std::size_t* bytes, void* userdata)
{
    auto& source{static_cast<impl::flac_context*>(userdata)->source};

    if(*bytes > 0)
    {
        *bytes = std::min(std::size(source.data) - source.position, *bytes);

        if(*bytes == 0)
        {
            return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        }

        std::copy_n(std::data(source.data) + source.position, *bytes, buffer);
        source.position += *bytes;

        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }

    return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
}

static FLAC__StreamDecoderSeekStatus memory_seek(const FLAC__StreamDecoder* decoder [[maybe_unused]], FLAC__uint64 position, void* userdata)
{
    auto& source{static_cast<impl::flac_context*>(userdata)->source};

    if(position > std::size(source.data))
    {
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    }

    source.position = position;

    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

static FLAC__StreamDecoderTellStatus memory_tell(const FLAC__StreamDecoder* decoder [[maybe_unused]], FLAC__uint64* output_position, void* userdata)
{
    auto& source{static_cast<impl::flac_context*>(userdata)->source};

    *output_position = source.position;

    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

static FLAC__StreamDecoderLengthStatus memory_length(const FLAC__StreamDecoder* decoder [[maybe_unused]], FLAC__uint64* output_size, void* userdata)
{
    auto& source{static_cast<impl::flac_context*>(userdata)->source};

    *output_size = std::size(source.data);

    return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

static FLAC__bool memory_eof(const FLAC__StreamDecoder* decoder [[maybe_unused]], void* userdata)
{
    auto& source{static_cast<impl::flac_context*>(userdata)->source};

    return source.position == std::size(source.data);
}

static FLAC__StreamDecoderWriteStatus common_write(const FLAC__StreamDecoder* decoder [[maybe_unused]], const FLAC__Frame* frame, const FLAC__int32* const* buffer, void* userdata)
{
    auto& context{*static_cast<impl::flac_context*>(userdata)};

    context.buffer.clear();
    context.buffer.resize(frame->header.blocksize * frame->header.channels);
    context.buffer_index = 0;

    const float factor{get_factor(frame->header.bits_per_sample)};
    std::size_t index{};

    for(std::uint32_t i{}; i < frame->header.blocksize; ++i)
    {
        for(std::uint32_t j{}; j < frame->header.channels; ++j)
        {
            context.buffer[index++] = static_cast<float>(buffer[j][i]) * factor;
        }
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void common_metadata(const FLAC__StreamDecoder* decoder [[maybe_unused]], const FLAC__StreamMetadata* metadata, void* userdata)
{
    auto& output{static_cast<impl::flac_context*>(userdata)->info};

    if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
    {
        output.channel_count = metadata->data.stream_info.channels;
        output.frame_count   = metadata->data.stream_info.total_samples;
        output.frequency     = metadata->data.stream_info.sample_rate;
        output.seekable      = true;
    }
}

static void common_error(const FLAC__StreamDecoder* decoder [[maybe_unused]], FLAC__StreamDecoderErrorStatus status, void* userdata)
{
    auto& context{*static_cast<impl::flac_context*>(userdata)};

    context.error = static_cast<std::int32_t>(status);
}

flac_reader::flac_reader(const std::filesystem::path& file, sound_reader_options options)
:m_options{options}
,m_decoder{FLAC__stream_decoder_new()}
,m_context{std::make_unique<impl::flac_context>()}
{
    std::ifstream ifs{file, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"swl::flac_reader can not open file \"" + file.string() + "\"."};

    if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        m_context->source_buffer.resize(std::filesystem::file_size(file));
        if(!ifs.read(reinterpret_cast<char*>(std::data(m_context->source_buffer)), static_cast<std::streamsize>(std::size(m_context->source_buffer))))
            throw std::runtime_error{"swl::flac_reader can not read file \"" + file.string() + "\"."};

        m_context->source.data = m_context->source_buffer;
        init_from_memory();
    }
    else if(static_cast<bool>(m_options & sound_reader_options::decoded))
    {
        m_context->stream = &ifs;
        init_from_stream();
    }
    else
    {
        m_context->file = std::move(ifs);
        m_context->stream = &m_context->file;
        init_from_stream();
    }
}

flac_reader::flac_reader(std::span<const uint8_t> data, sound_reader_options options)
:m_options{options}
,m_decoder{FLAC__stream_decoder_new()}
,m_context{std::make_unique<impl::flac_context>()}
{
    m_context->source.data = data;

    init_from_memory();
}

flac_reader::flac_reader(std::istream& stream, sound_reader_options options)
:m_options{options}
,m_decoder{FLAC__stream_decoder_new()}
,m_context{std::make_unique<impl::flac_context>()}
{
    assert(stream && "Invalid stream.");

    if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        const auto size{stream.seekg(0, std::ios_base::end).tellg()};
        m_context->source_buffer.resize(static_cast<std::size_t>(size));

        stream.seekg(0, std::ios_base::beg);
        if(!stream.read(reinterpret_cast<char*>(std::data(m_context->source_buffer)), static_cast<std::streamsize>(size)))
            throw std::runtime_error{"swl::flac_reader can not read file."};

        m_context->source.data = m_context->source_buffer;
        init_from_memory();
    }
    else
    {
        m_context->stream = &stream;
        init_from_stream();
    }
}

bool flac_reader::read(float* output, std::size_t frame_count)
{
    if(static_cast<bool>(m_options & sound_reader_options::decoded))
    {
        return read_samples_from_buffer(output, frame_count);
    }
    else if(m_decoder)
    {
        return read_samples_from_flac(output, frame_count);
    }

    return false;
}

void flac_reader::seek(std::uint64_t frame)
{
    if(static_cast<bool>(m_options & sound_reader_options::decoded))
    {
        m_current_frame = frame;
    }
    else
    {
        if(!FLAC__stream_decoder_seek_absolute(decoder_cast(m_decoder.get()), frame))
            throw std::runtime_error{"swl::flac_reader can not seek file."};

        m_current_frame = frame;
    }
}

std::uint64_t flac_reader::tell()
{
    return m_current_frame;
}

void flac_reader::init_from_memory()
{
    const auto error{FLAC__stream_decoder_init_stream(decoder_cast(m_decoder.get()), memory_read, memory_seek, memory_tell, memory_length, memory_eof, common_write, common_metadata, common_error, m_context.get())};
    if(error != FLAC__STREAM_DECODER_INIT_STATUS_OK)
        throw std::runtime_error{"swl::flac_reader can not init stream."};

    common_init();
}

void flac_reader::init_from_stream()
{
    const auto error{FLAC__stream_decoder_init_stream(decoder_cast(m_decoder.get()), stream_read, stream_seek, stream_tell, stream_length, stream_eof, common_write, common_metadata, common_error, m_context.get())};
    if(error != FLAC__STREAM_DECODER_INIT_STATUS_OK)
        throw std::runtime_error{"swl::flac_reader can not init stream."};

    common_init();
}

void flac_reader::common_init()
{
    if(!FLAC__stream_decoder_process_until_end_of_metadata(decoder_cast(m_decoder.get())))
        throw std::runtime_error{"swl::flac_reader can not decode metadata."};

    set_info(m_context->info);

    if(static_cast<bool>(m_options & sound_reader_options::decoded))
    {
        fill_buffer();
        close();
    }
}

void flac_reader::fill_buffer()
{
    m_decoded_buffer.reserve(info().frame_count * info().channel_count);

    FLAC__StreamDecoderState state{};
    while(state != FLAC__STREAM_DECODER_END_OF_STREAM)
    {
        FLAC__stream_decoder_process_single(decoder_cast(m_decoder.get()));

        m_decoded_buffer.insert(std::end(m_decoded_buffer), std::begin(m_context->buffer), std::end(m_context->buffer));

        state = FLAC__stream_decoder_get_state(decoder_cast(m_decoder.get()));
        if(state == FLAC__STREAM_DECODER_ABORTED || state == FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR)
        {
            throw std::runtime_error{"swl::flac_reader arboted."};
        }
    }
}

void flac_reader::close()
{
    m_decoder.reset();
    m_context.reset();
}

std::size_t flac_reader::sample_size(std::size_t frame_count)
{
    return frame_count * info().channel_count;
}

bool flac_reader::read_samples_from_buffer(float* output, std::size_t frame_count)
{
    if(sample_size(m_current_frame + frame_count) > std::size(m_decoded_buffer))
    {
        const auto* begin{std::data(m_decoded_buffer) + sample_size(m_current_frame)};
        const auto* end  {std::data(m_decoded_buffer) + std::size(m_decoded_buffer)};

        std::fill(std::copy(begin, end, output), output + sample_size(frame_count), 0.0f);
        m_current_frame += frame_count;

        return false;
    }
    else
    {
        const auto* begin{std::data(m_decoded_buffer) + sample_size(m_current_frame)};

        std::copy(begin, begin + sample_size(frame_count), output);
        m_current_frame += frame_count;

        return true;
    }
}

bool flac_reader::read_samples_from_flac(float* output, std::size_t frame_count)
{
    const std::size_t sample_count{sample_size(frame_count)};
    const std::size_t buffer_count{std::min(std::size(m_context->buffer) - m_context->buffer_index, sample_count)};

    std::copy(std::begin(m_context->buffer) + m_context->buffer_index, std::begin(m_context->buffer) + m_context->buffer_index + buffer_count, output);
    m_context->buffer_index += buffer_count;

    std::size_t total_read{buffer_count};

    while(total_read < sample_count)
    {
        FLAC__stream_decoder_process_single(decoder_cast(m_decoder.get()));
        const auto state{FLAC__stream_decoder_get_state(decoder_cast(m_decoder.get()))};

        const std::size_t count{std::min(std::size(m_context->buffer), sample_count - total_read)};
        std::copy(std::begin(m_context->buffer), std::begin(m_context->buffer) + count, output + total_read);

        m_context->buffer_index += count;
        total_read += count;

        if(state == FLAC__STREAM_DECODER_END_OF_STREAM)
        {
            std::fill(output + total_read, output + sample_count, 0.0f);
            break;
        }
        else if(state == FLAC__STREAM_DECODER_ABORTED || state == FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR)
        {
            throw std::runtime_error{"swl::flac_reader arboted."};
        }
    }

    m_current_frame += total_read / info().channel_count;

    return true;
}

}
