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

#ifndef SWELL_OGG_HPP_INCLUDED
#define SWELL_OGG_HPP_INCLUDED

#include "config.hpp"

#include <fstream>
#include <filesystem>
#include <span>
#include <vector>
#include <memory>

#include "sound_reader.hpp"

struct OggVorbis_File;

namespace swl
{

namespace impl
{

struct ogg_memory_stream
{
    std::span<const std::uint8_t> data{};
    std::size_t position{};
};

}


class SWELL_API ogg_reader : public sound_reader
{
    struct vorbis_deleter
    {
        void operator()(OggVorbis_File* file);
    };

public:
    ogg_reader() = default;
    ogg_reader(const std::filesystem::path& file, sound_reader_options options = sound_reader_options::none);
    ogg_reader(std::span<const std::uint8_t> data, sound_reader_options options = sound_reader_options::none);
    ogg_reader(std::istream& stream, sound_reader_options options = sound_reader_options::none);

    ~ogg_reader() = default;
    ogg_reader(const ogg_reader&) = delete;
    ogg_reader& operator=(const ogg_reader&) = delete;
    ogg_reader(ogg_reader&& other) noexcept = default;
    ogg_reader& operator=(ogg_reader&& other) noexcept = default;

    bool read(float* output, std::size_t frame_count) override;
    void seek(std::uint64_t frame) override;
    std::uint64_t tell() override;

private:
    void init_from_memory();
    void init_from_stream();

    void fill_info();
    void fill_buffer();
    void close();

    std::size_t sample_size(std::size_t frame_count);

    bool read_samples_from_buffer(float* output, std::size_t frame_count);
    bool read_samples_from_vorbis(float* output, std::size_t frame_count);

private:
    sound_reader_options m_options{};
    std::unique_ptr<OggVorbis_File, vorbis_deleter> m_vorbis{};
    int m_current_section{};
    std::uint64_t m_current_frame{};

    std::vector<std::uint8_t> m_source_buffer{};
    std::unique_ptr<std::ifstream> m_file{};

    std::vector<float> m_decoded_buffer{};
    std::unique_ptr<impl::ogg_memory_stream> m_source{};
    std::istream* m_stream{};
};

}

#endif
