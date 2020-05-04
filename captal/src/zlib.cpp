#include "zlib.hpp"

#include <cassert>

#include <zlib/zlib.h>

namespace cpt
{

zlib_compressor::zlib_compressor(std::uint32_t compression_level)
:m_stream{std::make_unique<z_stream_s>()}
{
    assert(compression_level <= 9 && "compression level must be in range [0; 9]");

    if(deflateInit(m_stream.get(), static_cast<int>(compression_level)) != Z_OK)
        throw std::runtime_error{"Can not init deflate stream."};
}

zlib_compressor::~zlib_compressor()
{
    if(m_stream)
    {
        deflateEnd(m_stream.get());
    }
}

std::size_t zlib_compressor::next(const std::uint8_t* input, std::size_t input_size, std::uint8_t* output, std::size_t output_size, bool finish)
{
    m_stream->next_in = reinterpret_cast<const Bytef*>(input);
    m_stream->avail_in = static_cast<uInt>(input_size);
    m_stream->next_out = reinterpret_cast<Bytef*>(output);
    m_stream->avail_out = static_cast<uInt>(output_size);

    if(deflate(m_stream.get(), finish ? Z_FINISH : Z_NO_FLUSH) == Z_STREAM_ERROR)
        return compression_failure;

    return output_size - m_stream->avail_out;
}

}
