#include "sound_file.hpp"

#include <fstream>

#include "wave.hpp"
#include "ogg.hpp"

namespace swl
{

static constexpr std::array<std::uint8_t, 4> wave_header{0x52, 0x49, 0x46, 0x46};
static constexpr std::array<std::uint8_t, 4> ogg_header{0x4F, 0x67, 0x67, 0x53};

static audio_file_format detect_format(std::span<const std::uint8_t> data) noexcept
{
    const auto header_data{data.subspan(0, 4)};

    if(std::equal(std::begin(header_data), std::end(header_data), std::begin(wave_header), std::end(wave_header)))
    {
        return audio_file_format::wave;
    }
    else if(std::equal(std::begin(header_data), std::end(header_data), std::begin(ogg_header), std::end(ogg_header)))
    {
        return audio_file_format::ogg;
    }

    return audio_file_format::unknown;
}

static audio_file_format detect_format(std::istream& stream)
{
    std::array<std::uint8_t, 4> header_data{};
    if(!stream.read(reinterpret_cast<char*>(std::data(header_data)), std::size(header_data)))
        throw std::runtime_error{"Can not detect audio file format from stream."};

    stream.seekg(0, std::ios_base::beg);

    return detect_format(header_data);
}

static audio_file_format detect_format(const std::filesystem::path& file)
{
    std::ifstream ifs{file, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not open file \"" + file.string() + "\"."};

    return detect_format(ifs);
}

sound_file_reader::sound_file_reader(const std::filesystem::path& file, sound_reader_options options)
{
    const auto format{detect_format(file)};

    if(format == audio_file_format::wave)
    {
        m_reader = std::make_unique<wave_reader>(file, options);
    }
    else if(format == audio_file_format::ogg)
    {
        m_reader = std::make_unique<ogg_reader>(file, options);
    }
    else
    {
        throw std::runtime_error{"File \"" + file.string() + "\" contains unknown audio format."};
    }
}

sound_file_reader::sound_file_reader(std::span<const std::uint8_t> data, sound_reader_options options)
{
    const auto format{detect_format(data)};

    if(format == audio_file_format::wave)
    {
        m_reader = std::make_unique<wave_reader>(data, options);
    }
    else if(format == audio_file_format::ogg)
    {
        m_reader = std::make_unique<ogg_reader>(data, options);
    }
    else
    {
        throw std::runtime_error{"File contains unknown audio format."};
    }
}

sound_file_reader::sound_file_reader(std::istream& stream, sound_reader_options options)
{
    const auto format{detect_format(stream)};

    if(format == audio_file_format::wave)
    {
        m_reader = std::make_unique<wave_reader>(stream, options);
    }
    else if(format == audio_file_format::ogg)
    {
        m_reader = std::make_unique<ogg_reader>(stream, options);
    }
    else
    {
        throw std::runtime_error{"File contains unknown audio format."};
    }
}

sound_file_reader::sound_file_reader(std::unique_ptr<sound_reader> m_reader) noexcept
:m_reader{std::move(m_reader)}
{

}

}
