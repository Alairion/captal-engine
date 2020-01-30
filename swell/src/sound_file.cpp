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

bool is_valid(std::string_view file, load_from_file_t)
{
    std::ifstream ifs{std::string{file}, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not open file \"" + std::string{file} + "\"."};

    header header_data{};
    ifs.read(reinterpret_cast<char*>(&header_data), sizeof(header));

    return header_data.file_type_block_id == file_type_block_id && header_data.file_format_id == file_format_id;
}

bool is_valid(std::string_view data, load_from_memory_t)
{
    const header header_data{*reinterpret_cast<const header*>(std::data(data))};

    return header_data.file_type_block_id == file_type_block_id && header_data.file_format_id == file_format_id;
}

}

namespace ogg
{

static constexpr std::array<char, 4> file_header{0x4F, 0x67, 0x67, 0x53};

bool is_valid(std::string_view file, load_from_file_t)
{
    std::ifstream ifs{std::string{file}, std::ios_base::binary};
    if(!ifs)
        throw std::runtime_error{"Can not open file \"" + std::string{file} + "\"."};

    std::array<char, 4> header_data{};
    ifs.read(std::data(header_data), std::size(header_data));

    return header_data == file_header;
}

bool is_valid(std::string_view data, load_from_memory_t)
{
    const std::array<char, 4> header_data{*reinterpret_cast<const std::array<char, 4>*>(std::data(data))};

    return header_data == file_header;
}

}

sound_file_reader::sound_file_reader(std::string_view file, load_from_file_t, sound_reader_options options)
{
    if(wave::is_valid(file, load_from_file))
    {
        m_reader = std::make_unique<wave_reader>(file, load_from_file, options);
    }
    else if(ogg::is_valid(file, load_from_file))
    {
        m_reader = std::make_unique<ogg_reader>(file, load_from_file, options);
    }
    else
    {
        throw std::runtime_error{"File \"" + std::string{file} + "\" contains unknown audio format."};
    }
}

sound_file_reader::sound_file_reader(std::string_view data, load_from_memory_t, sound_reader_options options)
{
    if(wave::is_valid(data, load_from_memory))
    {
        m_reader = std::make_unique<wave_reader>(data, load_from_memory, options);
    }
    else if(ogg::is_valid(data, load_from_memory))
    {
        m_reader = std::make_unique<ogg_reader>(data, load_from_memory, options);
    }
    else
    {
        throw std::runtime_error{"File contains unknown audio format."};
    }
}

}
