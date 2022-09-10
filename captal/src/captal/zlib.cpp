//MIT License
//
//Copyright (c) 2021 Alexy Pellegrini
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "zlib.hpp"

#include <cassert>

#include <zlib.h>

#include <captal_foundation/encoding.hpp>

namespace cpt
{

namespace impl
{

deflate_base::deflate_base(std::uint32_t compression_level, std::int32_t window_bits)
:m_stream{std::make_unique<z_stream_s>()}
{
    assert(compression_level <= 9 && "cpt::impl::deflate compression level must be in range [0; 9]");

    if(deflateInit2(m_stream.get(), static_cast<int>(compression_level), Z_DEFLATED, static_cast<int>(window_bits), 8, Z_DEFAULT_STRATEGY) != Z_OK)
        throw std::runtime_error{"Can not init deflate stream."};

    m_valid = true;
}

deflate_base::~deflate_base()
{
    deflateEnd(m_stream.get());
}

deflate_base::deflate_base(deflate_base&&) noexcept = default;
deflate_base& deflate_base::operator=(deflate_base&&) noexcept = default;

void deflate_base::compress_impl(const std::uint8_t*& input, std::size_t input_size, std::uint8_t*& output, std::size_t output_size, bool finish)
{
    assert(valid() && "cpt::impl::deflate::compress called on an invalid stream.");

#ifdef ZLIB_CONST
    m_stream->next_in = reinterpret_cast<const Bytef*>(input);
#else
    m_stream->next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(input));
#endif
    m_stream->avail_in = static_cast<uInt>(input_size);
    m_stream->next_out = reinterpret_cast<Bytef*>(output);
    m_stream->avail_out = static_cast<uInt>(output_size);

    const auto result{::deflate(m_stream.get(), finish ? Z_FINISH : Z_NO_FLUSH)};

    if(result == Z_STREAM_ERROR)
    {
        throw std::runtime_error{"Error in deflate stream. " + std::string{zError(Z_STREAM_ERROR)}};
    }
    else if(result == Z_STREAM_END)
    {
        m_valid = false;
    }

    input = reinterpret_cast<const std::uint8_t*>(m_stream->next_in);
    output = reinterpret_cast<std::uint8_t*>(m_stream->next_out);
}

std::size_t deflate_base::compress_bound(std::size_t input_size) const noexcept
{
    return static_cast<std::size_t>(deflateBound(m_stream.get(), static_cast<uLong>(input_size)));
}

void deflate_base::reset()
{
    if(deflateReset(m_stream.get()) != Z_OK)
        throw std::runtime_error{"Can not reset deflate stream."};

    m_valid = true;
}

inflate_base::inflate_base(std::int32_t window_bits)
:m_stream{std::make_unique<z_stream_s>()}
{
    if(inflateInit2(m_stream.get(), static_cast<int>(window_bits)) != Z_OK)
        throw std::runtime_error{"Can not init inflate stream."};

    m_valid = true;
}

inflate_base::~inflate_base()
{
    inflateEnd(m_stream.get());
}

inflate_base::inflate_base(inflate_base&&) noexcept = default;
inflate_base& inflate_base::operator=(inflate_base&&) noexcept = default;

void inflate_base::decompress_impl(const std::uint8_t*& input, std::size_t input_size, std::uint8_t*& output, std::size_t output_size, bool flush)
{
    assert(valid() && "cpt::impl::inflate::compress called on an invalid stream.");

#ifdef ZLIB_CONST
    m_stream->next_in = reinterpret_cast<const Bytef*>(input);
#else
    m_stream->next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(input));
#endif
    m_stream->avail_in = static_cast<uInt>(input_size);
    m_stream->next_out = reinterpret_cast<Bytef*>(output);
    m_stream->avail_out = static_cast<uInt>(output_size);

    const auto result{::inflate(m_stream.get(), flush ? Z_FINISH : Z_NO_FLUSH)};

    if(result == Z_STREAM_ERROR || result == Z_MEM_ERROR)
    {
        throw std::runtime_error{"Error in inflate stream. " + std::string{zError(Z_STREAM_ERROR)}};
    }
    else if(result == Z_STREAM_END || result == Z_NEED_DICT || result == Z_DATA_ERROR)
    {
        m_valid = false;
    }

    input = reinterpret_cast<const std::uint8_t*>(m_stream->next_in);
    output = reinterpret_cast<std::uint8_t*>(m_stream->next_out);
}

void inflate_base::reset()
{
    if(inflateReset(m_stream.get()) != Z_OK)
        throw std::runtime_error{"Can not reset inflate stream."};

    m_valid = true;
}

}

deflate::deflate(std::uint32_t compression_level)
:cpt::impl::deflate_base{compression_level, -15}
{

}

zlib_deflate::zlib_deflate(std::uint32_t compression_level)
:cpt::impl::deflate_base{compression_level, 15}
{

}

gzip_deflate::gzip_deflate(std::uint32_t compression_level)
:impl::deflate_base{compression_level, 16 + 15}
{

}

gzip_deflate::~gzip_deflate() = default;
gzip_deflate::gzip_deflate(gzip_deflate&&) noexcept = default;
gzip_deflate& gzip_deflate::operator=(gzip_deflate&&) noexcept = default;

void gzip_deflate::set_header(std::string_view name, std::string_view comment, std::string extra, std::time_t time)
{
    assert(valid() && "cpt::gzip_deflate::set_header called on an invalid stream.");

    m_name = convert<utf8, latin_1>(name);
    m_comment = convert<utf8, latin_1>(comment);
    m_extra = std::move(extra);
    m_time = time;
    m_header = std::make_unique<gz_header>();

    m_header->os = 255;

    if(!std::empty(m_name))
    {
        m_header->name = reinterpret_cast<Bytef*>(std::data(m_name));
    }

    if(!std::empty(m_comment))
    {
        m_header->comment = reinterpret_cast<Bytef*>(std::data(m_comment));
    }

    if(!std::empty(m_comment))
    {
        m_header->extra = reinterpret_cast<Bytef*>(std::data(m_extra));
        m_header->extra_len = static_cast<uInt>(std::size(m_extra));
    }

    m_header->time = static_cast<uLong>(time);

    deflateSetHeader(&get_zstream(), m_header.get());
}

inflate::inflate()
:impl::inflate_base{-15}
{

}

zlib_inflate::zlib_inflate()
:impl::inflate_base{15}
{

}

struct gzip_inflate::gzip_info
{
    std::array<std::uint8_t, 64 * 1024> extra{};
    std::array<char, 256> name{};
    std::array<char, 4 * 1024> comment{};
    gz_header header{};
};

gzip_inflate::gzip_inflate()
:impl::inflate_base{16 + 15}
{

}

gzip_inflate::~gzip_inflate() = default;
gzip_inflate::gzip_inflate(gzip_inflate&&) noexcept = default;
gzip_inflate& gzip_inflate::operator=(gzip_inflate&&) noexcept = default;

void gzip_inflate::grab_header()
{
    if(!m_header)
    {
        m_header = std::make_unique<gzip_info>();
    }
    else
    {
        m_header->header = gz_header{};
    }

    m_header->header.name      = reinterpret_cast<Bytef*>(std::data(m_header->name));
    m_header->header.name_max  = static_cast<uInt>(std::size(m_header->name) - 1);
    m_header->header.comment   = reinterpret_cast<Bytef*>(std::data(m_header->comment));
    m_header->header.comm_max  = static_cast<uInt>(std::size(m_header->comment) - 1);
    m_header->header.extra     = reinterpret_cast<Bytef*>(std::data(m_header->extra));
    m_header->header.extra_max = static_cast<uInt>(std::size(m_header->extra));

    if(inflateGetHeader(&get_zstream(), &m_header->header) != Z_OK)
        throw std::runtime_error{"Can not grab gzip header."};
}

bool gzip_inflate::is_header_ready() const noexcept
{
    if(m_header)
    {
        return m_header->header.done == 1;
    }

    return false;
}

std::string gzip_inflate::name() const
{
    assert(is_header_ready() && "cpt::gzip_inflate::is_header_ready returned false in cpt::gzip_inflate::original_name");

    return convert<latin_1, narrow>(std::string_view{std::data(m_header->name)});
}

std::string gzip_inflate::comment() const
{
    assert(is_header_ready() && "cpt::gzip_inflate::is_header_ready returned false in cpt::gzip_inflate::comment");

    return convert<latin_1, narrow>(std::string_view{std::data(m_header->comment)});
}

std::span<const std::uint8_t> gzip_inflate::extra() const noexcept
{
    assert(is_header_ready() && "cpt::gzip_inflate::is_header_ready returned false in cpt::gzip_inflate::extra");

    return std::span<const std::uint8_t>{std::data(m_header->extra), static_cast<std::size_t>(m_header->header.extra_len)};
}

std::chrono::system_clock::time_point gzip_inflate::time() const noexcept
{
    assert(is_header_ready() && "cpt::gzip_inflate::is_header_ready returned false in cpt::gzip_inflate::time");

    return std::chrono::system_clock::from_time_t(static_cast<std::time_t>(m_header->header.time));
}

}
