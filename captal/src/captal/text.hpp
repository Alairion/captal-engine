#ifndef CAPTAL_TEXT_HPP_INCLUDED
#define CAPTAL_TEXT_HPP_INCLUDED

#include "config.hpp"

#include "color.hpp"
#include "renderable.hpp"
#include "font.hpp"

namespace cpt
{

struct full_font_atlas final : std::exception
{
    full_font_atlas() noexcept = default;
    ~full_font_atlas() = default;
    full_font_atlas(const full_font_atlas&) noexcept = default;
    full_font_atlas& operator=(const full_font_atlas&) noexcept = default;
    full_font_atlas(full_font_atlas&& other) noexcept = default;
    full_font_atlas& operator=(full_font_atlas&& other) noexcept = default;

    const char* what() const noexcept override
    {
        return "cpt::font_atlas is full";
    }
};

enum class text_style : std::uint32_t
{
    regular = 0x00,
    italic = 0x01,
    bold = 0x02,
    underlined = 0x04,
    strikethrough = 0x08,
};

struct alignas(std::uint64_t) text_bounds
{
    std::uint32_t width{};
    std::uint32_t height{};
};

class CAPTAL_API text final : public basic_renderable
{
    friend class text_drawer;

public:
    text() = default;
    ~text() = default;
    text(const text&) = delete;
    text& operator=(const text&) = delete;
    text(text&& other) noexcept;
    text& operator=(text&& other) noexcept;

    void set_color(const cpt::color& color);

    text_bounds bounds() const noexcept
    {
        return m_bounds;
    }

    std::uint32_t width() const noexcept
    {
        return m_bounds.width;
    }

    std::uint32_t height() const noexcept
    {
        return m_bounds.height;
    }

private:
    explicit text(std::span<const std::uint32_t> indices, std::span<const vertex> vertices, std::weak_ptr<font_atlas> atlas, text_bounds bounds);

    void connect();

private:
    text_bounds m_bounds{};
    std::weak_ptr<font_atlas> m_atlas{};
    scoped_connection m_connection{};
};

enum class text_drawer_options : std::uint32_t
{
    none = 0x00,
    no_kerning = 0x01
};

enum class subpixel_adjustment : std::uint32_t
{
    x1 = 0,  //1  step  of width 1.0px
    x2 = 1,  //2  steps of width 0.5px
    x4 = 2,  //4  steps of width 0.25px
    x8 = 3,  //8  steps of width 0.125px
    x16 = 4, //16 steps of width 0.0625px
    x32 = 5, //32 steps of width 0.03125px
    x64 = 6, //64 steps of width 0.015625px
};

enum class text_align : std::uint32_t
{
    left = 0,
    right = 1,
    center = 2,
    justify = 3
};

struct font_set
{
    std::optional<font> regular{};
    std::optional<font> italic{};
    std::optional<font> bold{};
    std::optional<font> italic_bold{};
};

class CAPTAL_API text_drawer
{
public:
    static constexpr codepoint_t default_fallback{U'?'};

private:
    static constexpr codepoint_t line_filler_codepoint{0x110000};

public:
    text_drawer() = default;
    explicit text_drawer(font_set&& fonts, text_drawer_options options = text_drawer_options::none, glyph_format format = glyph_format::gray, const tph::sampler_info& sampling = tph::sampler_info{});

    ~text_drawer() = default;
    text_drawer(const text_drawer&) = delete;
    text_drawer& operator=(const text_drawer&) = delete;
    text_drawer(text_drawer&&) noexcept = default;
    text_drawer& operator=(text_drawer&&) noexcept = default;

    void resize(std::uint32_t pixels_size);

    void set_fallback(codepoint_t codepoint)
    {
        m_fallback = codepoint;
    }

    void set_option(text_drawer_options options) noexcept
    {
        m_options = options;
    }

    void set_adjustement(subpixel_adjustment adjustment) noexcept
    {
        m_adjustment = adjustment;
    }

    void set_line_adjustement(subpixel_adjustment adjustment) noexcept
    {
        m_line_adjustment = adjustment;
    }

    void set_style(text_style style) noexcept
    {
        m_style = style;
    }

    void set_color(const color& color) noexcept
    {
        m_color = static_cast<vec4f>(color);
    }

    void set_outline_color(const color& color) noexcept
    {
        m_outline_color = static_cast<vec4f>(color);
    }

    void set_underline_color(const color& color) noexcept
    {
        m_underline_color = static_cast<vec4f>(color);
    }

    void set_align(text_align align) noexcept
    {
        m_align = align;
    }

    void set_outline(float outline) noexcept
    {
        m_outline = outline;
    }

    text_bounds bounds(std::string_view string, std::uint32_t line_width = std::numeric_limits<std::uint32_t>::max());
    text draw(std::string_view string, std::uint32_t line_width = std::numeric_limits<std::uint32_t>::max());

    void upload();

    const font_set& fonts() const noexcept
    {
        return m_fonts;
    }

#ifdef CAPTAL_DEBUG
    void set_name(std::string_view name);
#else
    void set_name(std::string_view name [[maybe_unused]]) const noexcept
    {

    }
#endif

private:
    template<typename T>
    struct font_data //contains a value of any type for each font
    {
        T regular{};
        T italic{};
        T bold{};
        T italic_bold{};
    };

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
        cpt::font& font;
        float x{};
        float y{};
        float lowest_x{};
        float lowest_y{};
        float greatest_x{};
        float greatest_y{};
        float line_width{};
        float space{};
        vec2f texture_size{};
        std::uint64_t base_key{};
        std::u32string_view codepoints{};
        std::vector<vertex> vertices{};
        std::vector<vertex> lines{};
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

    struct word_bbox_info
    {
        float lowest_x{};
        float greatest_x{};
        float lowest_y{};
        float greatest_y{};
        float advance{};
    };

    struct line_bbox_info
    {
        float lowest_x{};
        float greatest_x{};
        float lowest_y{};
        float greatest_y{};
        std::u32string_view line{};
        std::u32string_view remainder{};
    };

private:
    font_data<float> compute_spaces();
    cpt::font& choose_font() noexcept;
    float choose_space() noexcept;

    void bounds(std::u32string_view line, draw_line_state& state);
    void bounds_default(std::u32string_view line, draw_line_state& state);
    void bounds_justify(std::u32string_view line, draw_line_state& state);

    void draw(std::u32string_view line, draw_line_state& state);
    void draw_left_aligned   (std::u32string_view line, draw_line_state& state);
    void draw_right_aligned  (std::u32string_view line, draw_line_state& state);
    void draw_center_aligned (std::u32string_view line, draw_line_state& state);
    void draw_justify_aligned(std::u32string_view line, draw_line_state& state);

    void add_underline(float line_width, draw_line_state& state);
    void add_strikeline(float line_width, draw_line_state& state);

    const glyph_info& load(cpt::font& font, std::uint64_t key, bool deferred = false);
    const glyph_info& load_line_filler(cpt::font& font, std::uint64_t base_key, float shift);

    word_width_info word_width(cpt::font& font, std::u32string_view word, std::uint64_t base_key, codepoint_t last, float base_shift);
    line_width_info line_width(cpt::font& font, std::u32string_view line, std::uint64_t base_key, float space, float line_width);
    word_bbox_info  word_bbox (cpt::font& font, std::u32string_view word, std::uint64_t base_key, codepoint_t last, float base_shift);
    line_bbox_info  line_bbox (cpt::font& font, std::u32string_view line, std::uint64_t base_key, float space, float line_width);

    vec2f get_kerning(cpt::font& font, codepoint_t left, codepoint_t right) const noexcept;

private:
    font_set m_fonts{};
    glyph_format m_format{};
    tph::sampler_info m_sampling{};

    text_drawer_options m_options{};
    subpixel_adjustment m_adjustment{cpt::subpixel_adjustment::x2};
    subpixel_adjustment m_line_adjustment{cpt::subpixel_adjustment::x1};
    codepoint_t m_fallback{default_fallback};
    text_style m_style{text_style::regular};
    vec4f m_color{1.0f, 1.0f, 1.0f, 1.0f};
    vec4f m_outline_color{0.0f, 0.0f, 0.0f, 1.0f};
    vec4f m_underline_color{0.0f, 0.0f, 0.0f, 1.0f};
    text_align m_align{text_align::left};
    float m_outline{};

    float m_line_filler{};
    font_data<float> m_spaces{};

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
