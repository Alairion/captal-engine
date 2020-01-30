#ifndef SWELL_SOUND_FILE_HPP_INCLUDED
#define SWELL_SOUND_FILE_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <string_view>

#include "mixer.hpp"

namespace swl
{

class sound_file_reader : public sound_reader
{
public:
    constexpr sound_file_reader() = default;
    sound_file_reader(std::string_view file, load_from_file_t, sound_reader_options options = sound_reader_options::none);
    sound_file_reader(std::string_view data, load_from_memory_t, sound_reader_options options = sound_reader_options::none);

    ~sound_file_reader() = default;
    sound_file_reader(const sound_file_reader&) = delete;
    sound_file_reader& operator=(const sound_file_reader&) = delete;
    sound_file_reader(sound_file_reader&& other) noexcept = default;
    sound_file_reader& operator=(sound_file_reader&& other) noexcept = default;

protected:
    virtual bool read_samples(float* output, std::size_t frame_count)
    {
        return m_reader->read(output, frame_count);
    }

    virtual void seek_samples(std::uint64_t frame_offset)
    {
        m_reader->seek(frame_offset);
    }

    virtual std::uint64_t get_frame_count()
    {
        return m_reader->frame_count();
    }

    virtual std::uint32_t get_frequency()
    {
        return m_reader->frequency();
    }

    virtual std::uint32_t get_channels()
    {
        return m_reader->channels_count();
    }

private:
    std::unique_ptr<sound_reader> m_reader{};
};

}

#endif
