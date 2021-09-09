#ifndef SWELL_FLAC_HPP_INCLUDED
#define SWELL_FLAC_HPP_INCLUDED

#include "config.hpp"

#include <fstream>
#include <filesystem>
#include <span>

#include "sound_reader.hpp"

namespace swl
{

namespace impl
{

struct flac_memory_stream
{
    std::span<const std::uint8_t> data{};
    std::size_t position{};
};

struct flac_context
{
    std::vector<std::uint8_t> source_buffer{};
    std::ifstream file{};

    flac_memory_stream source{};
    std::istream* stream{};

    sound_info info{};
    std::vector<float> buffer{};
    std::size_t buffer_index{};
    std::optional<std::int32_t> error{};
};

}

class SWELL_API flac_reader : public sound_reader
{
    struct flac_deleter
    {
        void operator()(void* file);
    };

public:
    flac_reader() = default;
    flac_reader(const std::filesystem::path& file, sound_reader_options options = sound_reader_options::none);
    flac_reader(std::span<const std::uint8_t> data, sound_reader_options options = sound_reader_options::none);
    flac_reader(std::istream& stream, sound_reader_options options = sound_reader_options::none);

    ~flac_reader() = default;
    flac_reader(const flac_reader&) = delete;
    flac_reader& operator=(const flac_reader&) = delete;
    flac_reader(flac_reader&& other) noexcept = default;
    flac_reader& operator=(flac_reader&& other) noexcept = default;

    bool read(float* output, std::size_t frame_count) override;
    void seek(std::uint64_t frame) override;
    std::uint64_t tell() override;

private:
    void init_from_memory();
    void init_from_stream();
    void common_init();

    void fill_buffer();
    void close();

    std::size_t sample_size(std::size_t frame_count);

    bool read_samples_from_buffer(float* output, std::size_t frame_count);
    bool read_samples_from_flac(float* output, std::size_t frame_count);

private:
    sound_reader_options m_options{};
    std::unique_ptr<void, flac_deleter> m_decoder{};
    std::uint64_t m_current_frame{};
    std::unique_ptr<impl::flac_context> m_context{};
    std::vector<float> m_decoded_buffer{};
};

}

#endif
