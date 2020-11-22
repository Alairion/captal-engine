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
    explicit text(std::span<const std::uint32_t> indices, std::span<const vertex> vertices, std::weak_ptr<font_atlas> atlas, text_style style, std::uint32_t width, std::uint32_t height, std::size_t count);

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
    std::weak_ptr<font_atlas> m_atlas{};
    scoped_connection m_connection{};
};

enum class text_drawer_options : std::uint32_t
{
    none = 0x00,
};

enum class text_subpixel_adjustment : std::uint32_t
{
    x1 = 0,  //1  step  of width 1
    x2 = 1,  //2  steps of width 0.5
    x4 = 2,  //4  steps of width 0.25
    x8 = 3,  //8  steps of width 0.125
    x16 = 4, //16 steps of width 0.0625
    x32 = 5, //32 steps of width 0.03125
    x64 = 6, //64 steps of width 0.015625
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
    explicit text_drawer(cpt::font font, text_drawer_options options = text_drawer_options::none, text_subpixel_adjustment adjustment = text_subpixel_adjustment::x2, const tph::sampling_options& sampling = tph::sampling_options{});

    ~text_drawer() = default;
    text_drawer(const text_drawer&) = delete;
    text_drawer& operator=(const text_drawer&) = delete;
    text_drawer(text_drawer&&) noexcept = default;
    text_drawer& operator=(text_drawer&&) noexcept = default;

    void set_font(cpt::font font) noexcept;
    void set_fallback(codepoint_t codepoint);
    void resize(std::uint32_t pixels_size);

    void set_subpixel_adjustement(text_subpixel_adjustment adjustment) noexcept
    {
        m_adjustment = adjustment;
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

    text_subpixel_adjustment subpixel_adjustment() const noexcept
    {
        return m_adjustment;
    }

#ifdef CAPTAL_DEBUG
    void set_name(std::string_view name);
#else
    void set_name(std::string_view name [[maybe_unused]]) const noexcept
    {

    }
#endif

private:
    struct glyph_info
    {
        vec2f origin{};
        float advance{};
        bin_packer::rect rect{};
        bool flipped{};
        bool deferred{};
    };

    struct draw_line_state
    {
        float x{};
        float y{};
        float lowest_x{};
        float lowest_y{};
        float greatest_x{};
        float greatest_y{};
        float line_width{};
        vec2f texture_size{};
        text_style style{};
        color color{};
        std::uint64_t font_size{};
        std::u32string_view codepoints{};
        std::vector<vertex> vertices{};
    };

    struct word_width_info
    {
        float width{};
        float advance{};
    };

    struct line_width_info
    {
        float width{};
        std::u32string_view line{};
        std::u32string_view remainder{};
    };

private:
    void draw_line(std::u32string_view line, text_align align, draw_line_state& state);
    void draw_left_aligned   (std::u32string_view line, draw_line_state& state);
    void draw_right_aligned  (std::u32string_view line, draw_line_state& state);
    void draw_center_aligned (std::u32string_view line, draw_line_state& state);
    void draw_justify_aligned(std::u32string_view line, draw_line_state& state);

    void line_bounds(std::u32string_view line, text_align align, draw_line_state& state);
    void default_bounds(std::u32string_view line, draw_line_state& state);

private:
    const glyph_info& load(std::uint64_t key, bool deferred = false);
    word_width_info word_width(std::u32string_view word, std::uint64_t font_size, bool embolden, codepoint_t last, float base_shift);
    line_width_info line_width(std::u32string_view line, std::uint64_t font_size, bool embolden, float line_width);

private:
    cpt::font m_font{};
    text_drawer_options m_options{};
    text_subpixel_adjustment m_adjustment{};
    tph::sampling_options m_sampling{};
    codepoint_t m_fallback{default_fallback};
    float m_space{};
    std::shared_ptr<font_atlas> m_atlas{};
    std::unordered_map<std::uint64_t, glyph_info> m_glyphs{};
#ifdef CAPTAL_DEBUG
    std::string m_name{};
#endif
};

}

template<> struct cpt::enable_enum_operations<cpt::text_style> {static constexpr bool value{true};};
template<> struct cpt::enable_enum_operations<cpt::text_drawer_options> {static constexpr bool value{true};};

#endif
