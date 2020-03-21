#include "wave.hpp"

#include <cassert>

namespace swl
{

static std::uint16_t bswap(std::uint16_t value) noexcept
{
    return (value << 8) | (value >> 8);
}

static std::uint32_t bswap(std::uint32_t value) noexcept
{
    value = ((value << 8) & 0xFF00FF00) | ((value >> 8) & 0xFF00FF);
    return (value << 16) | (value >> 16);
}

static bool is_big_endian() noexcept
{
    const std::uint16_t word{1};
    const std::uint8_t* bytes{reinterpret_cast<const std::uint8_t*>(&word)};

    return !bytes[0];
}

static std::int8_t read_int8(const char* data) noexcept
{
    return static_cast<std::int8_t>(data[0]);
}

static std::int16_t read_int16(const char* data) noexcept
{
    return static_cast<std::int16_t>(static_cast<std::uint8_t>(data[0]) | (static_cast<std::uint8_t>(data[1]) << 8));
}

static std::int32_t read_int24(const char* data) noexcept
{
    return static_cast<std::int32_t>(static_cast<std::uint8_t>(data[0]) | (static_cast<std::uint8_t>(data[1]) << 8) | (static_cast<std::uint8_t>(data[2]) << 16));
}

static std::int32_t read_int32(const char* data) noexcept
{
    return static_cast<std::int32_t>(static_cast<std::uint8_t>(data[0]) | (static_cast<std::uint8_t>(data[1]) << 8) | (static_cast<std::uint8_t>(data[2]) << 16) | (static_cast<std::uint8_t>(data[3]) << 24));
}

static float read_sample(const char* data, std::uint32_t bits_per_sample) noexcept
{
    if(bits_per_sample == 8)
    {
        return static_cast<float>(read_int8(data)) / static_cast<float>(0x80);
    }
    else if(bits_per_sample == 16)
    {
        return static_cast<float>(read_int16(data)) / static_cast<float>(0x8000);
    }
    else if(bits_per_sample == 24)
    {
        return static_cast<float>(read_int24(data)) / static_cast<float>(0x800000);
    }
    else if(bits_per_sample == 32)
    {
        return static_cast<float>(read_int32(data)) / static_cast<float>(0x80000000);
    }

    return 0.0f;
}

static void read_samples_impl(const char* data, std::uint32_t bits_per_sample, float* output, std::size_t sample_count) noexcept
{
    const std::size_t byte_per_sample{bits_per_sample / 8u};

    for(std::size_t i{}; i < sample_count; ++i)
        output[i] = read_sample(data + (i * byte_per_sample), bits_per_sample);
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

    if(!ifs.read(reinterpret_cast<char*>(&m_header), sizeof(m_header)))
        throw std::runtime_error{"Too short wave data."};

    swap_header_bytes();
    check_header();

    if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        const std::string data{std::istreambuf_iterator<char>{ifs}, std::istreambuf_iterator<char>{}};

        m_buffer.resize(m_header.data_size / (m_header.bits_per_sample / 8u));
        read_samples_impl(std::data(data), m_header.bits_per_sample, std::data(m_buffer), std::size(m_buffer));
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
        throw std::runtime_error{"Too short wave data."};

    m_header = *reinterpret_cast<const header*>(std::data(data));

    swap_header_bytes();
    check_header();

    if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        m_buffer.resize(m_header.data_size / (m_header.bits_per_sample / 8u));
        read_samples_impl(std::data(data), m_header.bits_per_sample, std::data(m_buffer), std::size(m_buffer));
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

    if(!stream.read(reinterpret_cast<char*>(&m_header), sizeof(m_header)))
        throw std::runtime_error{"Too short wave data."};

    swap_header_bytes();
    check_header();

    if(static_cast<bool>(m_options & sound_reader_options::buffered))
    {
        const std::string data{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};

        m_buffer.resize(m_header.data_size / (m_header.bits_per_sample / 8u));
        read_samples_impl(std::data(data), m_header.bits_per_sample, std::data(m_buffer), std::size(m_buffer));
    }

    m_stream = &stream;
}

bool wave_reader::read_samples(float* output, std::size_t frame_count)
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

void wave_reader::seek_samples(std::uint64_t frame_offset)
{
    m_current_frame = frame_offset;

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

std::uint64_t wave_reader::get_frame_count()
{
    return m_header.data_size / (m_header.bits_per_sample / 8u) / m_header.channel_count;
}

std::uint32_t wave_reader::get_frequency()
{
    return m_header.frequency;
}

std::uint32_t wave_reader::get_channels()
{
    return static_cast<std::uint32_t>(m_header.channel_count);
}

void wave_reader::swap_header_bytes()
{
    if(is_big_endian()) //Wave is little-endian, we need to swap on big endian machines
    {
        m_header.file_size = bswap(m_header.file_size);
        m_header.block_size = bswap(m_header.block_size);
        m_header.format = bswap(m_header.format);
        m_header.channel_count = bswap(m_header.channel_count);
        m_header.frequency = bswap(m_header.frequency);
        m_header.byte_per_second = bswap(m_header.byte_per_second);
        m_header.byte_per_block = bswap(m_header.byte_per_block);
        m_header.bits_per_sample = bswap(m_header.bits_per_sample);
        m_header.data_size = bswap(m_header.data_size);
    }
}

void wave_reader::check_header()
{
    if(m_header.file_type_block_id != file_type_block_id)
        throw std::runtime_error{"Invalid Wave data."};
    if(m_header.file_format_id != file_format_id)
        throw std::runtime_error{"Invalid Wave data."};
    if(m_header.format_block_id != format_block_id)
        throw std::runtime_error{"Invalid Wave data."};

    if(m_header.format != 1)
        throw std::runtime_error{"Invalid Wave format."};
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

bool wave_reader::read_samples_from_memory(float* output, std::size_t frame_count)
{
    if(byte_size(m_current_frame + frame_count) > std::size(m_source))
    {
        const std::size_t max_samples{(std::size(m_source) - byte_size(m_current_frame)) / (m_header.bits_per_sample / 8)};

        read_samples_impl(std::data(m_source) + sample_size(m_current_frame), m_header.bits_per_sample, output, max_samples);
        return false;
    }
    else
    {
        read_samples_impl(std::data(m_source) + sample_size(m_current_frame), m_header.bits_per_sample, output, sample_size(frame_count));
        return true;
    }
}

bool wave_reader::read_samples_from_stream(float* output, std::size_t frame_count)
{
    std::string data{};
    data.resize(byte_size(frame_count));

    m_stream->read(std::data(data), std::size(data));
    m_current_frame += frame_count;

    read_samples_impl(std::data(data), m_header.bits_per_sample, output, sample_size(frame_count));

    return static_cast<std::size_t>(m_stream->gcount()) == std::size(data);
}

}
