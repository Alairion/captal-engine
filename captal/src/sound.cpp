#include "sound.hpp"

#include "engine.hpp"

namespace cpt
{

sound::sound(const std::filesystem::path& file, swl::sound_reader_options options)
:swl::sound{engine::instance().audio_mixer(), std::make_unique<swl::sound_file_reader>(file, options)}
{

}

sound::sound(std::span<const std::uint8_t> data, swl::sound_reader_options options)
:swl::sound{engine::instance().audio_mixer(), std::make_unique<swl::sound_file_reader>(data, options)}
{

}

sound::sound(std::istream& stream, swl::sound_reader_options options)
:swl::sound{engine::instance().audio_mixer(), std::make_unique<swl::sound_file_reader>(stream, options)}
{

}

sound::sound(std::unique_ptr<swl::sound_reader> reader)
:swl::sound{engine::instance().audio_mixer(), std::move(reader)}
{

}

}
