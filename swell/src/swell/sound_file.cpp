#include "sound_file.hpp"

#include <fstream>

#include "wave.hpp"
#include "ogg.hpp"
#include "flac.hpp"

namespace swl
{

static constexpr std::array<std::uint8_t, 4> wave_header{0x52, 0x49, 0x46, 0x46};
static constexpr std::array<std::uint8_t, 4> ogg_header {0x4F, 0x67, 0x67, 0x53};
static constexpr std::array<std::uint8_t, 4> flac_header{0x66, 0x4c, 0x61, 0x43};

audio_file_format file_format(std::span<const std::uint8_t> data) noexcept
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
    else if(std::equal(std::begin(header_data), std::end(header_data), std::begin(flac_header), std::end(flac_header)))
    {
        return audio_file_format::flac;
    }

    return audio_file_format::unknown;
}

audio_file_format file_format(std::istream& stream)
{
    std::array<std::uint8_t, 4> header_data{};
    if(!stream.read(reinterpret_cast<char*>(std::data(header_data)), std::size(header_data)))
        throw std::runtime_error{"Can not detect audio file format from stream."};

    stream.seekg(0, std::ios_base::beg);
    const auto output{file_format(header_data)};
    stream.seekg(0, std::ios_base::beg);

    return output;
}

audio_file_format file_format(const std::filesystem::path& file)
{
    std::ifstream ifs{file, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not open file \"" + file.string() + "\"."};

    return file_format(ifs);
}

template<typename... Args>
std::unique_ptr<sound_reader> make_reader(audio_file_format format, Args&&... args)
{
    if(format == audio_file_format::wave)
    {
        return std::make_unique<wave_reader>(std::forward<Args>(args)...);
    }
    else if(format == audio_file_format::ogg)
    {
        return std::make_unique<ogg_reader>(std::forward<Args>(args)...);
    }
    else if(format == audio_file_format::flac)
    {
        return std::make_unique<flac_reader>(std::forward<Args>(args)...);
    }

    throw std::runtime_error{"File contains unknown audio format."};
}

std::unique_ptr<sound_reader> open_file(const std::filesystem::path& file, sound_reader_options options)
{
    return make_reader(file_format(file), file, options);
}

std::unique_ptr<sound_reader> open_file(std::span<const std::uint8_t> data, sound_reader_options options)
{
    return make_reader(file_format(data), data, options);
}

std::unique_ptr<sound_reader> open_file(std::istream& stream, sound_reader_options options)
{
    return make_reader(file_format(stream), stream, options);
}

}
