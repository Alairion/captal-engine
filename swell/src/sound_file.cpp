#include "sound_file.hpp"

#include <fstream>

#include "wave.hpp"
#include "ogg.hpp"

namespace swl
{

namespace wave
{

struct header
{
    std::array<char, 4> file_type_block_id{};
    std::uint32_t file_size{};
    std::array<char, 4> file_format_id{};
};

static constexpr std::array<char, 4> file_type_block_id{0x52, 0x49, 0x46, 0x46};
static constexpr std::array<char, 4> file_format_id{0x57, 0x41, 0x56, 0x45};

static bool is_valid(const std::string_view& data)
{
    const header header_data{*reinterpret_cast<const header*>(std::data(data))};

    return header_data.file_type_block_id == file_type_block_id && header_data.file_format_id == file_format_id;
}

static bool is_valid(std::istream& stream)
{
    header header_data{};
    stream.read(reinterpret_cast<char*>(&header_data), sizeof(header));

    return header_data.file_type_block_id == file_type_block_id && header_data.file_format_id == file_format_id;
}

static bool is_valid(const std::filesystem::path& file)
{
    std::ifstream ifs{file, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not open file \"" + file.string() + "\"."};

    return is_valid(ifs);
}

}

namespace ogg
{

static constexpr std::array<char, 4> file_header{0x4F, 0x67, 0x67, 0x53};

static bool is_valid(std::istream& stream)
{
    std::array<char, 4> header_data{};
    stream.read(std::data(header_data), std::size(header_data));

    return header_data == file_header;
}

static bool is_valid(const std::string_view& data)
{
    const std::array<char, 4> header_data{*reinterpret_cast<const std::array<char, 4>*>(std::data(data))};

    return header_data == file_header;
}

static bool is_valid(const std::filesystem::path& file)
{
    std::ifstream ifs{file, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not open file \"" + file.string() + "\"."};

    return is_valid(ifs);
}

}

sound_file_reader::sound_file_reader(const std::filesystem::path& file, sound_reader_options options)
{
    if(wave::is_valid(file))
    {
        m_reader = std::make_unique<wave_reader>(file, options);
    }
    else if(ogg::is_valid(file))
    {
        m_reader = std::make_unique<ogg_reader>(file, options);
    }
    else
    {
        throw std::runtime_error{"File \"" + file.string() + "\" contains unknown audio format."};
    }
}

sound_file_reader::sound_file_reader(const std::string_view& data, sound_reader_options options)
{
    if(wave::is_valid(data))
    {
        m_reader = std::make_unique<wave_reader>(data, options);
    }
    else if(ogg::is_valid(data))
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
    if(wave::is_valid(stream))
    {
        m_reader = std::make_unique<wave_reader>(stream, options);
    }
    else if(ogg::is_valid(stream))
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
