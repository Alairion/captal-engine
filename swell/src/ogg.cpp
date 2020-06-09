#include "ogg.hpp"

#include <cassert>

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>

namespace swl
{

void ogg_reader::vorbis_deleter::operator()(OggVorbis_File* file)
{
    ov_clear(file);
    delete file;
}

static std::size_t stream_read(void* ptr, std::size_t size, std::size_t nmemb, void* datasource)
{
    std::istream& stream = *static_cast<std::istream*>(datasource);

    const std::size_t total_size{size * nmemb};
    stream.read(reinterpret_cast<char*>(ptr), total_size);

    return static_cast<std::size_t>(stream.gcount());
}

static int stream_seek(void* datasource, ogg_int64_t offset, int whence)
{
    std::istream& stream = *static_cast<std::istream*>(datasource);

    stream.clear();

    if(whence == SEEK_SET)
    {
        stream.seekg(offset, std::ios_base::beg);
    }
    else if(whence == SEEK_CUR)
    {
        stream.seekg(offset, std::ios_base::cur);
    }
    else if(whence == SEEK_END)
    {
        stream.seekg(offset, std::ios_base::end);
    }

    return static_cast<int>(stream.tellg());
}

static long stream_tell(void* datasource)
{
    std::istream& stream = *static_cast<std::istream*>(datasource);

    return static_cast<long>(stream.tellg());
}

static std::size_t memory_read(void* ptr, std::size_t size, std::size_t nmemb, void* datasource)
{
    memory_stream& stream = *static_cast<memory_stream*>(datasource);

    const std::size_t total_size{size * nmemb};
    const std::size_t to_read{std::min(std::size(stream.data) - static_cast<std::size_t>(stream.pos), total_size)};

    std::copy(std::cbegin(stream.data) + stream.pos, std::cbegin(stream.data) + stream.pos + to_read, reinterpret_cast<char*>(ptr));
    stream.pos += to_read;

    return to_read;
}

static int memory_seek(void* datasource, ogg_int64_t offset, int whence)
{
    memory_stream& stream = *static_cast<memory_stream*>(datasource);

    if(whence == SEEK_SET)
    {
        stream.pos = offset;
    }
    else if(whence == SEEK_CUR)
    {
        stream.pos += offset;
    }
    else if(whence == SEEK_END)
    {
        stream.pos = static_cast<std::streampos>(std::size(stream.data)) + offset;
    }

    return static_cast<int>(stream.pos);
}

static long memory_tell(void* datasource)
{
    memory_stream& stream = *static_cast<memory_stream*>(datasource);
    return static_cast<long>(stream.pos);
}

static constexpr ov_callbacks stream_callbacks{stream_read, stream_seek, nullptr, stream_tell};
static constexpr ov_callbacks memory_callbacks{memory_read, memory_seek, nullptr, memory_tell};

ogg_reader::ogg_reader(const std::filesystem::path& file, sound_reader_options options)
:m_options{options}
,m_vorbis{new OggVorbis_File{}, vorbis_deleter{}}
,m_file{file, std::ios_base::binary}
{
    if(!m_file)
        throw std::runtime_error{"Can not open file \"" + file.string() + "\"."};

    if(const auto error{ov_open_callbacks(&m_file, m_vorbis.get(), nullptr, 0, stream_callbacks)}; error < 0)
        throw std::runtime_error{"Can not open the ogg file. #" + std::to_string(error)};

    m_frame_count = static_cast<std::uint64_t>(ov_pcm_total(m_vorbis.get(), -1));

    const vorbis_info* info{ov_info(m_vorbis.get(), 0)};
    m_channel_count = static_cast<std::uint32_t>(info->channels);
    m_frequency = static_cast<std::uint32_t>(info->rate);

    if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        fill_buffer();
        close();
        m_file.close();
    }
}

ogg_reader::ogg_reader(const std::string_view& data, sound_reader_options options)
:m_options{options}
,m_vorbis{new OggVorbis_File{}, vorbis_deleter{}}
,m_source{data}
{
    if(auto error = ov_open_callbacks(&m_source, m_vorbis.get(), nullptr, 0, memory_callbacks); error < 0)
        throw std::runtime_error{"Can not open the audio file. #" + std::to_string(error)};

    m_frame_count = static_cast<std::uint64_t>(ov_pcm_total(m_vorbis.get(), -1));

    const vorbis_info* info{ov_info(m_vorbis.get(), 0)};
    m_channel_count = static_cast<std::uint32_t>(info->channels);
    m_frequency = static_cast<std::uint32_t>(info->rate);

    if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        fill_buffer();
        close();
    }
}

ogg_reader::ogg_reader(std::istream& stream, sound_reader_options options)
:m_options{options}
,m_vorbis{new OggVorbis_File{}, vorbis_deleter{}}
,m_stream{&stream}
{
    assert(stream && "Invalid stream.");

    if(const auto error{ov_open_callbacks(m_stream, m_vorbis.get(), nullptr, 0, stream_callbacks)}; error < 0)
        throw std::runtime_error{"Can not open the ogg file. #" + std::to_string(error)};

    m_frame_count = static_cast<std::uint64_t>(ov_pcm_total(m_vorbis.get(), -1));

    const vorbis_info* info{ov_info(m_vorbis.get(), 0)};
    m_channel_count = static_cast<std::uint32_t>(info->channels);
    m_frequency = static_cast<std::uint32_t>(info->rate);

    if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        fill_buffer();
        close();
    }
}

ogg_reader::~ogg_reader()
{
    if(m_vorbis)
        ov_clear(m_vorbis.get());
}

bool ogg_reader::read_samples(float* output, std::size_t frame_count)
{
    if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        return read_samples_from_buffer(output, frame_count);
    }
    else if(m_vorbis)
    {
        return read_samples_from_vorbis(output, frame_count);
    }

    return false;
}

void ogg_reader::seek_samples(std::uint64_t frame_offset)
{
    if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        m_current_frame = static_cast<std::uint32_t>(frame_offset);
    }
    else
    {
        if(const auto error{ov_pcm_seek(m_vorbis.get(), frame_offset)}; error < 0)
            throw std::runtime_error{"Can not seek pos in the audio file. #" + std::to_string(error)};
    }
}

std::uint64_t ogg_reader::get_frame_count()
{
    return m_frame_count;
}

std::uint32_t ogg_reader::get_frequency()
{
    return m_frequency;
}

std::uint32_t ogg_reader::get_channels()
{
    return static_cast<std::uint32_t>(m_channel_count);
}

void ogg_reader::fill_buffer()
{
    m_buffer.reserve(m_frame_count * m_channel_count);

    std::uint64_t total_read{};

    while(total_read < m_frame_count)
    {
        float** data{};
        const auto read{ov_read_float(m_vorbis.get(), &data, static_cast<int>(m_frame_count), &m_current_section)};

        for(std::size_t i{}; i < static_cast<std::size_t>(read); ++i)
        {
            for(std::size_t j{}; j < m_channel_count; ++j)
            {
                m_buffer.push_back(data[j][i]);
            }
        }

        total_read += read;

        if(read > 0)
        {
            total_read += read;
        }
        else if(read == 0)
        {
            return;
        }
        else if(read < 0)
        {
            throw std::runtime_error{"Can not read the audio file. #" + std::to_string(read)};
        }
    }
}

void ogg_reader::close()
{
    ov_clear(m_vorbis.get());
    m_vorbis.reset();
}

std::size_t ogg_reader::sample_size(std::size_t frame_count)
{
    return frame_count * m_channel_count;
}

bool ogg_reader::read_samples_from_buffer(float* output, std::size_t frame_count)
{
    if(sample_size(m_current_frame + frame_count) > std::size(m_buffer))
    {
        const auto* begin{std::data(m_buffer) + sample_size(m_current_frame)};
        const auto* end{std::data(m_buffer) + std::size(m_buffer)};

        std::copy(begin, end, output);
        m_current_frame += frame_count;

        return false;
    }
    else
    {
        const auto* begin{std::data(m_buffer) + sample_size(m_current_frame)};

        std::copy(begin, begin + sample_size(frame_count), output);
        m_current_frame += frame_count;

        return true;
    }
}

bool ogg_reader::read_samples_from_vorbis(float* output, std::size_t frame_count)
{
    std::uint64_t total_read{};

    while(total_read < frame_count)
    {
        float** data{};
        const auto read{ov_read_float(m_vorbis.get(), &data, static_cast<int>(frame_count - total_read), &m_current_section)};

        for(std::size_t i{}; i < static_cast<std::size_t>(read); ++i)
        {
            for(std::size_t j{}; j < m_channel_count; ++j)
            {
                output[(i * m_channel_count) + j] = data[j][i];
            }
        }

        if(read > 0)
        {
            total_read += read;
            output += read * m_channel_count;
        }
        else if(read == 0)
        {
            return false;
        }
        else if(read < 0)
        {
            throw std::runtime_error{"Can not read the audio file. #" + std::to_string(read)};
        }
    }

    return true;
}

}
