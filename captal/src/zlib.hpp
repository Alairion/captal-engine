#ifndef CAPTAL_ZLIB_HPP_INCLUDED
#define CAPTAL_ZLIB_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <array>
#include <iterator>

struct z_stream_s;

namespace cpt
{

struct zlib_compressor
{
private:
    static constexpr std::size_t compression_failure{std::numeric_limits<std::size_t>::max()};
    static constexpr std::size_t buffer_size{4096};

public:
    zlib_compressor(std::uint32_t compression_level = 6);
    ~zlib_compressor();
    zlib_compressor(const zlib_compressor&) = delete;
    zlib_compressor& operator=(const zlib_compressor&) = delete;
    zlib_compressor(zlib_compressor&&) noexcept = default;
    zlib_compressor& operator=(zlib_compressor&&) noexcept = default;

    template<typename InputIt, typename OutputIt>
    OutputIt compress(InputIt input_begin, InputIt input_end, OutputIt output_begin, OutputIt output_end)
    {
        const std::size_t input_size{static_cast<std::size_t>(std::distance(input_begin, input_end))};
        const std::size_t output_size{static_cast<std::size_t>(std::distance(output_begin, output_end))};

        std::array<std::uint8_t, buffer_size> input_buffer{};
        std::copy(input_begin, input_end, std::begin(input_buffer));

        std::array<std::uint8_t, buffer_size> output_buffer{};
        std::copy(output_begin, output_end, std::begin(output_buffer));


    }

private:
    std::size_t next(const std::uint8_t* input, std::size_t input_size, std::uint8_t* output, std::size_t output_size, bool finish);

private:
    std::unique_ptr<z_stream_s> m_stream{};
};

std::size_t compress_bound(std::size_t input_size) noexcept;

}

#endif
