#ifndef SWELL_SOUND_READER_HPP_INCLUDED
#define SWELL_SOUND_READER_HPP_INCLUDED

#include "config.hpp"

#include <limits>

namespace swl
{

enum class sound_reader_options : std::uint32_t
{
    none = 0x00,
    buffered = 0x01
};

struct sound_info
{
    std::uint64_t frame_count{};
    std::uint32_t frequency{};
    std::uint32_t channel_count{};
    bool seekable{};
};

class SWELL_API sound_reader
{
public:
    virtual ~sound_reader() = default;

    virtual bool read(float* output [[maybe_unused]], std::size_t frame_count [[maybe_unused]])
    {
        return false;
    }

    virtual void seek(std::uint64_t frame [[maybe_unused]])
    {

    }

    virtual std::uint64_t tell()
    {
        return 0;
    }

    const sound_info& info() const noexcept
    {
        return m_info;
    }

protected:
    sound_reader() = default;
    sound_reader(const sound_reader&) = delete;
    sound_reader& operator=(const sound_reader&) = delete;
    sound_reader(sound_reader&& other) noexcept = default;
    sound_reader& operator=(sound_reader&& other) noexcept = default;

    void set_info(const sound_info& info) noexcept
    {
        m_info = info;
    }

private:
    sound_info m_info{};
};

}

template<> struct swl::enable_enum_operations<swl::sound_reader_options> {static constexpr bool value{true};};

#endif
