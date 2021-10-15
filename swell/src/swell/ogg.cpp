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

#include "ogg.hpp"

#include <cassert>

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
    auto& stream{*static_cast<std::istream*>(datasource)};

    stream.read(reinterpret_cast<char*>(ptr), static_cast<std::streamsize>(size * nmemb));

    return static_cast<std::size_t>(stream.gcount());
}

static int stream_seek(void* datasource, ogg_int64_t offset, int whence)
{
    auto& stream{*static_cast<std::istream*>(datasource)};

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
    auto& stream{*static_cast<std::istream*>(datasource)};

    return static_cast<long>(stream.tellg());
}

static std::size_t memory_read(void* ptr, std::size_t size, std::size_t nmemb, void* datasource)
{
    auto& stream = *static_cast<impl::ogg_memory_stream*>(datasource);

    const std::size_t total_size{size * nmemb};
    const std::size_t to_read{std::min(std::size(stream.data) - stream.position, total_size)};

    std::copy(std::cbegin(stream.data) + stream.position, std::cbegin(stream.data) + stream.position + to_read, reinterpret_cast<std::uint8_t*>(ptr));
    stream.position += to_read;

    return to_read;
}

static int memory_seek(void* datasource, ogg_int64_t offset, int whence)
{
    auto& stream = *static_cast<impl::ogg_memory_stream*>(datasource);

    if(whence == SEEK_SET)
    {
        stream.position = offset;
    }
    else if(whence == SEEK_CUR)
    {
        stream.position += offset;
    }
    else if(whence == SEEK_END)
    {
        stream.position = std::size(stream.data) + offset;
    }

    return static_cast<int>(stream.position);
}

static long memory_tell(void* datasource)
{
    auto& stream = *static_cast<impl::ogg_memory_stream*>(datasource);
    return static_cast<long>(stream.position);
}

static constexpr ov_callbacks stream_callbacks{stream_read, stream_seek, nullptr, stream_tell};
static constexpr ov_callbacks memory_callbacks{memory_read, memory_seek, nullptr, memory_tell};

ogg_reader::ogg_reader(const std::filesystem::path& file, sound_reader_options options)
:m_options{options}
,m_vorbis{new OggVorbis_File{}}
{
    std::ifstream ifs{file, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"swl::ogg_reader can not open file \"" + file.string() + "\"."};

    if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        m_source_buffer.resize(std::filesystem::file_size(file));
        if(!ifs.read(reinterpret_cast<char*>(std::data(m_source_buffer)), static_cast<std::streamsize>(std::size(m_source_buffer))))
            throw std::runtime_error{"swl::ogg_reader can not read file \"" + file.string() + "\"."};

        m_source = std::make_unique<impl::ogg_memory_stream>(m_source_buffer);
        init_from_memory();
    }
    else if(static_cast<bool>(m_options & sound_reader_options::decoded))
    {
        m_stream = &ifs;
        init_from_stream();
    }
    else
    {
        m_file = std::make_unique<std::ifstream>(std::move(ifs));
        m_stream = m_file.get();
        init_from_stream();
    }
}

ogg_reader::ogg_reader(std::span<const uint8_t> data, sound_reader_options options)
:m_options{options}
,m_vorbis{new OggVorbis_File{}}
,m_source{std::make_unique<impl::ogg_memory_stream>(data)}
{
    init_from_memory();
}

ogg_reader::ogg_reader(std::istream& stream, sound_reader_options options)
:m_options{options}
,m_vorbis{new OggVorbis_File{}}
{
    assert(stream && "Invalid stream.");

    if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        const auto size{stream.seekg(0, std::ios_base::end).tellg()};
        m_source_buffer.resize(static_cast<std::size_t>(size));

        stream.seekg(0, std::ios_base::beg);
        if(!stream.read(reinterpret_cast<char*>(std::data(m_source_buffer)), static_cast<std::streamsize>(size)))
            throw std::runtime_error{"swl::ogg_reader can not read file."};

        m_source = std::make_unique<impl::ogg_memory_stream>(m_source_buffer);
        init_from_memory();
    }
    else
    {
        m_stream = &stream;
        init_from_stream();
    }
}

bool ogg_reader::read(float* output, std::size_t frame_count)
{
    if(static_cast<bool>(m_options & sound_reader_options::decoded))
    {
        return read_samples_from_buffer(output, frame_count);
    }
    else if(m_vorbis)
    {
        return read_samples_from_vorbis(output, frame_count);
    }

    return false;
}

void ogg_reader::seek(std::uint64_t frame)
{
    if(static_cast<bool>(m_options & sound_reader_options::decoded))
    {
        m_current_frame = frame;
    }
    else
    {
        if(const auto error{ov_pcm_seek(m_vorbis.get(), frame)}; error < 0)
            throw std::runtime_error{"swl::ogg_reader can not seek pos in the audio file. #" + std::to_string(error)};
    }
}

std::uint64_t ogg_reader::tell()
{
    if(static_cast<bool>(m_options & sound_reader_options::decoded))
    {
        return m_current_frame;
    }
    else
    {
        return ov_pcm_tell(m_vorbis.get());
    }
}

void ogg_reader::init_from_memory()
{
    if(auto error = ov_open_callbacks(m_source.get(), m_vorbis.get(), nullptr, 0, memory_callbacks); error < 0)
        throw std::runtime_error{"swl::ogg_reader can not open the audio file. #" + std::to_string(error)};

    fill_info();

    if(static_cast<bool>(m_options & sound_reader_options::decoded))
    {
        fill_buffer();
        close();
    }
}

void ogg_reader::init_from_stream()
{
    if(const auto error{ov_open_callbacks(m_stream, m_vorbis.get(), nullptr, 0, stream_callbacks)}; error < 0)
        throw std::runtime_error{"swl::ogg_reader can not open the ogg file. #" + std::to_string(error)};

    fill_info();

    if(static_cast<bool>(m_options & sound_reader_options::decoded))
    {
        fill_buffer();
        close();
    }
}

void ogg_reader::fill_info()
{
    const vorbis_info* vorbis_info{ov_info(m_vorbis.get(), 0)};

    sound_info info{};
    info.frame_count   = static_cast<std::uint64_t>(ov_pcm_total(m_vorbis.get(), -1));
    info.channel_count = static_cast<std::uint32_t>(vorbis_info->channels);
    info.frequency     = static_cast<std::uint32_t>(vorbis_info->rate);
    info.seekable      = true;

    set_info(info);
}

void ogg_reader::fill_buffer()
{
    m_decoded_buffer.resize(info().frame_count * info().channel_count);

    std::uint64_t total_read{};
    std::size_t index{};

    while(total_read < info().frame_count)
    {
        float** data{};
        const auto read{ov_read_float(m_vorbis.get(), &data, static_cast<int>(info().frame_count), &m_current_section)};

        for(std::size_t i{}; i < static_cast<std::size_t>(read); ++i)
        {
            for(std::size_t j{}; j < info().channel_count; ++j)
            {
                m_decoded_buffer[index++] = data[j][i];
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
            throw std::runtime_error{"swl::ogg_reader can not read the audio file. #" + std::to_string(read)};
        }
    }
}

void ogg_reader::close()
{
    m_vorbis.reset();
}

std::size_t ogg_reader::sample_size(std::size_t frame_count)
{
    return frame_count * info().channel_count;
}

bool ogg_reader::read_samples_from_buffer(float* output, std::size_t frame_count)
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

bool ogg_reader::read_samples_from_vorbis(float* output, std::size_t frame_count)
{
    std::size_t total_read{};

    while(total_read < frame_count)
    {
        float** data{};
        const auto read{ov_read_float(m_vorbis.get(), &data, static_cast<int>(frame_count - total_read), &m_current_section)};

        std::size_t index{};
        for(std::size_t i{}; i < static_cast<std::size_t>(read); ++i)
        {
            for(std::size_t j{}; j < info().channel_count; ++j)
            {
                output[index++] = data[j][i];
            }
        }

        if(read > 0)
        {
            total_read += read;
            output += read * info().channel_count;
        }
        else if(read == 0)
        {
            std::fill(output, output + (frame_count - total_read) * info().channel_count, 0.0f);

            return false;
        }
        else if(read < 0)
        {
            throw std::runtime_error{"swl::ogg_reader can not read the audio file. #" + std::to_string(read)};
        }
    }

    return true;
}

}
