#ifndef SWELL_WAVE_HPP_INCLUDED
#define SWELL_WAVE_HPP_INCLUDED

#include "config.hpp"

#include <fstream>
#include <filesystem>
#include <span>
#include <variant>

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
