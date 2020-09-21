#include "sound.hpp"

#include "engine.hpp"

#include <swell/sound_file.hpp>

namespace cpt
{

sound::sound(const std::filesystem::path& file, swl::sound_reader_options options)
:sound{swl::open_file(file, options)}
{

}

sound::sound(std::span<const std::uint8_t> data, swl::sound_reader_options options)
:sound{swl::open_file(data, options)}
{

}

sound::sound(std::istream& stream, swl::sound_reader_options options)
:sound{swl::open_file(stream, options)}
{

}

sound::sound(std::unique_ptr<swl::sound_reader> reader)
:swl::sound{engine::instance().audio_mixer(), std::move(reader)}
{

}

}
