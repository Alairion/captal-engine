#ifndef SWELL_SOUND_FILE_HPP_INCLUDED
#define SWELL_SOUND_FILE_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <filesystem>
#include <span>
#include <istream>

#include "sound_reader.hpp"

namespace swl
{

enum class audio_file_format : std::uint32_t
{
    unknown = 0,
    wave = 1,
    ogg = 2
};

SWELL_API audio_file_format file_format(std::span<const std::uint8_t> data) noexcept;
SWELL_API audio_file_format file_format(std::istream& stream);
SWELL_API audio_file_format file_format(const std::filesystem::path& file);

SWELL_API std::unique_ptr<sound_reader> open_file(const std::filesystem::path& file, sound_reader_options options = sound_reader_options::none);
SWELL_API std::unique_ptr<sound_reader> open_file(std::span<const std::uint8_t> data, sound_reader_options options = sound_reader_options::none);
SWELL_API std::unique_ptr<sound_reader> open_file(std::istream& stream, sound_reader_options options = sound_reader_options::none);

}

#endif
