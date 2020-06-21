#include "wave.hpp"

#include <cassert>

namespace swl
{

static std::int8_t read_int8(const char* data) noexcept
{
    return static_cast<std::int8_t>(data[0]);
}

static std::int16_t read_int16(const char* data) noexcept
{
    return static_cast<std::int16_t>(static_cast<std::uint8_t>(data[0]) |
                                    (static_cast<std::uint8_t>(data[1]) << 8));
}

static std::int16_t read_uint16(const char* data) noexcept
{
    return static_cast<std::uint16_t>(static_cast<std::uint8_t>(data[0]) |
                                     (static_cast<std::uint8_t>(data[1]) << 8));
}

static std::int32_t read_int24(const char* data) noexcept
{
    return static_cast<std::int32_t>(static_cast<std::uint8_t>(data[0]) |
                                    (static_cast<std::uint8_t>(data[1]) << 8) |
                                    (static_cast<std::uint8_t>(data[2]) << 16));
}

static std::int32_t read_int32(const char* data) noexcept
{
    return static_cast<std::int32_t>(static_cast<std::uint8_t>(data[0]) |
                                    (static_cast<std::uint8_t>(data[1]) << 8) |
                                    (static_cast<std::uint8_t>(data[2]) << 16) |
                                    (static_cast<std::uint8_t>(data[3]) << 24));
}

static std::int32_t read_uint32(const char* data) noexcept
{
    return static_cast<std::uint32_t>(static_cast<std::uint8_t>(data[0]) |
                                     (static_cast<std::uint8_t>(data[1]) << 8) |
                                     (static_cast<std::uint8_t>(data[2]) << 16) |
                                     (static_cast<std::uint8_t>(data[3]) << 24));
}

static std::array<char, 4> read_bits32(const char* data) noexcept
{
    return {data[0], data[1], data[2], data[3]};
}

static void read_samples(const char* data, std::uint32_t bits_per_sample, float* output, std::size_t sample_count) noexcept
{
    const std::size_t byte_per_sample{bits_per_sample / 8u};

    if(bits_per_sample == 8)
    {
        for(std::size_t i{}; i < sample_count; ++i)
        {
            output[i] = static_cast<float>(read_int8(data + (i * byte_per_sample))) / static_cast<float>(0x80);
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

static constexpr std::array<char, 4> file_type_block_id{0x52, 0x49, 0x46, 0x46};
static constexpr std::array<char, 4> file_format_id{0x57, 0x41, 0x56, 0x45};
static constexpr std::array<char, 4> format_block_id{0x66,0x6D, 0x74,0x20};

wave_reader::wave_reader(const std::filesystem::path& file, sound_reader_options options)
:m_options{options}
{
    std::ifstream ifs{file, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not read file \"" + file.string() + "\"."};

    std::array<char, 44> header_data{};
    if(!ifs.read(std::data(header_data), std::size(header_data)))
        throw std::runtime_error{"Too short wave data."};

    read_header(header_data);
    check_header();

    if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        const std::string data{std::istreambuf_iterator<char>{ifs}, std::istreambuf_iterator<char>{}};

        m_buffer.resize(m_header.data_size / (m_header.bits_per_sample / 8u));
        read_samples(std::data(data), m_header.bits_per_sample, std::data(m_buffer), std::size(m_buffer));
    }
    else
    {
        m_file = std::move(ifs);
        m_stream = &m_file;
    }
}

wave_reader::wave_reader(std::string_view data, sound_reader_options options)
:m_options{options}
{
    if(std::size(data) < 44)
    {
        throw std::runtime_error{"Too short wave data."};
    }

    std::array<char, 44> header_data{};
    std::copy(std::begin(data), std::begin(data) + 44, std::begin(header_data));

    read_header(header_data);

    if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        m_buffer.resize(m_header.data_size / (m_header.bits_per_sample / 8u));
        read_samples(std::data(data), m_header.bits_per_sample, std::data(m_buffer), std::size(m_buffer));
    }
    else
    {
        m_source = data.substr(44); //don't keep header
    }
}

wave_reader::wave_reader(std::istream& stream, sound_reader_options options)
:m_options{options}
{
    assert(stream && "Invalid stream.");

    std::array<char, 44> header_data{};
    if(!stream.read(std::data(header_data), std::size(header_data)))
        throw std::runtime_error{"Too short wave data."};

    read_header(header_data);

    if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        const std::string data{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};

        m_buffer.resize(m_header.data_size / (m_header.bits_per_sample / 8u));
        read_samples(std::data(data), m_header.bits_per_sample, std::data(m_buffer), std::size(m_buffer));
    }

    m_stream = &stream;
}

bool wave_reader::read(float* output, std::size_t frame_count)
{
    if(static_cast<bool>(m_options & sound_reader_options::buffered))
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
    m_current_frame = static_cast<std::uint32_t>(frame_offset);

    if(m_file.is_open())
    {
        m_file.clear();
        m_file.seekg(44 + byte_size(frame_offset));
    }
    else if(m_stream)
    {
        m_stream->clear();
        m_stream->seekg(44 + byte_size(frame_offset));
    }
}

std::uint64_t wave_reader::tell()
{
    return static_cast<std::uint64_t>(m_current_frame);
}

std::uint64_t wave_reader::frame_count()
{
    return m_header.data_size / (m_header.bits_per_sample / 8u) / m_header.channel_count;
}

std::uint32_t wave_reader::frequency()
{
    return m_header.frequency;
}

std::uint32_t wave_reader::channel_count()
{
    return static_cast<std::uint32_t>(m_header.channel_count);
}

void wave_reader::read_header(const std::array<char, 44>& header_data)
{
    const char* const data{std::data(header_data)};

    m_header.file_type_block_id = read_bits32(data + 0);
    m_header.file_size          = read_uint32(data + 4);
    m_header.file_format_id     = read_bits32(data + 8);
    m_header.format_block_id    = read_bits32(data + 12);
    m_header.block_size         = read_uint32(data + 16);
    m_header.format             = read_uint16(data + 20);
    m_header.channel_count      = read_uint16(data + 22);
    m_header.frequency          = read_uint32(data + 24);
    m_header.byte_per_second    = read_uint32(data + 28);
    m_header.byte_per_block     = read_uint16(data + 32);
    m_header.bits_per_sample    = read_uint16(data + 34);
    m_header.data_bloc_id       = read_bits32(data + 36);
    m_header.data_size          = read_uint32(data + 40);

    check_header();
}

void wave_reader::check_header()
{
    if(m_header.file_type_block_id != file_type_block_id)
    {
        throw std::runtime_error{"Invalid Wave data."};
    }

    if(m_header.file_format_id != file_format_id)
    {
        throw std::runtime_error{"Invalid Wave data."};
    }

    if(m_header.format_block_id != format_block_id)
    {
        throw std::runtime_error{"Invalid Wave data."};
    }

    if(m_header.format != 1)
    {
        throw std::runtime_error{"Invalid Wave format."};
    }
}

std::size_t wave_reader::sample_size(std::size_t frame_count)
{
    return frame_count * m_header.channel_count;
}

std::size_t wave_reader::byte_size(std::size_t frame_count)
{
    return frame_count * m_header.channel_count * (m_header.bits_per_sample / 8);
}

bool wave_reader::read_samples_from_buffer(float* output, std::size_t frame_count)
{
    if(sample_size(m_current_frame + frame_count) > std::size(m_buffer))
    {
        const auto* const begin{std::data(m_buffer) + sample_size(m_current_frame)};
        const auto* const end{std::data(m_buffer) + std::size(m_buffer)};

        std::copy(begin, end, output);
        m_current_frame += frame_count;

        return false;
    }
    else
    {
        const auto* const begin{std::data(m_buffer) + sample_size(m_current_frame)};

        std::copy(begin, begin + sample_size(frame_count), output);
        m_current_frame += frame_count;

        return true;
    }
}

bool wave_reader::read_samples_from_memory(float* output, std::size_t frame_count)
{
    if(byte_size(m_current_frame + frame_count) > std::size(m_source))
    {
        const std::size_t max_samples{(std::size(m_source) - byte_size(m_current_frame)) / (m_header.bits_per_sample / 8)};

        read_samples(std::data(m_source) + sample_size(m_current_frame), m_header.bits_per_sample, output, max_samples);
        return false;
    }
    else
    {
        read_samples(std::data(m_source) + sample_size(m_current_frame), m_header.bits_per_sample, output, sample_size(frame_count));
        return true;
    }
}

bool wave_reader::read_samples_from_stream(float* output, std::size_t frame_count)
{
    std::string data{};
    data.resize(byte_size(frame_count));

    m_stream->read(std::data(data), std::size(data));
    m_current_frame += frame_count;

    read_samples(std::data(data), m_header.bits_per_sample, output, sample_size(frame_count));

    return static_cast<std::size_t>(m_stream->gcount()) == std::size(data);
}

}
