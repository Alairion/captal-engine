#ifndef SWELL_SOUND_FILE_HPP_INCLUDED
#define SWELL_SOUND_FILE_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <filesystem>
#include <string_view>
#include <istream>

#include "mixer.hpp"

namespace swl
{

class SWELL_API sound_file_reader : public sound_reader
{
public:
    constexpr sound_file_reader() = default;

    sound_file_reader(const std::filesystem::path& file, sound_reader_options options = sound_reader_options::none);
    sound_file_reader(std::string_view data, sound_reader_options options = sound_reader_options::none);
    sound_file_reader(std::istream& stream, sound_reader_options options = sound_reader_options::none);
    sound_file_reader(std::unique_ptr<sound_reader> m_reader) noexcept;

    virtual ~sound_file_reader() = default;
    sound_file_reader(const sound_file_reader&) = delete;
    sound_file_reader& operator=(const sound_file_reader&) = delete;
    sound_file_reader(sound_file_reader&& other) noexcept = default;
    sound_file_reader& operator=(sound_file_reader&& other) noexcept = default;

    bool read(float* output, std::size_t frame_count) override
    {
        return m_reader->read(output, frame_count);
    }

    void seek(std::uint64_t frame_offset) override
    {
        m_reader->seek(frame_offset);
    }

    std::uint64_t tell() override
    {
        return m_reader->tell();
    }

    std::uint64_t frame_count() override
    {
        return m_reader->frame_count();
    }

    std::uint32_t frequency() override
    {
        return m_reader->frequency();
    }

    std::uint32_t channel_count() override
    {
        return m_reader->channel_count();
    }

private:
    std::unique_ptr<sound_reader> m_reader{};
};

}

#endif
