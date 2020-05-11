#ifndef CAPTAL_ZLIB_HPP_INCLUDED
#define CAPTAL_ZLIB_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <array>
#include <iterator>
#include <ctime>

struct z_stream_s;
struct gz_header_s;

namespace cpt
{

namespace impl
{

class deflate
{
public:
    static constexpr bool flush{true};
    static constexpr bool known_compress_bound{true};

public:
    deflate(std::uint32_t compression_level, std::int32_t window_bits);
    ~deflate();
    deflate(const deflate&) = delete;
    deflate& operator=(const deflate&) = delete;
    deflate(deflate&&) noexcept = default;
    deflate& operator=(deflate&&) noexcept = default;

    template<typename InContiguousIt, typename OutContiguousIt>
    bool compress(InContiguousIt& input_begin, InContiguousIt input_end, OutContiguousIt& output_begin, OutContiguousIt output_end, bool flush = false)
    {
        static_assert(sizeof(typename std::iterator_traits<InContiguousIt>::value_type) == 1, "cpt::deflate::compress only works on bytes.");
        static_assert(sizeof(typename std::iterator_traits<OutContiguousIt>::value_type) == 1, "cpt::deflate::compress only works on bytes.");

        const std::uint8_t* const input_address{reinterpret_cast<const std::uint8_t*>(std::addressof(*input_begin))};
        const std::uint8_t* input_ptr{input_address};
        const std::size_t input_size{static_cast<std::size_t>(std::distance(input_begin, input_end))};

        std::uint8_t* const output_address{reinterpret_cast<std::uint8_t*>(std::addressof(*output_begin))};
        std::uint8_t* output_ptr{output_address};
        const std::size_t output_size{static_cast<std::size_t>(std::distance(output_begin, output_end))};

        compress_impl(input_ptr, input_size, output_ptr, output_size, flush);

        std::advance(input_begin, input_ptr - input_address);
        std::advance(output_begin, output_ptr - output_address);

        return m_valid;
    }

    std::size_t compress_bound(std::size_t input_size) const noexcept;

    template<typename InputIt>
    std::size_t compress_bound(InputIt begin, InputIt end) const
    {
        static_assert(sizeof(typename std::iterator_traits<InputIt>::value_type) == 1, "cpt::deflate::compress_bound only works on bytes.");

        return compress_bound(static_cast<std::size_t>(std::distance(begin, end)));
    }

    void reset();

    bool valid() const noexcept
    {
        return m_valid;
    }

protected:
    z_stream_s& get_zstream() const noexcept
    {
        return *m_stream;
    }

private:
    void compress_impl(const std::uint8_t*& input, std::size_t input_size, std::uint8_t*& output, std::size_t output_size, bool finish);

private:
    std::unique_ptr<z_stream_s> m_stream{};
    bool m_valid{};
};

class inflate
{
public:
    static constexpr bool known_compress_bound{true};

public:
    inflate(std::int32_t window_bits);
    ~inflate();
    inflate(const inflate&) = delete;
    inflate& operator=(const inflate&) = delete;
    inflate(inflate&&) noexcept = default;
    inflate& operator=(inflate&&) noexcept = default;

    template<typename InContiguousIt, typename OutContiguousIt>
    bool decompress(InContiguousIt& input_begin, InContiguousIt input_end, OutContiguousIt& output_begin, OutContiguousIt output_end, bool flush = false)
    {
        static_assert(sizeof(typename std::iterator_traits<InContiguousIt>::value_type) == 1, "cpt::inflate::decompress only works on bytes.");
        static_assert(sizeof(typename std::iterator_traits<OutContiguousIt>::value_type) == 1, "cpt::inflate::decompress only works on bytes.");

        const std::uint8_t* const input_address{reinterpret_cast<const std::uint8_t*>(std::addressof(*input_begin))};
        const std::uint8_t* input_ptr{input_address};
        const std::size_t input_size{static_cast<std::size_t>(std::distance(input_begin, input_end))};

        std::uint8_t* const output_address{reinterpret_cast<std::uint8_t*>(std::addressof(*output_begin))};
        std::uint8_t* output_ptr{output_address};
        const std::size_t output_size{static_cast<std::size_t>(std::distance(output_begin, output_end))};

        decompress_impl(input_ptr, input_size, output_ptr, output_size, flush);

        std::advance(input_begin, input_ptr - input_address);
        std::advance(output_begin, output_ptr - output_address);

        return m_valid;
    }

    void reset();

    bool valid() const noexcept
    {
        return m_valid;
    }

protected:
    z_stream_s& get_zstream() const noexcept
    {
        return *m_stream;
    }

private:
    void decompress_impl(const std::uint8_t*& input, std::size_t input_size, std::uint8_t*& output, std::size_t output_size, bool finish);

private:
    std::unique_ptr<z_stream_s> m_stream{};
    bool m_valid{};
};

}

class deflate : public impl::deflate
{
public:
    explicit deflate(std::uint32_t compression_level = 6)
    :impl::deflate{compression_level, -15}
    {

    }

    ~deflate() = default;
    deflate(const deflate&) = delete;
    deflate& operator=(const deflate&) = delete;
    deflate(deflate&&) noexcept = default;
    deflate& operator=(deflate&&) noexcept = default;
};

class zlib_deflate : public impl::deflate
{
public:
    explicit zlib_deflate(std::uint32_t compression_level = 6)
    :impl::deflate{compression_level, 15}
    {

    }

    ~zlib_deflate() = default;
    zlib_deflate(const zlib_deflate&) = delete;
    zlib_deflate& operator=(const zlib_deflate&) = delete;
    zlib_deflate(zlib_deflate&&) noexcept = default;
    zlib_deflate& operator=(zlib_deflate&&) noexcept = default;
};

class gzip_deflate : public impl::deflate
{
public:
    explicit gzip_deflate(std::uint32_t compression_level = 6)
    :impl::deflate{compression_level, 16 + 15}
    {

    }

    ~gzip_deflate() = default;
    gzip_deflate(const gzip_deflate&) = delete;
    gzip_deflate& operator=(const gzip_deflate&) = delete;
    gzip_deflate(gzip_deflate&&) noexcept = default;
    gzip_deflate& operator=(gzip_deflate&&) noexcept = default;

    void set_header(const std::string& name, const std::string& comment, std::string extra = std::string{}, std::time_t time = std::time(nullptr));

private:
    std::string m_name{};
    std::string m_comment{};
    std::string m_extra{};
    std::time_t m_time{};
    std::unique_ptr<gz_header_s> m_header{};
};

class inflate : public impl::inflate
{
public:
    inflate()
    :impl::inflate{-15}
    {

    }

    ~inflate() = default;
    inflate(const inflate&) = delete;
    inflate& operator=(const inflate&) = delete;
    inflate(inflate&&) noexcept = default;
    inflate& operator=(inflate&&) noexcept = default;
};

class zlib_inflate : public impl::inflate
{
public:
    zlib_inflate()
    :impl::inflate{15}
    {

    }

    ~zlib_inflate() = default;
    zlib_inflate(const zlib_inflate&) = delete;
    zlib_inflate& operator=(const zlib_inflate&) = delete;
    zlib_inflate(zlib_inflate&&) noexcept = default;
    zlib_inflate& operator=(zlib_inflate&&) noexcept = default;
};

class gzip_inflate : public impl::inflate
{
    struct gzip_info;

public:
    gzip_inflate();
    ~gzip_inflate() = default;
    gzip_inflate(const gzip_inflate&) = delete;
    gzip_inflate& operator=(const gzip_inflate&) = delete;
    gzip_inflate(gzip_inflate&&) noexcept = default;
    gzip_inflate& operator=(gzip_inflate&&) noexcept = default;

    void reset();
    void grab_header();

    std::string name() const;
    std::string comment() const;
    std::string_view extra() const noexcept;
    std::time_t time() const noexcept;
    bool is_header_ready() const noexcept;

private:
    std::unique_ptr<gzip_info> m_header{};
};

}

#endif
