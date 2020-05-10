#include "zlib.hpp"

#include <cassert>

#include <zlib/zlib.h>

namespace cpt
{

namespace impl
{

deflate::deflate(std::uint32_t compression_level, std::int32_t window_bits)
:m_stream{std::make_unique<z_stream_s>()}
{
    assert(compression_level <= 9 && "compression level must be in range [0; 9]");

    if(deflateInit2(m_stream.get(), static_cast<int>(compression_level), Z_DEFLATED, static_cast<int>(window_bits), 8, Z_DEFAULT_STRATEGY) != Z_OK)
        throw std::runtime_error{"Can not init deflate stream."};

    m_valid = true;
}

deflate::~deflate()
{
    if(m_stream)
    {
        deflateEnd(m_stream.get());
    }
}

void deflate::compress_impl(const std::uint8_t*& input, std::size_t input_size, std::uint8_t*& output, std::size_t output_size, bool finish)
{
    assert(valid() && "cpt::impl::deflate::compress called on an invalid stream.");

    m_stream->next_in = reinterpret_cast<const Bytef*>(input);
    m_stream->avail_in = static_cast<uInt>(input_size);
    m_stream->next_out = reinterpret_cast<Bytef*>(output);
    m_stream->avail_out = static_cast<uInt>(output_size);

    const auto result{::deflate(m_stream.get(), finish ? Z_FINISH : Z_NO_FLUSH)};

    if(result == Z_STREAM_ERROR)
        throw std::runtime_error{"Error in deflate stream. " + std::string{zError(Z_STREAM_ERROR)}};

    if(result == Z_STREAM_END)
        m_valid = false;

    input = reinterpret_cast<const std::uint8_t*>(m_stream->next_in);
    output = reinterpret_cast<std::uint8_t*>(m_stream->next_out);
}

std::size_t deflate::compress_bound(std::size_t input_size) const noexcept
{
    return static_cast<std::size_t>(deflateBound(m_stream.get(), static_cast<uLong>(input_size)));
}

void deflate::reset()
{
    assert(m_stream && "cpt::deflate::reset can only be called with constructed streams.");

    if(deflateReset(m_stream.get()) != Z_OK)
        throw std::runtime_error{"Can not init deflate stream."};

     m_valid = true;
}

}

void gzip_deflate::set_header(std::string original_name, std::string comment, std::string extra, std::time_t time)
{
    m_original_name = std::move(original_name);
    m_comment = std::move(comment);
    m_extra = std::move(extra);
    m_time = time;

    gz_header header{};
    header.os = 255;

    if(!std::empty(m_original_name))
    {
        header.name = reinterpret_cast<Bytef*>(std::data(m_original_name));
    }

    if(!std::empty(m_comment))
    {
        header.comment = reinterpret_cast<Bytef*>(std::data(m_comment));
    }

    if(!std::empty(m_comment))
    {
        header.extra = reinterpret_cast<Bytef*>(std::data(m_extra));
        header.extra_len = static_cast<uInt>(std::size(m_extra));
    }

    header.time = static_cast<uLong>(time);

    deflateSetHeader(&get_zstream(), &header);
}

}
