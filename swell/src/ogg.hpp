#ifndef SWELL_OGG_HPP_INCLUDED
#define SWELL_OGG_HPP_INCLUDED

#include "config.hpp"

#include <fstream>
#include <filesystem>
#include <string_view>

#include "mixer.hpp"

struct OggVorbis_File;

namespace swl
{

struct memory_stream
{
    std::string_view data{};
    std::uint64_t pos{};
};

class ogg_reader : public sound_reader
{
    struct vorbis_deleter
    {
        void operator()(OggVorbis_File* file);
    };

public:
    ogg_reader() = default;
    ogg_reader(const std::filesystem::path& file, sound_reader_options options = sound_reader_options::none);
    ogg_reader(const std::string_view& data, sound_reader_options options = sound_reader_options::none);
    ogg_reader(std::istream& stream, sound_reader_options options = sound_reader_options::none);

    ~ogg_reader();
    ogg_reader(const ogg_reader&) = delete;
    ogg_reader& operator=(const ogg_reader&) = delete;
    ogg_reader(ogg_reader&& other) noexcept = default;
    ogg_reader& operator=(ogg_reader&& other) noexcept = default;

protected:
    bool read_samples(float* output, std::size_t frame_count) override;
    void seek_samples(std::uint64_t frame_offset) override;
    std::uint64_t get_frame_count() override;
    std::uint32_t get_frequency() override;
    std::uint32_t get_channels() override;

    void fill_buffer();
    void close();

    std::size_t sample_size(std::size_t frame_count);

    bool read_samples_from_buffer(float* output, std::size_t frame_count);
    bool read_samples_from_vorbis(float* output, std::size_t frame_count);

private:
    sound_reader_options m_options{};
    std::unique_ptr<OggVorbis_File, vorbis_deleter> m_vorbis{};
    std::uint64_t m_frame_count{};
    std::uint32_t m_frequency{};
    std::uint32_t m_channel_count{};
    int m_current_section{};
    std::uint32_t m_current_frame{};
    std::vector<float> m_buffer{};
    std::ifstream m_file{};
    memory_stream m_source{};
    std::istream* m_stream{};
};

}

#endif
