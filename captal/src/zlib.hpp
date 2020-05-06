#ifndef CAPTAL_ZLIB_HPP_INCLUDED
#define CAPTAL_ZLIB_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <array>
#include <iterator>

struct z_stream_s;

namespace cpt
{

class zlib_compressor
{
public:
    static constexpr bool flush{true};

public:
    zlib_compressor(std::uint32_t compression_level = 6);
    ~zlib_compressor();
    zlib_compressor(const zlib_compressor&) = delete;
    zlib_compressor& operator=(const zlib_compressor&) = delete;
    zlib_compressor(zlib_compressor&&) noexcept = default;
    zlib_compressor& operator=(zlib_compressor&&) noexcept = default;

    template<typename InContiguousIt, typename OutContiguousIt>
    void compress(InContiguousIt& input_begin, InContiguousIt input_end, OutContiguousIt& output_begin, OutContiguousIt output_end, bool flush = false)
    {
        static_assert(sizeof(typename std::iterator_traits<InContiguousIt>::value_type) == 1, "cpt::zlib_compressor::compress only works on bytes.");
        static_assert(sizeof(typename std::iterator_traits<OutContiguousIt>::value_type) == 1, "cpt::zlib_compressor::compress only works on bytes.");

        const std::uint8_t* const input_address{reinterpret_cast<const std::uint8_t*>(std::addressof(*input_begin))};
        const std::uint8_t* input_ptr{input_address};
        const std::size_t input_size{static_cast<std::size_t>(std::distance(input_begin, input_end))};

        std::uint8_t* const output_address{reinterpret_cast<std::uint8_t*>(std::addressof(*output_begin))};
        std::uint8_t* output_ptr{output_address};
        const std::size_t output_size{static_cast<std::size_t>(std::distance(output_begin, output_end))};

        compress_impl(input_ptr, input_size, output_ptr, output_size, flush);

        std::advance(input_begin, input_ptr - input_address);
        std::advance(output_begin, output_ptr - output_address);
    }

    std::size_t compress_bound(std::size_t input_size) const noexcept;

    template<typename InputIt>
    std::size_t compress_bound(InputIt begin, InputIt end) const
    {
        return compress_bound(static_cast<std::size_t>(std::distance(begin, end) * sizeof(typename std::iterator_traits<InputIt>::value_type)));
    }

private:
    void compress_impl(const std::uint8_t*& input, std::size_t input_size, std::uint8_t*& output, std::size_t output_size, bool finish);

private:
    std::unique_ptr<z_stream_s> m_stream{};
};



}

#endif
