#ifndef CAPTAL_TEXT_HPP_INCLUDED
#define CAPTAL_TEXT_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <string_view>
#include <unordered_map>

#include <glm/vec2.hpp>

#include <tephra/texture.hpp>
#include <tephra/image.hpp>

#include "renderable.hpp"

namespace cpt
{

struct glyph
{
    glm::vec2 origin{};
    float advance{};
    float ascent{};
    float descent{};
    tph::image image{};
};

enum class font_style : std::uint32_t
{
    regular = 0x00,
    bold = 0x01,
    italic = 0x02,
    underlined = 0x04,
    strikethrough = 0x08,
};

template<> struct enable_enum_operations<font_style> {static constexpr bool value{true};};

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

class font
{
    struct freetype_info;

    struct freetype_deleter
    {
        void operator()(freetype_info* ptr) noexcept;
    };

public:
    font() = default;
    font(std::string_view data, load_from_memory_t, std::uint32_t initial_size = 16);
    font(std::string_view file, load_from_file_t, std::uint32_t initial_size = 16);
    ~font() = default;
    font(const font&) = delete;
    font& operator=(const font&) = delete;
    font(font&&) noexcept = default;
    font& operator=(font&&) noexcept = default;

    void resize(std::uint32_t pixels_size);
    void set_style(font_style style) noexcept;

    std::optional<glyph> load(char32_t codepoint);
    float kerning(char32_t left, char32_t right);

    const font_info& info() const noexcept
    {
        return m_info;
    }

private:
    std::unique_ptr<freetype_info, freetype_deleter> m_loader{};
    font_info m_info{};
};

class text : public renderable
{
public:
    text() = default;
    text(const std::vector<std::uint16_t>& indices, const std::vector<vertex>& vertices, texture_ptr texture, std::uint32_t width, std::uint32_t height);

    ~text() = default;
    text(const text&) = delete;
    text& operator=(const text&) = delete;
    text(text&&) noexcept = default;
    text& operator=(text&&) noexcept = default;

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
};

using text_ptr = std::shared_ptr<text>;

enum class text_drawer_options : std::uint32_t
{
    none = 0x00,
    cached = 0x01,
    kerning = 0x02
};

template<> struct enable_enum_operations<text_drawer_options> {static constexpr bool value{true};};

class text_drawer
{
public:
    text_drawer() = default;
    text_drawer(cpt::font font, text_drawer_options options = text_drawer_options::cached | text_drawer_options::kerning);
    ~text_drawer() = default;
    text_drawer(const text_drawer&) = delete;
    text_drawer& operator=(const text_drawer&) = delete;
    text_drawer(text_drawer&&) noexcept = default;
    text_drawer& operator=(text_drawer&&) noexcept = default;

    void set_font(cpt::font font) noexcept;
    void set_style(font_style style) noexcept;
    void resize(std::uint32_t pixels_size);

    text_ptr draw(std::string_view u8string, const glm::vec4& color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
    text_ptr draw(std::u32string u32string, const glm::vec4& color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});

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
    text_ptr draw_cached(std::u32string u32string, const glm::vec4& color);
    texture_ptr make_cached_texture(std::u32string u32string, std::unordered_map<char32_t, std::pair<std::shared_ptr<glyph>, glm::vec2>>& cache, tph::command_buffer& command_buffer);

    text_ptr draw_uncached(std::u32string string, const glm::vec4& color);
    texture_ptr make_uncached_texture(std::u32string string, std::unordered_map<char32_t, std::pair<glyph, glm::vec2>>& cache, tph::command_buffer& command_buffer);

private:
    cpt::font m_font{};
    text_drawer_options m_options{};
    std::unordered_map<char32_t, std::shared_ptr<glyph>> m_cache{};
};



}

#endif
