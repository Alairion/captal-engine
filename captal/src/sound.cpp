#include "sound.hpp"

#include "engine.hpp"

namespace cpt
{

sound::sound(std::string_view file, swl::load_from_file_t, swl::sound_reader_options options)
:swl::sound_file_reader{file, swl::load_from_file, options}
,swl::sound{engine::instance().audio_mixer(), *this}
{

}

sound::sound(std::string_view data, swl::load_from_memory_t, swl::sound_reader_options options)
:swl::sound_file_reader{data, swl::load_from_memory, options}
,swl::sound{engine::instance().audio_mixer(), *this}
{

}

}
