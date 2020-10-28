#ifndef CAPTAL_TEXT_HPP_INCLUDED
#define CAPTAL_TEXT_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <string_view>
#include <filesystem>
#include <istream>
#include <unordered_map>

#include <captal_foundation/math.hpp>
#include <captal_foundation/encoding.hpp>

#include <tephra/texture.hpp>
#include <tephra/image.hpp>

#include "color.hpp"
#include "renderable.hpp"
#include "bin_packing.hpp"

namespace cpt
{

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
    glyph_format format{};
    std::vector<std::uint8_t> data{};
    std::uint32_t width{};
    std::uint32_t height{};
};

enum class font_style : std::uint32_t
{
    regular = 0x00,
    bold = 0x01,
    underlined = 0x04,
    strikethrough = 0x08,
};

using font_atlas_resize_signal = cpt::signal<>;

class CAPTAL_API font_atlas
{
public:
    font_atlas(glyph_format format);
    ~font_atlas() = default;
    font_atlas(const font_atlas&) = delete;
    font_atlas& operator=(const font_atlas&) = delete;
    font_atlas(font_atlas&&) noexcept = default;
    font_atlas& operator=(font_atlas&&) noexcept = default;

    std::optional<bin_packer::rect> add_glyph(const std::vector<std::uint8_t>& image, std::uint32_t width, std::uint32_t height);
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
    font_atlas_resize_signal m_signal{};
    bin_packer m_packer{};
    std::vector<transfer_buffer> m_buffers{};
    std::vector<std::uint8_t> m_buffer_data{};
    std::uint32_t m_max_size{};
    bool m_resized{};
};

struct font_info
{
    std::string family{};
    std::uint32_t glyph_count{};
    font_style style{};
    std::uint32_t size{};
    std::uint32_t max_glyph_height{};
    std::uint32_t max_glyph_width{};
    std::uint32_t max_ascent{};
    float line_height{};
    float underline_position{};
    float underline_thickness{};
};

class CAPTAL_API font
{
    struct freetype_info;

    struct CAPTAL_API freetype_deleter
    {
        void operator()(freetype_info* ptr) noexcept;
    };

public:
    font() = default;
    explicit font(const std::filesystem::path& file, std::uint32_t initial_size = 16);
    explicit font(std::span<const std::uint8_t> data, std::uint32_t initial_size = 16);
    explicit font(std::istream& stream, std::uint32_t initial_size = 16);

    ~font() = default;
    font(const font&) = delete;
    font& operator=(const font&) = delete;
    font(font&&) noexcept = default;
    font& operator=(font&&) noexcept = default;

    void resize(std::uint32_t pixels_size);

    void set_style(font_style style) noexcept
    {
        m_info.style = style;
    }

    std::optional<glyph> load(codepoint_t codepoint);
    float kerning(codepoint_t left, codepoint_t right);

    const font_info& info() const noexcept
    {
        return m_info;
    }

private:
    void init(std::uint32_t initial_size);

private:
    std::vector<std::uint8_t> m_data{};
    std::unique_ptr<freetype_info, freetype_deleter> m_loader{};
    font_info m_info{};
};

class CAPTAL_API text final : public renderable
{
public:
    text() = default;
    explicit text(std::span<const std::uint32_t> indices, std::span<const vertex> vertices, texture_ptr texture, std::uint32_t width, std::uint32_t height, std::size_t count);

    ~text() = default;
    text(const text&) = delete;
    text& operator=(const text&) = delete;
    text(text&&) noexcept = default;
    text& operator=(text&&) noexcept = default;

    void set_color(const color& color);
    void set_color(std::uint32_t character_index, const color& color);
    void set_color(std::uint32_t first, std::uint32_t count, const color& color);

    std::uint32_t width() const noexcept
    {
        return m_width;
    }

    std::uint32_t height() const noexcept
    {
        return m_height;
    }

private:
    std::uint32_t m_width{};
    std::uint32_t m_height{};
    std::size_t m_count{};
};

enum class text_drawer_options : std::uint32_t
{
    none = 0x00,
    kerning = 0x01
};

template<> struct enable_enum_operations<text_drawer_options> {static constexpr bool value{true};};

enum class text_align : std::uint32_t
{
    left = 0,
    right = 1,
    center = 2,
    justify = 3
};

struct text_bounds
{
    std::uint32_t width{};
    std::uint32_t height{};
};

class CAPTAL_API text_drawer
{
public:
    text_drawer() = default;
    explicit text_drawer(cpt::font font, text_drawer_options options = text_drawer_options::kerning);

    ~text_drawer() = default;
    text_drawer(const text_drawer&) = delete;
    text_drawer& operator=(const text_drawer&) = delete;
    text_drawer(text_drawer&&) noexcept = default;
    text_drawer& operator=(text_drawer&&) noexcept = default;

    void set_font(cpt::font font) noexcept;
    void set_style(font_style style) noexcept;
    void resize(std::uint32_t pixels_size);

    text_bounds bounds(std::string_view string);
    text draw(std::string_view string, const color& color = colors::white);
    text draw(std::string_view string, std::uint32_t line_width, text_align align = text_align::left, const color& color = colors::white);

    cpt::font& font() noexcept
    {
        return m_font;
    }

    const cpt::font& font() const noexcept
    {
        return m_font;
    }

    text_drawer_options options() const noexcept
    {
        return m_options;
    }

private:
    struct draw_line_state
    {
        float current_x{};
        float current_y{};
        float lowest_x{};
        float lowest_y{};
        float greatest_x{};
        float greatest_y{};
        float texture_width{};
        float texture_height{};
    };

    void draw_line(std::string_view line, std::uint32_t line_width, text_align align, draw_line_state& state, std::vector<vertex>& vertices, const std::unordered_map<codepoint_t, std::pair<std::shared_ptr<glyph>, vec2f>>& cache, const color& color);

private:
    texture_ptr make_texture(std::string_view string, std::unordered_map<codepoint_t, std::pair<std::shared_ptr<glyph>, vec2f>>& cache, tph::command_buffer& command_buffer);
    const std::shared_ptr<glyph>& load_glyph(codepoint_t codepoint);

private:
    cpt::font m_font{};
    text_drawer_options m_options{};
    std::unordered_map<codepoint_t, std::shared_ptr<glyph>> m_cache{};
};

text CAPTAL_API draw_text(cpt::font& font, std::string_view string,  const color& color = colors::white, text_drawer_options options = text_drawer_options::kerning);
text CAPTAL_API draw_text(cpt::font&& font, std::string_view string, const color& color = colors::white, text_drawer_options options = text_drawer_options::kerning);
text CAPTAL_API draw_text(cpt::font& font, std::string_view string,  std::uint32_t line_width, text_align align = text_align::left, const color& color = colors::white, text_drawer_options options = text_drawer_options::kerning);
text CAPTAL_API draw_text(cpt::font&& font, std::string_view string, std::uint32_t line_width, text_align align = text_align::left, const color& color = colors::white, text_drawer_options options = text_drawer_options::kerning);

}

template<> struct cpt::enable_enum_operations<cpt::font_style> {static constexpr bool value{true};};

#endif
