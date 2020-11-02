#ifndef CAPTAL_FONT_HPP_INCLUDED
#define CAPTAL_FONT_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <string_view>
#include <filesystem>
#include <istream>
#include <unordered_map>
#include <mutex>

#include <captal_foundation/math.hpp>
#include <captal_foundation/encoding.hpp>

#include "texture.hpp"
#include "signal.hpp"
#include "bin_packing.hpp"

namespace cpt
{

class CAPTAL_API font_engine
{
    struct CAPTAL_API freetype_deleter
    {
        void operator()(void* ptr) noexcept;
    };

    using handle_type = std::unique_ptr<void, freetype_deleter>;

public:
    font_engine();
    ~font_engine() = default;
    font_engine(const font_engine&) = delete;
    font_engine& operator=(const font_engine&) = delete;
    font_engine(font_engine&&) noexcept = delete;
    font_engine& operator=(font_engine&&) noexcept = delete;

    std::mutex& mutex() noexcept
    {
        return m_mutex;
    }

    void* handle() const noexcept
    {
        return m_library.get();
    }

private:
    handle_type m_library{};
    std::mutex m_mutex{};
};

enum class glyph_format : std::uint32_t
{
    gray = 0,
    color = 1
};

struct glyph
{
    vec2f origin{};
    float advance{};
    float ascent{};
    float descent{};
    std::vector<std::uint8_t> data{};
    std::uint32_t width{};
    std::uint32_t height{};
};

using font_atlas_resize_signal = cpt::signal<texture_ptr>;

class CAPTAL_API font_atlas
{
public:
    font_atlas() = default;
    explicit font_atlas(glyph_format format, const tph::sampling_options& sampling = tph::sampling_options{});

    ~font_atlas() = default;
    font_atlas(const font_atlas&) = delete;
    font_atlas& operator=(const font_atlas&) = delete;
    font_atlas(font_atlas&&) noexcept = default;
    font_atlas& operator=(font_atlas&&) noexcept = default;

    std::optional<bin_packer::rect> add_glyph(std::span<const std::uint8_t> image, std::uint32_t width, std::uint32_t height);
    void upload();

    const texture_ptr& texture() const noexcept
    {
        return m_texture;
    }

    font_atlas_resize_signal& signal() noexcept
    {
        return m_signal;
    }

    bool need_upload() const noexcept
    {
        return !std::empty(m_buffers);
    }

private:
    void resize(tph::command_buffer& buffer, asynchronous_resource_keeper& keeper);

private:
    struct transfer_buffer
    {
        std::size_t begin{};
        bin_packer::rect rect{};
    };

private:
    glyph_format m_format{};
    texture_ptr m_texture{};
    tph::sampling_options m_sampling{};
    font_atlas_resize_signal m_signal{};
    bin_packer m_packer{};
    std::vector<transfer_buffer> m_buffers{};
    std::vector<std::uint8_t> m_buffer_data{};
    std::uint32_t m_max_size{};
    bool m_resized{};
};

enum class font_category : std::uint32_t
{
    regular = 0x00,
    italic = 0x01,
    bold = 0x02,
};

enum class font_features : std::uint32_t
{
    scalable = 0x0001,
    fixed_sizes = 0x0002,
    fixed_width = 0x0004,
    horizontal = 0x0010,
    vertical = 0x0020,
    kerning = 0x0040,
    glyph_names = 0x0100,
    hinter = 0x0400,
    color = 0x2000
};

struct font_info
{
    glyph_format format{};
    std::string family{};
    std::uint32_t glyph_count{};
    font_category category{};
    font_features features{};
    std::uint32_t size{};
    std::uint32_t max_glyph_height{};
    std::uint32_t max_glyph_width{};
    std::uint32_t max_ascent{};
    float line_height{};
    float underline_position{};
    float underline_thickness{};
    float strikeout_position{};
};

class CAPTAL_API font
{
    struct CAPTAL_API freetype_deleter
    {
        void operator()(void* ptr) noexcept;
    };

    using handle_type = std::unique_ptr<void, freetype_deleter>;

public:
    font() = default;
    explicit font(const std::filesystem::path& file, std::uint32_t initial_size, glyph_format format = glyph_format::gray);
    explicit font(std::span<const std::uint8_t> data, std::uint32_t initial_size, glyph_format format = glyph_format::gray);
    explicit font(std::istream& stream, std::uint32_t initial_size, glyph_format format = glyph_format::gray);

    ~font() = default;
    font(const font&) = delete;
    font& operator=(const font&) = delete;
    font(font&&) noexcept = default;
    font& operator=(font&&) noexcept = default;

    std::optional<glyph> load(codepoint_t codepoint);
    std::optional<glyph> load_image(codepoint_t codepoint, bool embolden = false);
    bool has(codepoint_t codepoint) const noexcept;

    vec2f kerning(codepoint_t left, codepoint_t right);
    void resize(std::uint32_t pixels_size);

    const font_info& info() const noexcept
    {
        return m_info;
    }

private:
    void init(std::uint32_t initial_size, glyph_format format);

private:
    handle_type m_loader{};
    std::vector<std::uint8_t> m_data{};
    font_info m_info{};
};

}

template<> struct cpt::enable_enum_operations<cpt::font_category> {static constexpr bool value{true};};
template<> struct cpt::enable_enum_operations<cpt::font_features> {static constexpr bool value{true};};

#endif
