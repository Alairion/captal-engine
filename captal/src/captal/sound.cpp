#include "sound.hpp"

#include "engine.hpp"

#include <swell/sound_file.hpp>

namespace cpt
{

sound::sound(const std::filesystem::path& file, swl::sound_reader_options options)
:sound{swl::open_file(file, options)}
{

}//MIT License
//
//Copyright (c) 2021 Alexy Pellegrini
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.



sound::sound(std::span<const std::uint8_t> data, swl::sound_reader_options options)
:sound{swl::open_file(data, options)}
{

}

sound::sound(std::istream& stream, swl::sound_reader_options options)
:sound{swl::open_file(stream, options)}
{

}

sound::sound(std::unique_ptr<swl::sound_reader> reader)
:swl::sound{engine::instance().audio_world(), std::move(reader)}
{

}

}
