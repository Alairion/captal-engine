//MIT License
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
    ogg = 2,
    flac = 3
};

SWELL_API audio_file_format file_format(std::span<const std::uint8_t> data) noexcept;
SWELL_API audio_file_format file_format(std::istream& stream);
SWELL_API audio_file_format file_format(const std::filesystem::path& file);

SWELL_API std::unique_ptr<sound_reader> open_file(const std::filesystem::path& file, sound_reader_options options = sound_reader_options::none);
SWELL_API std::unique_ptr<sound_reader> open_file(std::span<const std::uint8_t> data, sound_reader_options options = sound_reader_options::none);
SWELL_API std::unique_ptr<sound_reader> open_file(std::istream& stream, sound_reader_options options = sound_reader_options::none);

}

#endif
