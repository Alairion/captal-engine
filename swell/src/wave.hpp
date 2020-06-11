#ifndef SWELL_WAVE_HPP_INCLUDED
#define SWELL_WAVE_HPP_INCLUDED

#include "config.hpp"

#include <fstream>
#include <filesystem>
#include <string_view>

#include "mixer.hpp"

namespace swl
{

class wave_reader : public sound_reader
{
    struct header
    {
        std::array<char, 4> file_type_block_id{};
        std::uint32_t file_size{};
        std::array<char, 4> file_format_id{};
        std::array<char, 4> format_block_id{};
        std::uint32_t block_size{};
        std::uint16_t format{};
        std::uint16_t channel_count{};
        std::uint32_t frequency{};
        std::uint32_t byte_per_second{};
        std::uint16_t byte_per_block{};
        std::uint16_t bits_per_sample{};
        std::array<char, 4> data_bloc_id{};
        std::uint32_t data_size{};
    };

public:
    wave_reader() = default;
    wave_reader(const std::filesystem::path& file, sound_reader_options options = sound_reader_options::none);
    wave_reader(const std::string_view& data, sound_reader_options options = sound_reader_options::none);
    wave_reader(std::istream& stream, sound_reader_options options = sound_reader_options::none);

    ~wave_reader() = default;
    wave_reader(const wave_reader&) = delete;
    wave_reader& operator=(const wave_reader&) = delete;
    wave_reader(wave_reader&& other) noexcept = default;
    wave_reader& operator=(wave_reader&& other) noexcept = default;

    bool read(float* output, std::size_t frame_count) override;
    void seek(std::uint64_t frame_offset) override;
    std::uint64_t tell() override;
    std::uint64_t frame_count() override;
    std::uint32_t frequency() override;
    std::uint32_t channel_count() override;

private:
    void read_header(const std::array<char, 44>& data);
    void check_header();

    std::size_t sample_size(std::size_t frame_count);
    std::size_t byte_size(std::size_t frame_count);

    bool read_samples_from_buffer(float* output, std::size_t frame_count);
    bool read_samples_from_memory(float* output, std::size_t frame_count);
    bool read_samples_from_stream(float* output, std::size_t frame_count);

private:
    header m_header{};
    sound_reader_options m_options{};
    std::uint32_t m_current_frame{};
    std::vector<float> m_buffer{};
    std::ifstream m_file{};
    std::string_view m_source{};
    std::istream* m_stream{};
};

}

#endif
