#ifndef CAPTAL_TEXT_HPP_INCLUDED
#define CAPTAL_TEXT_HPP_INCLUDED

#include "config.hpp"

#include "color.hpp"
#include "renderable.hpp"
#include "font.hpp"

namespace cpt
{

enum class text_style : std::uint32_t
{
    regular = 0x00,
    italic = 0x01,
    bold = 0x02,
    underlined = 0x04,
    strikethrough = 0x08,
};

class CAPTAL_API text final : public renderable
{
public:
    text() = default;
    explicit text(std::span<const std::uint32_t> indices, std::span<const vertex> vertices, std::shared_ptr<font_atlas> atlas, text_style style, std::uint32_t width, std::uint32_t height, std::size_t count);

    ~text() = default;
    text(const text&) = delete;
    text& operator=(const text&) = delete;
    text(text&& other) noexcept;
    text& operator=(text&& other) noexcept;

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
    void connect();

private:
    std::uint32_t m_width{};
    std::uint32_t m_height{};
    text_style m_style{};
    std::size_t m_count{};
    std::shared_ptr<font_atlas> m_atlas{};
    scoped_connection m_connection{};
};

enum class text_drawer_options : std::uint32_t
{
    none = 0x00
};

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
    static constexpr codepoint_t default_fallback{U'?'};

public:
    text_drawer() = default;
    explicit text_drawer(cpt::font font, text_drawer_options options = text_drawer_options::none, const tph::sampling_options& sampling = tph::sampling_options{});

    ~text_drawer() = default;
    text_drawer(const text_drawer&) = delete;
    text_drawer& operator=(const text_drawer&) = delete;
    text_drawer(text_drawer&&) noexcept = default;
    text_drawer& operator=(text_drawer&&) noexcept = default;

    void set_font(cpt::font font) noexcept
    {
        m_font = std::move(font);
        m_atlases.clear();
    }

    void set_fallback(codepoint_t codepoint) noexcept
    {
        m_fallback = codepoint;
    }

    void resize(std::uint32_t pixels_size)
    {
        m_font.resize(pixels_size);
    }

    text_bounds bounds(std::string_view string, text_style style = text_style::regular);
    text_bounds bounds(std::string_view string, std::uint32_t line_width, text_align align = text_align::left, text_style style = text_style::regular);
    text draw(std::string_view string, text_style style = text_style::regular, const color& color = colors::white);
    text draw(std::string_view string, std::uint32_t line_width, text_align align = text_align::left, text_style style = text_style::regular, const color& color = colors::white);
    void upload();

    cpt::font drain_font() noexcept
    {
        return std::move(m_font);
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
    struct glyph_info
    {
        vec2f origin{};
        float advance{};
        float ascent{};
        float descent{};
        bin_packer::rect rect{};
    };

    struct atlas_info
    {
        std::shared_ptr<font_atlas> atlas{};
        std::unordered_map<std::uint64_t, glyph_info> glyphs{};
    };

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
    atlas_info& ensure(std::string_view string, text_style style);
    bool load(atlas_info& atlas, codepoint_t codepoint, std::uint64_t font_size, bool embolden, bool fallback = true);

private:
    cpt::font m_font{};
    text_drawer_options m_options{};
    codepoint_t m_fallback{default_fallback};
    tph::sampling_options m_sampling{};
    std::vector<atlas_info> m_atlases{};
};

text CAPTAL_API draw_text(cpt::font& font, std::string_view string,  const color& color = colors::white, text_drawer_options options = text_drawer_options::none);
text CAPTAL_API draw_text(cpt::font&& font, std::string_view string, const color& color = colors::white, text_drawer_options options = text_drawer_options::none);
text CAPTAL_API draw_text(cpt::font& font, std::string_view string,  std::uint32_t line_width, text_align align = text_align::left, const color& color = colors::white, text_drawer_options options = text_drawer_options::none);
text CAPTAL_API draw_text(cpt::font&& font, std::string_view string, std::uint32_t line_width, text_align align = text_align::left, const color& color = colors::white, text_drawer_options options = text_drawer_options::none);

}

template<> struct cpt::enable_enum_operations<cpt::text_style> {static constexpr bool value{true};};
template<> struct cpt::enable_enum_operations<cpt::text_drawer_options> {static constexpr bool value{true};};

#endif
