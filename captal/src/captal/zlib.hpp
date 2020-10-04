#ifndef CAPTAL_ZLIB_HPP_INCLUDED
#define CAPTAL_ZLIB_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <array>
#include <iterator>
#include <concepts>
#include <chrono>
#include <algorithm>
#include <span>

struct z_stream_s;
struct gz_header_s;

namespace cpt
{

namespace impl
{

class CAPTAL_API deflate_base
{
public:
    static constexpr bool flush{true};
    static constexpr bool known_compress_bound{true};

public:
    deflate_base(std::uint32_t compression_level, std::int32_t window_bits);

    ~deflate_base();
    deflate_base(const deflate_base&) = delete;
    deflate_base& operator=(const deflate_base&) = delete;
    deflate_base(deflate_base&&) noexcept = default;
    deflate_base& operator=(deflate_base&&) noexcept = default;

    template<std::contiguous_iterator InContiguousIt, std::contiguous_iterator OutContiguousIt>
    bool compress(InContiguousIt& input_begin, InContiguousIt input_end, OutContiguousIt& output_begin, OutContiguousIt output_end, bool flush = false)
    {
        static_assert(sizeof(typename std::iterator_traits<InContiguousIt>::value_type) == 1, "cpt::deflate_base::compress only works on bytes.");
        static_assert(sizeof(typename std::iterator_traits<OutContiguousIt>::value_type) == 1, "cpt::deflate_base::compress only works on bytes.");

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

    template<std::size_t BufferSize = 2048, std::input_iterator InputIt, std::output_iterator<std::uint8_t> OutputIt>
    bool compress_buffered(InputIt begin, InputIt end, OutputIt& output, bool flush = false)
    {
        std::array<std::uint8_t, BufferSize> input_buffer{};
        std::array<std::uint8_t, BufferSize> output_buffer{};

        while(begin != end)
        {
            std::size_t count{};
            while(count < std::size(input_buffer) && begin != end)
            {
                input_buffer[count++] = static_cast<std::uint8_t>(*begin++);
            }

            auto input_buffer_begin{std::begin(input_buffer)};
            const auto input_buffer_end{std::begin(input_buffer) + count};

            auto output_buffer_begin{std::end(output_buffer)}; //i hate do-whiles :)
            const auto output_buffer_end{std::end(output_buffer)};

            while(output_buffer_begin == output_buffer_end)
            {
                output_buffer_begin = std::begin(output_buffer);

                compress(input_buffer_begin, input_buffer_end, output_buffer_begin, output_buffer_end, flush && !(begin != end));
                output = std::copy(std::begin(output_buffer), output_buffer_begin, output);

                if(!m_valid)
                {
                    return false;
                }
            }
        }

        return m_valid;
    }

    std::size_t compress_bound(std::size_t input_size) const noexcept;

    template<std::input_iterator InputIt>
    std::size_t compress_bound(InputIt begin, InputIt end) const
    {
        static_assert(sizeof(typename std::iterator_traits<InputIt>::value_type) == 1, "cpt::deflate_base::compress_bound only works on bytes.");

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

class CAPTAL_API inflate_base
{
public:
    static constexpr bool flush{true};

public:
    explicit inflate_base(std::int32_t window_bits);

    ~inflate_base();
    inflate_base(const inflate_base&) = delete;
    inflate_base& operator=(const inflate_base&) = delete;
    inflate_base(inflate_base&&) noexcept = default;
    inflate_base& operator=(inflate_base&&) noexcept = default;

    template<std::contiguous_iterator InContiguousIt, std::contiguous_iterator OutContiguousIt>
    bool decompress(InContiguousIt& input_begin, InContiguousIt input_end, OutContiguousIt& output_begin, OutContiguousIt output_end, bool flush)
    {
        static_assert(sizeof(typename std::iterator_traits<InContiguousIt>::value_type) == 1, "cpt::inflate_base::decompress only works on bytes.");
        static_assert(sizeof(typename std::iterator_traits<OutContiguousIt>::value_type) == 1, "cpt::inflate_base::decompress only works on bytes.");

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

    template<std::size_t BufferSize = 2048, std::input_iterator InputIt, std::output_iterator<std::uint8_t> OutputIt>
    bool decompress_buffered(InputIt begin, InputIt end, OutputIt& output, bool flush = false)
    {
        std::array<std::uint8_t, BufferSize> input_buffer{};
        std::array<std::uint8_t, BufferSize> output_buffer{};

        while(begin != end)
        {
            std::size_t count{};
            while(count < std::size(input_buffer) && begin != end)
            {
                input_buffer[count++] = static_cast<std::uint8_t>(*begin++);
            }

            auto input_buffer_begin{std::begin(input_buffer)};
            const auto input_buffer_end{std::begin(input_buffer) + count};

            auto output_buffer_begin{std::end(output_buffer)}; //i hate do-whiles :)
            const auto output_buffer_end{std::end(output_buffer)};

            while(output_buffer_begin == output_buffer_end)
            {
                output_buffer_begin = std::begin(output_buffer);

                decompress(input_buffer_begin, input_buffer_end, output_buffer_begin, output_buffer_end, flush && !(begin != end));
                output = std::copy(std::begin(output_buffer), output_buffer_begin, output);

                if(!m_valid)
                {
                    return false;
                }
            }
        }

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
    void decompress_impl(const std::uint8_t*& input, std::size_t input_size, std::uint8_t*& output, std::size_t output_size, bool flush);

private:
    std::unique_ptr<z_stream_s> m_stream{};
    bool m_valid{};
};

}

class CAPTAL_API deflate : public impl::deflate_base
{
public:
    explicit deflate(std::uint32_t compression_level = 6);
    ~deflate() = default;
    deflate(const deflate&) = delete;
    deflate& operator=(const deflate&) = delete;
    deflate(deflate&&) noexcept = default;
    deflate& operator=(deflate&&) noexcept = default;
};

class CAPTAL_API zlib_deflate : public impl::deflate_base
{
public:
    explicit zlib_deflate(std::uint32_t compression_level = 6);
    ~zlib_deflate() = default;
    zlib_deflate(const zlib_deflate&) = delete;
    zlib_deflate& operator=(const zlib_deflate&) = delete;
    zlib_deflate(zlib_deflate&&) noexcept = default;
    zlib_deflate& operator=(zlib_deflate&&) noexcept = default;
};

class CAPTAL_API gzip_deflate : public impl::deflate_base
{
public:
    explicit gzip_deflate(std::uint32_t compression_level = 6);
    ~gzip_deflate() = default;
    gzip_deflate(const gzip_deflate&) = delete;
    gzip_deflate& operator=(const gzip_deflate&) = delete;
    gzip_deflate(gzip_deflate&&) noexcept = default;
    gzip_deflate& operator=(gzip_deflate&&) noexcept = default;

    void set_header(std::string_view name, std::string_view comment, std::string extra = std::string{}, std::time_t time = std::time(nullptr));

private:
    std::string m_name{};
    std::string m_comment{};
    std::string m_extra{};
    std::time_t m_time{};
    std::unique_ptr<gz_header_s> m_header{};
};

class CAPTAL_API inflate : public impl::inflate_base
{
public:
    explicit inflate();
    ~inflate() = default;
    inflate(const inflate&) = delete;
    inflate& operator=(const inflate&) = delete;
    inflate(inflate&&) noexcept = default;
    inflate& operator=(inflate&&) noexcept = default;
};

class CAPTAL_API zlib_inflate : public impl::inflate_base
{
public:
    explicit zlib_inflate();
    ~zlib_inflate() = default;
    zlib_inflate(const zlib_inflate&) = delete;
    zlib_inflate& operator=(const zlib_inflate&) = delete;
    zlib_inflate(zlib_inflate&&) noexcept = default;
    zlib_inflate& operator=(zlib_inflate&&) noexcept = default;
};

class CAPTAL_API gzip_inflate : public impl::inflate_base
{
    struct gzip_info;

public:
    explicit gzip_inflate();
    ~gzip_inflate() = default;
    gzip_inflate(const gzip_inflate&) = delete;
    gzip_inflate& operator=(const gzip_inflate&) = delete;
    gzip_inflate(gzip_inflate&&) noexcept = default;
    gzip_inflate& operator=(gzip_inflate&&) noexcept = default;

    void grab_header();
    bool is_header_ready() const noexcept;

    std::string name() const;
    std::string comment() const;
    std::span<const std::uint8_t> extra() const noexcept;
    std::chrono::system_clock::time_point time() const noexcept;

private:
    std::unique_ptr<gzip_info> m_header{};
};

template<typename Compressor, std::contiguous_iterator InContiguousIt, std::contiguous_iterator OutContiguousIt, typename... Args>
std::pair<OutContiguousIt, bool> compress(InContiguousIt input_begin, InContiguousIt input_end, OutContiguousIt output_begin, OutContiguousIt output_end, Args&&... args)
{
    Compressor compressor{std::forward<Args>(args)...};
    compressor.compress(input_begin, input_end, output_begin, output_end, true);

    return std::make_pair(output_begin, !compressor.valid());
}

template<typename Decompressor, std::contiguous_iterator InContiguousIt, std::contiguous_iterator OutContiguousIt, typename... Args>
std::pair<OutContiguousIt, bool> decompress(InContiguousIt input_begin, InContiguousIt input_end, OutContiguousIt output_begin, OutContiguousIt output_end, Args&&... args)
{
    Decompressor decompressor{std::forward<Args>(args)...};
    decompressor.decompress(input_begin, input_end, output_begin, output_end, true);

    return std::make_pair(output_begin, !decompressor.valid());
}

template<typename Compressor, std::size_t BufferSize = 2048, std::input_iterator InputIt, std::output_iterator<std::uint8_t> OutputIt, typename... Args>
std::pair<OutputIt, bool> compress_buffered(InputIt begin, InputIt end, OutputIt output, Args&&... args)
{
    Compressor compressor{std::forward<Args>(args)...};
    compressor. template compress_buffered<BufferSize>(begin, end, output, true);

    return std::make_pair(output, !compressor.valid());
}

template<typename Decompressor, std::size_t BufferSize = 2048, std::input_iterator InputIt, std::output_iterator<std::uint8_t> OutputIt, typename... Args>
std::pair<OutputIt, bool> decompress_buffered(InputIt begin, InputIt end, OutputIt output, Args&&... args)
{
    Decompressor decompressor{std::forward<Args>(args)...};
    decompressor. template decompress_buffered<BufferSize>(begin, end, output, true);

    return std::make_pair(output, !decompressor.valid());
}

}

#endif
