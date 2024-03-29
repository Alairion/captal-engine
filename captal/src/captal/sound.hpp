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

#ifndef CAPTAL_SOUND_HPP_INCLUDED
#define CAPTAL_SOUND_HPP_INCLUDED

#include "config.hpp"

#include <filesystem>
#include <span>
#include <istream>

#include <swell/audio_world.hpp>

namespace cpt
{

class CAPTAL_API sound : swl::sound
{
public:
    sound();
    explicit sound(const std::filesystem::path& file, swl::sound_reader_options options = swl::sound_reader_options::none);
    explicit sound(std::span<const std::uint8_t> data, swl::sound_reader_options options = swl::sound_reader_options::none);
    explicit sound(std::istream& stream, swl::sound_reader_options options = swl::sound_reader_options::none);
    explicit sound(std::unique_ptr<swl::sound_reader> reader);

    ~sound() = default;
    sound(const sound&) = delete;
    sound& operator=(const sound&) = delete;
    sound(sound&&) noexcept = default;
    sound& operator=(sound&&) noexcept = default;

    using swl::sound::start;
    using swl::sound::stop;
    using swl::sound::pause;
    using swl::sound::resume;
    using swl::sound::fade_out;
    using swl::sound::fade_in;

    using swl::sound::set_volume;
    using swl::sound::set_loop_points;
    using swl::sound::enable_spatialization;
    using swl::sound::disable_spatialization;
    using swl::sound::relative_spatialization;
    using swl::sound::absolute_spatialization;
    using swl::sound::set_minimum_distance;
    using swl::sound::set_attenuation;
    using swl::sound::move;
    using swl::sound::move_to;
    using swl::sound::change_reader;

    using swl::sound::status;
    using swl::sound::volume;
    using swl::sound::loop_points;
    using swl::sound::is_spatialization_enabled;
    using swl::sound::is_spatialization_relative;
    using swl::sound::minimum_distance;
    using swl::sound::attenuation;
    using swl::sound::position;
    using swl::sound::tell;

    using swl::sound::frames_to_time;
    using swl::sound::time_to_frame;

    swl::sound& get_sound() noexcept
    {
        return *this;
    }

    const swl::sound& get_sound() const noexcept
    {
        return *this;
    }
};

}

#endif
