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

#include "wave.hpp"

#include <cassert>
#include <algorithm>

#include <captal_foundation/stack_allocator.hpp>

namespace swl
{

template<std::uint32_t Bits>
static std::int32_t extend_sign(std::uint32_t value) noexcept
{
    if(value > (1u << (Bits - 1u)))
    {
        return static_cast<std::int32_t>((0xFFFFFFFFu << Bits) | value);
    }
    else
    {
        return static_cast<std::int32_t>(value);
    }
}


static std::uint8_t read_uint8(const std::uint8_t* data) noexcept
{
    return data[0];
}

static std::int16_t read_int16(const std::uint8_t* data) noexcept
{
    return static_cast<std::int16_t>(data[0] | (data[1] << 8));
}

static std::uint16_t read_uint16(const std::uint8_t* data) noexcept
{
    return static_cast<std::uint16_t>(data[0] | (data[1] << 8));
}

static std::int32_t read_int24(const std::uint8_t* data) noexcept
{
    return extend_sign<24>(static_cast<std::uint32_t>(data[0] | (data[1] << 8) | (data[2] << 16)));
}

static std::int32_t read_int32(const std::uint8_t* data) noexcept
{
    return static_cast<std::int32_t>(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}

static std::uint32_t read_uint32(const std::uint8_t* data) noexcept
{
    return static_cast<std::uint32_t>(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}

static std::array<std::uint8_t, 4> read_bits32(const std::uint8_t* data) noexcept
{
    return {data[0], data[1], data[2], data[3]};
}

static void read_samples(const std::uint8_t* data, std::size_t bits_per_sample, float* output, std::size_t sample_count) noexcept
{
    const std::size_t byte_per_sample{bits_per_sample / 8u};

    if(bits_per_sample == 8)
    {
        for(std::size_t i{}; i < sample_count; ++i)
        {
            output[i] = static_cast<float>(static_cast<std::int32_t>(read_uint8(data + (i * byte_per_sample))) - 128) / static_cast<float>(0x80);
        }
    }
    else if(bits_per_sample == 16)
    {
        for(std::size_t i{}; i < sample_count; ++i)
        {
            output[i] = static_cast<float>(read_int16(data + (i * byte_per_sample))) / static_cast<float>(0x8000);
        }
    }
    else if(bits_per_sample == 24)
    {
        for(std::size_t i{}; i < sample_count; ++i)
        {
            output[i] = static_cast<float>(read_int24(data + (i * byte_per_sample))) / static_cast<float>(0x800000);
        }
    }
    else if(bits_per_sample == 32)
    {
        for(std::size_t i{}; i < sample_count; ++i)
        {
            output[i] = static_cast<float>(read_int32(data + (i * byte_per_sample))) / static_cast<float>(0x80000000);
        }
    }
}

static constexpr std::array<std::uint8_t, 4> riff_block_id  {0x52, 0x49, 0x46, 0x46};
static constexpr std::array<std::uint8_t, 4> riff_type_wave {0x57, 0x41, 0x56, 0x45};
static constexpr std::array<std::uint8_t, 4> format_block_id{0x66, 0x6D, 0x74, 0x20};
static constexpr std::array<std::uint8_t, 4> data_block_id  {0x64, 0x61, 0x74, 0x61};

class wave_decoder
{
public:
    wave_decoder(std::span<const std::uint8_t> source)
    :m_source{source}
    {
        init();
    }

    wave_decoder(std::istream& stream)
    :m_stream{&stream}
    {
        init();
    }

    ~wave_decoder() = default;
    wave_decoder(const wave_decoder&) = delete;
    wave_decoder& operator=(const wave_decoder&) = delete;
    wave_decoder(wave_decoder&&) noexcept = delete;
    wave_decoder& operator=(wave_decoder&&) noexcept = delete;

    const sound_info& info() const noexcept
    {
        return m_info;
    }

    std::size_t data_offset() const noexcept
    {
        return m_data_offset;
    }

    std::size_t bits_per_sample() const noexcept
    {
        return m_bits_per_sample;
    }

private:
    void init()
    {
        std::array<std::uint8_t, 12> header_data;
        read_source(std::data(header_data), std::size(header_data));
        read_header(std::data(header_data));

        std::array<std::uint8_t, 8> block_header;
        while(read_source(std::data(block_header), std::size(block_header)))
        {
            read_block(std::data(block_header));
        }

        if(m_info.channel_count == 0 || m_info.frequency == 0 || m_bits_per_sample == 0)
        {
            throw std::runtime_error{"swl::wave_reader invalid format."};
        }

        m_info.frame_count = m_data_size / (m_bits_per_sample / 8u) / m_info.channel_count;
        m_info.seekable    = true;
    }

    void read_header(const std::uint8_t* header_data)
    {
        if(read_bits32(header_data) != riff_block_id)
        {
            throw std::runtime_error{"swl::wave_reader first block is not RIFF."};
        }

        if(read_bits32(header_data + 8) != riff_type_wave)
        {
            throw std::runtime_error{"swl::wave_reader invalid RIFF type. Only WAVE is supported."};
        }
    }

    void read_block(const std::uint8_t* block)
    {
        const auto id  {read_bits32(block)};
        const auto size{read_uint32(block + 4)};

        if(id == format_block_id)
        {
            std::array<std::uint8_t, 16> format_data;
            read_source(std::data(format_data), std::size(format_data));
            read_format_block(std::data(format_data));

            if(size > 16)
            {
                skip_source(align_up(size - 16u, 2u));
            }
        }
        else if(id == data_block_id)
        {
            m_data_offset = tell_source();
            m_data_size   = static_cast<std::size_t>(size);

            skip_source(size);
        }
        else
        {
            skip_source(size);
        }
    }

    void read_format_block(const std::uint8_t* block)
    {
        if(read_uint16(block) != 1)
        {
            throw std::runtime_error{"swl::wave_reader invalid format. Only uncompressed (PCM) data are supported."};
        }

        m_info.channel_count = static_cast<std::uint32_t>(read_uint16(block + 2));
        m_info.frequency     = read_uint32(block + 4);
        m_bits_per_sample    = static_cast<std::size_t>(read_uint16(block + 14));
    }

    bool read_source(std::uint8_t* output, std::size_t size)
    {
        if(std::size(m_source) > 0)
        {
            if(m_source_offset + size > std::size(m_source))
            {
                return false;
            }

            std::copy_n(std::data(m_source) + m_source_offset, size, output);
            m_source_offset += size;

            return true;
        }
        else
        {
            return static_cast<bool>(m_stream->read(reinterpret_cast<char*>(output), static_cast<std::streamsize>(size)));
        }
    }

    bool seek_source(std::size_t position)
    {
        if(std::size(m_source) > 0)
        {
            if(position > std::size(m_source))
            {
                return false;
            }

            m_source_offset = position;

            return true;
        }
        else
        {
            return static_cast<bool>(m_stream->seekg(static_cast<std::streamoff>(position), std::ios_base::beg));
        }
    }

    std::size_t tell_source()
    {
        if(std::size(m_source) > 0)
        {
            return m_source_offset;
        }
        else
        {
            return static_cast<std::size_t>(m_stream->tellg());
        }
    }

    bool skip_source(std::size_t size)
    {
        if(std::size(m_source) > 0)
        {
            if(m_source_offset + size > std::size(m_source))
            {
                return false;
            }

            m_source_offset += size;

            return true;
        }
        else
        {
            return static_cast<bool>(m_stream->seekg(static_cast<std::streamoff>(size), std::ios_base::cur));
        }
    }

private:
    std::size_t m_source_offset{};
    std::span<const std::uint8_t> m_source{};
    std::istream* m_stream{};

    sound_info m_info{};
    std::size_t m_bits_per_sample{};
    std::size_t m_data_offset{};
    std::size_t m_data_size{};
};

wave_reader::wave_reader(const std::filesystem::path& file, sound_reader_options options)
:m_options{options}
{
    std::ifstream ifs{file, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not read file \"" + file.string() + "\"."};

    wave_decoder decoder{ifs};
    set_info(decoder.info());
    m_data_offset     = decoder.data_offset();
    m_bits_per_sample = decoder.bits_per_sample();

    if(static_cast<bool>(m_options & sound_reader_options::decoded))
    {
        std::vector<std::uint8_t> data{};
        data.resize(byte_size(info().frame_count));

        ifs.clear();
        ifs.seekg(m_data_offset, std::ios_base::beg);
        if(!ifs.read(reinterpret_cast<char*>(std::data(data)), static_cast<std::streamsize>(std::size(data))))
            throw std::runtime_error{"Too short wave data."};

        const auto sample_count{sample_size(info().frame_count)};

        m_decoded_buffer.resize(sample_count);
        read_samples(std::data(data), m_bits_per_sample, std::data(m_decoded_buffer), sample_count);
    }
    else if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        m_source_buffer.resize(byte_size(info().frame_count));

        ifs.clear();
        ifs.seekg(m_data_offset, std::ios_base::beg);
        if(!ifs.read(reinterpret_cast<char*>(std::data(m_source_buffer)), static_cast<std::streamsize>(std::size(m_source_buffer))))
            throw std::runtime_error{"Too short wave data."};

        m_source = m_source_buffer;
    }
    else
    {
        m_file   = std::move(ifs);
        m_stream = &m_file;
    }

    seek(0);
}

wave_reader::wave_reader(std::span<const std::uint8_t> data, sound_reader_options options)
:m_options{options}
{
    wave_decoder decoder{data};
    set_info(decoder.info());
    m_data_offset     = decoder.data_offset();
    m_bits_per_sample = decoder.bits_per_sample();

    if(static_cast<bool>(m_options & sound_reader_options::decoded))
    {
        m_decoded_buffer.resize(sample_size(info().frame_count));

        read_samples(std::data(data) + m_data_offset, m_bits_per_sample, std::data(m_decoded_buffer), std::size(m_decoded_buffer));
    }
    else if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        data = data.subspan(m_data_offset, byte_size(info().frame_count));

        m_source_buffer.resize(std::size(data));
        std::copy(std::begin(data), std::end(data), std::begin(m_source_buffer));

        m_source = m_source_buffer;
    }
    else
    {
        m_source = data.subspan(m_data_offset, byte_size(info().frame_count));
    }

    seek(0);
}

wave_reader::wave_reader(std::istream& stream, sound_reader_options options)
:m_options{options}
{
    assert(stream && "Invalid stream.");

    wave_decoder decoder{stream};
    set_info(decoder.info());
    m_data_offset     = decoder.data_offset();
    m_bits_per_sample = decoder.bits_per_sample();

    if(static_cast<bool>(m_options & sound_reader_options::decoded))
    {
        m_decoded_buffer.resize(sample_size(info().frame_count));

        std::vector<std::uint8_t> data{};
        data.resize(byte_size(info().frame_count));

        stream.clear();
        stream.seekg(m_data_offset, std::ios_base::beg);
        if(!stream.read(reinterpret_cast<char*>(std::data(data)), static_cast<std::streamsize>(std::size(data))))
            throw std::runtime_error{"Too short wave data."};

        read_samples(std::data(data), m_bits_per_sample, std::data(m_decoded_buffer), sample_size(info().frame_count));
    }
    else if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        m_source_buffer.resize(byte_size(info().frame_count));

        stream.clear();
        stream.seekg(m_data_offset, std::ios_base::beg);
        if(!stream.read(reinterpret_cast<char*>(std::data(m_source_buffer)), static_cast<std::streamsize>(std::size(m_source_buffer))))
            throw std::runtime_error{"Too short wave data."};

        m_source = m_source_buffer;
    }
    else
    {
        m_stream = &stream;
    }

    seek(0);
}

bool wave_reader::read(float* output, std::size_t frame_count)
{
    if(static_cast<bool>(m_options & sound_reader_options::decoded))
    {
        return read_samples_from_buffer(output, frame_count);
    }
    else if(std::size(m_source) > 0)
    {
        return read_samples_from_memory(output, frame_count);
    }
    else if(m_stream)
    {
        return read_samples_from_stream(output, frame_count);
    }

    return false;
}

void wave_reader::seek(std::uint64_t frame_offset)
{
    m_current_frame = frame_offset;

    if(m_stream)
    {
        m_stream->clear();
        m_stream->seekg(m_data_offset + byte_size(frame_offset));
    }
}

std::uint64_t wave_reader::tell()
{
    return m_current_frame;
}

std::size_t wave_reader::sample_size(std::size_t frame_count)
{
    return frame_count * info().channel_count;
}

std::size_t wave_reader::byte_size(std::size_t frame_count)
{
    return frame_count * info().channel_count * (m_bits_per_sample / 8);
}

bool wave_reader::read_samples_from_buffer(float* output, std::size_t frame_count)
{
    if(sample_size(m_current_frame + frame_count) > std::size(m_decoded_buffer))
    {
        const auto begin{std::data(m_decoded_buffer) + sample_size(m_current_frame)};
        const auto end  {std::data(m_decoded_buffer) + std::size(m_decoded_buffer)};

        std::fill(std::copy(begin, end, output), output + sample_size(frame_count), 0.0f);

        m_current_frame += frame_count;

        return false;
    }
    else
    {
        const auto begin{std::data(m_decoded_buffer) + sample_size(m_current_frame)};

        std::copy(begin, begin + sample_size(frame_count), output);
        m_current_frame += frame_count;

        return true;
    }
}

bool wave_reader::read_samples_from_memory(float* output, std::size_t frame_count)
{
    if(byte_size(m_current_frame + frame_count) > std::size(m_source))
    {
        const auto size{byte_size(m_current_frame)};
        const auto max {(std::size(m_source) - size) / (m_bits_per_sample / 8)};

        read_samples(std::data(m_source) + size, m_bits_per_sample, output, max);
        std::fill(output + max, output + sample_size(frame_count), 0.0f);

        m_current_frame += frame_count;

        return false;
    }
    else
    {
        read_samples(std::data(m_source) + byte_size(m_current_frame), m_bits_per_sample, output, sample_size(frame_count));

        m_current_frame += frame_count;

        return true;
    }
}

bool wave_reader::read_samples_from_stream(float* output, std::size_t frame_count)
{
    m_source_buffer.clear();
    m_source_buffer.resize(byte_size(frame_count));

    m_stream->read(reinterpret_cast<char*>(std::data(m_source_buffer)), std::size(m_source_buffer));
    read_samples(std::data(m_source_buffer), m_bits_per_sample, output, sample_size(frame_count));

    m_current_frame += frame_count;

    return static_cast<std::size_t>(m_stream->gcount()) == std::size(m_source_buffer);
}


}
