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

#ifndef SWELL_WAVE_HPP_INCLUDED
#define SWELL_WAVE_HPP_INCLUDED

#include "config.hpp"

#include <fstream>
#include <filesystem>
#include <span>
#include <variant>
#include <vector>

#include "sound_reader.hpp"

namespace swl
{

class SWELL_API wave_reader final : public sound_reader
{
public:
    wave_reader() = default;
    wave_reader(const std::filesystem::path& file, sound_reader_options options = sound_reader_options::none);
    wave_reader(std::span<const std::uint8_t> data, sound_reader_options options = sound_reader_options::none);
    wave_reader(std::istream& stream, sound_reader_options options = sound_reader_options::none);

    ~wave_reader() = default;
    wave_reader(const wave_reader&) = delete;
    wave_reader& operator=(const wave_reader&) = delete;
    wave_reader(wave_reader&& other) noexcept = default;
    wave_reader& operator=(wave_reader&& other) noexcept = default;

    bool read(float* output, std::size_t frame_count) override;
    void seek(std::uint64_t frame_offset) override;
    std::uint64_t tell() override;

private:
    std::size_t sample_size(std::size_t frame_count);
    std::size_t byte_size(std::size_t frame_count);

    bool read_samples_from_buffer(float* output, std::size_t frame_count);
    bool read_samples_from_memory(float* output, std::size_t frame_count);
    bool read_samples_from_stream(float* output, std::size_t frame_count);

private:
    using buffered_state = std::variant<std::vector<float>, std::vector<std::uint8_t>, std::span<const std::uint8_t>>;

private:
    sound_reader_options m_options{};
    std::uint64_t m_current_frame{};
    std::size_t m_data_offset{};
    std::size_t m_bits_per_sample{};

    std::vector<std::uint8_t> m_source_buffer{};
    std::ifstream m_file{};

    std::vector<float> m_decoded_buffer{};
    std::span<const std::uint8_t> m_source{};
    std::istream* m_stream{};
};

}

#endif
