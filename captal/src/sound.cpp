#include "sound.hpp"

#include "engine.hpp"

namespace cpt
{

sound::sound(const std::filesystem::path& file, swl::sound_reader_options options)
:swl::sound_file_reader{file, options}
,swl::sound{engine::instance().audio_mixer(), *this}
{

}

sound::sound(const std::string_view& data, swl::sound_reader_options options)
:swl::sound_file_reader{data, options}
,swl::sound{engine::instance().audio_mixer(), *this}
{

}

sound::sound(std::istream& stream, swl::sound_reader_options options)
:swl::sound_file_reader{stream, options}
,swl::sound{engine::instance().audio_mixer(), *this}
{

}

sound::sound(swl::sound_file_reader reader)
:swl::sound_file_reader{std::move(reader)}
,swl::sound{engine::instance().audio_mixer(), *this}
{

}

}
