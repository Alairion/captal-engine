#include "text.hpp"

#include <cassert>
#include <algorithm>
#include <fstream>
#include <ranges>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H

#include <captal_foundation/utility.hpp>

#include "engine.hpp"
#include "texture.hpp"
#include "algorithm.hpp"

namespace cpt
{

struct full_font_atlas{};

text::text(std::span<const std::uint32_t> indices, std::span<const vertex> vertices, std::weak_ptr<font_atlas> atlas, text_style style, std::uint32_t width, std::uint32_t height, std::size_t count)
:renderable{static_cast<std::uint32_t>(std::size(indices)), static_cast<std::uint32_t>(std::size(vertices))}
,m_width{width}
,m_height{height}
,m_style{style}
,m_count{count}
,m_atlas{std::move(atlas)}
{
    set_indices(indices);
    set_vertices(vertices);
    set_texture(m_atlas.lock()->texture());

    connect();
}

text::text(text&& other) noexcept
:renderable{std::move(other)}
,m_width{other.m_width}
,m_height{other.m_height}
,m_style{other.m_style}
,m_count{other.m_count}
,m_atlas{std::move(other.m_atlas)}
{
    other.m_connection.disconnect();
    connect();
}

text& text::operator=(text&& other) noexcept
{
    renderable::operator=(std::move(other));
    m_width = other.m_width;
    m_height = other.m_height;
    m_style = other.m_style;
    m_count = other.m_count;
    m_atlas = std::move(other.m_atlas);

    other.m_connection.disconnect();
    connect();

    return *this;
}

void text::set_color(const color& color)
{
    const vec4f native_color{static_cast<vec4f>(color)};
    for(auto& vertex : get_vertices())
    {
        vertex.color = native_color;
    }

    update();
}

void text::set_color(std::uint32_t character_index, const color& color)
{
    const auto current{get_vertices().subspan(character_index * 4, 4)};
    const vec4f native_color{static_cast<vec4f>(color)};

    current[0].color = native_color;
    current[1].color = native_color;
    current[2].color = native_color;
    current[3].color = native_color;

    update();
}

void text::set_color(std::uint32_t first, std::uint32_t count, const color& color)
{
    const auto vertices{get_vertices().subspan(first * 4)};
    const vec4f native_color{static_cast<vec4f>(color)};

    for(std::uint32_t i{}; i < count; ++i)
    {
        const auto current{vertices.subspan(i * 4, 4)};

        current[0].color = native_color;
        current[1].color = native_color;
        current[2].color = native_color;
        current[3].color = native_color;
    }

    update();
}

void text::connect()
{
    if(const auto atlas{m_atlas.lock()}; atlas)
    {
        m_connection = atlas->signal().connect([this](texture_ptr new_texture)
        {
            const auto old_width{static_cast<float>(texture()->width())};
            const auto old_height{static_cast<float>(texture()->height())};
            const auto new_width{static_cast<float>(new_texture->width())};
            const auto new_height{static_cast<float>(new_texture->height())};

            const vec2f factor{old_width / new_width, old_height / new_height};
            for(auto& vertex : get_vertices())
            {
                vertex.texture_coord *= factor;
            }

            set_texture(new_texture);
        });
    }
}

static std::uint64_t make_key(codepoint_t codepoint, std::uint64_t font_size, std::uint64_t adjustment, bool embolden) noexcept
{
    return (static_cast<std::uint64_t>(embolden) << 63) | (adjustment << 56) | (font_size << 32) | codepoint;
}

static bool need_embolden(font_category category, text_style style) noexcept
{
    return !static_cast<bool>(category & font_category::bold) && static_cast<bool>(style & text_style::bold);
}

static constexpr std::array adjustment_steps{1.0f, 0.5f, 0.25f, 0.125f, 0.0625f, 0.03125f, 0.015625f};

static std::uint64_t adjust(text_subpixel_adjustment adjustment, float x) noexcept
{
    const float padding{x - std::floor(x)};
    const float step{adjustment_steps[static_cast<std::uint32_t>(adjustment)]};

    return static_cast<std::uint64_t>(std::round(padding / step) * step * 64.0f);
}

static void add_glyph(std::vector<vertex>& vertices, float x, float y, float width, float height, const vec4f& color, vec2f texpos, vec2f texsize, bool flipped)
{
    if(flipped)
    {
        vertices.emplace_back(vertex{vec3f{x, y, 0.0f}, color, texpos / texsize});
        vertices.emplace_back(vertex{vec3f{x + width, y, 0.0f}, color, vec2f{texpos.x(), texpos.y() + width} / texsize});
        vertices.emplace_back(vertex{vec3f{x + width, y + height, 0.0f}, color, vec2f{texpos.x() + height, texpos.y() + width} / texsize});
        vertices.emplace_back(vertex{vec3f{x, y + height, 0.0f}, color, vec2f{texpos.x() + height, texpos.y()} / texsize});
    }
    else
    {
        vertices.emplace_back(vertex{vec3f{x, y, 0.0f}, color, texpos / texsize});
        vertices.emplace_back(vertex{vec3f{x + width, y, 0.0f}, color, vec2f{texpos.x() + width, texpos.y()} / texsize});
        vertices.emplace_back(vertex{vec3f{x + width, y + height, 0.0f}, color, vec2f{texpos.x() + width, texpos.y() + height} / texsize});
        vertices.emplace_back(vertex{vec3f{x, y + height, 0.0f}, color, vec2f{texpos.x(), texpos.y() + height} / texsize});
    }
}

static void add_placeholder(std::vector<vertex>& vertices)
{
    vertices.resize(std::size(vertices) + 4);
}

static std::vector<std::uint32_t> generate_indices(std::size_t codepoint_count)
{
    std::vector<std::uint32_t> indices{};
    indices.reserve(codepoint_count * 6);

    for(std::size_t i{}; i < codepoint_count; ++i)
    {
        const std::uint32_t shift{static_cast<std::uint32_t>(i * 4)};

        indices.emplace_back(shift + 0);
        indices.emplace_back(shift + 1);
        indices.emplace_back(shift + 2);
        indices.emplace_back(shift + 2);
        indices.emplace_back(shift + 3);
        indices.emplace_back(shift + 0);
    }

    return indices;
}

text_drawer::text_drawer(cpt::font font, text_drawer_options options, text_subpixel_adjustment adjustment, const tph::sampling_options& sampling)
:m_font{std::move(font)}
,m_options{options}
,m_adjustment{adjustment}
,m_sampling{sampling}
,m_atlas{std::make_shared<font_atlas>(m_font.info().format, m_sampling)}
{

}

text_bounds text_drawer::bounds(std::string_view string, text_style style)
{
    return bounds(string, std::numeric_limits<std::uint32_t>::max(), text_align::left, style);
}

text_bounds text_drawer::bounds(std::string_view string, std::uint32_t line_width, text_align align, text_style style)
{/*
    const bool embolden{need_embolden(m_font.info().category, style)};

    auto& atlas{ensure(string, embolden)};

    if(!load(atlas, U' ', m_font.info().size, embolden)) //ensure that the space glyph is loaded (in case the string doesn't contains any)
    {
        throw std::runtime_error{"How did you find a font without the space character ?"};
    }

    draw_line_state state{};
    state.current_y = static_cast<float>(m_font.info().max_ascent);
    state.lowest_y = static_cast<float>(m_font.info().max_glyph_height);
    state.line_width = line_width;
    state.style = style;
    state.font_size = m_font.info().size;

    for(auto&& line : split(string, '\n'))
    {
        line_bounds(atlas, line, align, state);

        state.current_x = 0.0f;
        state.current_y += m_font.info().line_height;
    }

    const auto text_width{static_cast<std::uint32_t>(state.greatest_x - state.lowest_x)};
    const auto text_height{static_cast<std::uint32_t>(state.greatest_y - state.lowest_y)};

    return text_bounds{text_width, text_height};*/
}

text text_drawer::draw(std::string_view string, text_style style, const color& color)
{
    return draw(string, std::numeric_limits<std::uint32_t>::max(), text_align::left, style, color);
}

text text_drawer::draw(std::string_view string, std::uint32_t line_width, text_align align, text_style style, const color& color)
{
    const auto codepoints{convert_to<utf32>(string)};

    draw_line_state state
    {
        .y = static_cast<float>(m_font.info().max_ascent),
        .lowest_y = static_cast<float>(m_font.info().max_glyph_height),
        .line_width = static_cast<float>(line_width),
        .texture_size = vec2f{static_cast<float>(m_atlas->texture()->width()), static_cast<float>(m_atlas->texture()->height())},
        .style = style,
        .color = color,
        .font_size = m_font.info().size,
        .codepoints = codepoints
    };

    state.vertices.reserve(std::size(state.codepoints) * 4 + 4);

    for(auto&& [line, _] : split(state.codepoints, U'\n'))
    {
        draw_line(line, align, state);

        state.x = 0.0f;
        state.y += m_font.info().line_height;

        add_placeholder(state.vertices);
    }

    //There is an additionnal newline at the end
    state.vertices.resize(std::size(state.vertices) - 4);

    const vec3f shift{-std::floor(state.lowest_x), -std::floor(state.lowest_y), 0.0f};
    for(auto& vertex : state.vertices)
    {
        vertex.position += shift;
    }

    const auto indices{generate_indices(std::size(state.vertices))};
    const auto text_width{static_cast<std::uint32_t>(state.greatest_x - state.lowest_x)};
    const auto text_height{static_cast<std::uint32_t>(state.greatest_y - state.lowest_y)};

    return text{indices, state.vertices, m_atlas, style, text_width, text_height, std::size(state.vertices)};
}

void text_drawer::upload()
{
    if(m_atlas->need_upload())
    {
        m_atlas->upload();
    }
}

#ifdef CAPTAL_DEBUG
void text_drawer::set_name(std::string_view name)
{
    m_name = name;

    m_atlas->set_name(m_name + " atlas" );
}
#endif

void text_drawer::draw_line(std::u32string_view line, text_align align, draw_line_state& state)
{
    if(align == text_align::left)
    {
        draw_left_aligned(line, state);
    }
    else if(align == text_align::right)
    {
        draw_right_aligned(line, state);
    }
    else if(align == text_align::center)
    {
        draw_center_aligned(line, state);
    }
    else
    {
        assert(false && "cpt::text_align::justify is not supported yet");
    }
}

void text_drawer::draw_left_aligned(std::u32string_view line, draw_line_state& state)
{
    const float space{load(make_key(U' ', state.font_size, 0, false)).advance};
    const bool embolden{need_embolden(m_font.info().category, state.style)};

    codepoint_t last{};
    for(auto&& [word, _] : split(line, U' '))
    {
        const auto shift{state.x - std::floor(state.x)};
        if(state.x + word_width(word, state.font_size, embolden, last, shift) > state.line_width)
        {
            state.x = 0.0f;
            state.y += m_font.info().line_height;
            last = 0;
        }

        for(const auto codepoint : word)
        {
            const vec2f kerning{m_font.kerning(last, codepoint)};
            const auto  key    {make_key(codepoint, state.font_size, adjust(m_adjustment, state.x + kerning.x()), embolden)};
            const auto& glyph  {load(key)};

            const vec2f texpos{static_cast<float>(glyph.rect.x), static_cast<float>(glyph.rect.y)};
            const float width {static_cast<float>(glyph.flipped ? glyph.rect.height : glyph.rect.width)};
            const float height{static_cast<float>(glyph.flipped ? glyph.rect.width : glyph.rect.height)};

            if(width > 0.0f)
            {
                const float x_padding{last != 0 ? glyph.origin.x() + kerning.x() : 0.0f};

                const float x{state.x + x_padding};
                const float y{state.y + glyph.origin.y() + kerning.y()};

                add_glyph(state.vertices, std::floor(x), y, width, height, static_cast<vec4f>(state.color), texpos, state.texture_size, glyph.flipped);

                state.lowest_x = std::min(state.lowest_x, x);
                state.lowest_y = std::min(state.lowest_y, y);
                state.greatest_x = std::max(state.greatest_x, x + width);
                state.greatest_y = std::max(state.greatest_y, y + height);
            }
            else
            {
                add_placeholder(state.vertices);
            }

            state.x += glyph.advance;
            last = codepoint;
        }

        add_placeholder(state.vertices);

        state.x += space;
        last = U' ';
    }

    //There is an additionnal space at the end
    state.vertices.resize(std::size(state.vertices) - 4);
}

void text_drawer::draw_right_aligned(std::u32string_view line, draw_line_state& state)
{
    state.lowest_x = state.line_width;
    state.greatest_x = state.line_width;

    const float space{load(make_key(U' ', state.font_size, 0, false)).advance};
    const bool embolden{need_embolden(m_font.info().category, state.style)};

    std::size_t begin{std::size(state.vertices)};
    std::size_t count{};
    float lowest_x{};
    float greatest_x{};
    codepoint_t last{};

    auto do_shift = [&state](std::size_t begin, std::size_t count, float lowest_x, float greatest_x) mutable
    {
        const float width{std::floor(greatest_x - lowest_x)};
        const float shift{state.line_width - width};

        for(auto& vertex : std::span{std::begin(state.vertices) + begin, count * 4})
        {
            vertex.position.x() += shift;
        }

        state.lowest_x = std::min(state.lowest_x, shift);
    };

    for(auto&& [word, _] : split(line, U' '))
    {
        const auto shift{state.x - std::floor(state.x)};
        if(state.x + word_width(word, state.font_size, embolden, last, shift) > state.line_width)
        {
            do_shift(begin, count, lowest_x, greatest_x);

            state.x = 0.0f;
            state.y += m_font.info().line_height;

            begin = std::size(state.vertices);
            count = 0;
            lowest_x = 0.0f;
            greatest_x = 0.0f;
            last = 0;
        }

        for(const auto codepoint : word)
        {
            const vec2f kerning{m_font.kerning(last, codepoint)};
            const auto  key    {make_key(codepoint, state.font_size, adjust(m_adjustment, state.x + kerning.x()), embolden)};
            const auto& glyph  {load(key)};

            const vec2f texpos{static_cast<float>(glyph.rect.x), static_cast<float>(glyph.rect.y)};
            const float width {static_cast<float>(glyph.flipped ? glyph.rect.height : glyph.rect.width)};
            const float height{static_cast<float>(glyph.flipped ? glyph.rect.width : glyph.rect.height)};

            if(width > 0.0f)
            {
                const float x_padding{last != 0 ? glyph.origin.x() + kerning.x() : 0.0f};

                const float x{state.x + x_padding};
                const float y{state.y + glyph.origin.y() + kerning.y()};

                add_glyph(state.vertices, std::floor(x), y, width, height, static_cast<vec4f>(state.color), texpos, state.texture_size, glyph.flipped);

                lowest_x = std::min(lowest_x, x);
                greatest_x = std::max(greatest_x, x + width);

                state.lowest_y = std::min(state.lowest_y, y);
                state.greatest_y = std::max(state.greatest_y, y + height);
            }
            else
            {
                add_placeholder(state.vertices);
            }

            state.x += glyph.advance;
            count += 1;
            last = codepoint;
        }

        add_placeholder(state.vertices);

        state.x += space;
        count += 1;
        last = U' ';
    }

    do_shift(begin, count, lowest_x, greatest_x);

    //There is an additionnal space at the end
    state.vertices.resize(std::size(state.vertices) - 4);
}

void text_drawer::draw_center_aligned(std::u32string_view line, draw_line_state& state)
{
    state.lowest_x = static_cast<float>(state.line_width);

    const float space{load(make_key(U' ', state.font_size, 0, false)).advance};
    const bool embolden{need_embolden(m_font.info().category, state.style)};

    codepoint_t last{};
    line_width_info line_info{.remainder = line};

    do
    {
        line_info = line_width(line_info.remainder, state.font_size, embolden, state.line_width, space);

        state.x = (state.line_width - line_info.width) / 2.0f;

        for(auto&& [word, _] : split(line_info.line, U' '))
        {
            for(const auto codepoint : word)
            {
                const vec2f kerning{m_font.kerning(last, codepoint)};
                const auto  key    {make_key(codepoint, state.font_size, adjust(m_adjustment, state.x + kerning.x()), embolden)};
                const auto& glyph  {load(key)};

                const vec2f texpos{static_cast<float>(glyph.rect.x), static_cast<float>(glyph.rect.y)};
                const float width {static_cast<float>(glyph.flipped ? glyph.rect.height : glyph.rect.width)};
                const float height{static_cast<float>(glyph.flipped ? glyph.rect.width : glyph.rect.height)};

                if(width > 0.0f)
                {
                    const float x_padding{last != 0 ? glyph.origin.x() + kerning.x() : 0.0f};

                    const float x{state.x + x_padding};
                    const float y{state.y + glyph.origin.y() + kerning.y()};

                    add_glyph(state.vertices, std::floor(x), y, width, height, static_cast<vec4f>(state.color), texpos, state.texture_size, glyph.flipped);

                    state.lowest_x = std::min(state.lowest_x, x);
                    state.lowest_y = std::min(state.lowest_y, y);
                    state.greatest_x = std::max(state.greatest_x, x + width);
                    state.greatest_y = std::max(state.greatest_y, y + height);
                }
                else
                {
                    add_placeholder(state.vertices);
                }

                state.x += glyph.advance;
                last = codepoint;
            }

            add_placeholder(state.vertices);

            state.x += space;
            last = U' ';
        }

        state.y += m_font.info().line_height;
        last = 0;

    } while(!std::empty(line_info.line) && !std::empty(line_info.remainder));

    //There is an additionnal space at the end
    state.vertices.resize(std::size(state.vertices) - 4);
}

void text_drawer::line_bounds(std::u32string_view line, text_align align, draw_line_state& state)
{
    if(align == text_align::left || align == text_align::right || align == text_align::center)
    {
        default_bounds(line, state);
    }
    else
    {
        assert(false && "cpt::text_align::justify is not supported yet");
    }
}

void text_drawer::default_bounds(std::u32string_view line, draw_line_state& state)
{
    const float space{load(make_key(U' ', state.font_size, 0, false)).advance};
    const bool embolden{need_embolden(m_font.info().category, state.style)};

    codepoint_t last{};
    for(auto&& [word, _] : split(line, U' '))
    {
        const auto shift{state.x - std::floor(state.x)};
        if(state.x + word_width(word, state.font_size, embolden, last, shift) > state.line_width)
        {
            state.x = 0.0f;
            state.y += m_font.info().line_height;
            last = 0;
        }

        for(const auto codepoint : word)
        {
            const auto key{make_key(codepoint, state.font_size, adjust(m_adjustment, state.x), embolden)};
            const glyph_info& glyph{load(key)};

            const vec2f texpos{static_cast<float>(glyph.rect.x), static_cast<float>(glyph.rect.y)};
            const float width {static_cast<float>(glyph.flipped ? glyph.rect.height : glyph.rect.width)};
            const float height{static_cast<float>(glyph.flipped ? glyph.rect.width : glyph.rect.height)};

            if(width > 0.0f)
            {
                const vec2f kerning{m_font.kerning(last, codepoint)};
                const float x_padding{last != 0 ? glyph.origin.x() + kerning.x() : 0.0f};

                const float x{state.x + x_padding};
                const float y{state.y + glyph.origin.y() + kerning.y()};

                state.lowest_x = std::min(state.lowest_x, x);
                state.lowest_y = std::min(state.lowest_y, y);
                state.greatest_x = std::max(state.greatest_x, x + width);
                state.greatest_y = std::max(state.greatest_y, y + height);
            }

            state.x += glyph.advance;
            last = codepoint;
        }

        state.x += space;
        last = U' ';
    }
}

const text_drawer::glyph_info& text_drawer::load(std::uint64_t key)
{
    const auto it{m_glyphs.find(key)};
    if(it == std::end(m_glyphs))
    {
        const auto codepoint{static_cast<codepoint_t>(key)};
        const auto embolden{static_cast<bool>(key >> 63)};
        const auto adjustement{(key >> 56) & 0x7F};

        if(!m_font.has(codepoint))
        {
            //Load the fallback in case the requested codepoint does not have a glyph inside the font
            if(codepoint != m_fallback)
            {
                return load(make_key(m_fallback, m_font.info().size, adjustement, embolden));
            }
            else
            {
                throw std::runtime_error{"Can not render text, '" + convert_to<narrow>(std::u32string_view{&codepoint, 1}) + "' is not available nor is '" + convert_to<narrow>(std::u32string_view{&m_fallback, 1}) + "'"};
            }
        }

        const auto glyph{m_font.load_image(codepoint, embolden, adjustement / 64.0f)};

        if(glyph)
        {
            glyph_info info{};
            info.origin = glyph->origin;
            info.advance = glyph->advance;

            if(glyph->width != 0)
            {
                const auto rect{m_atlas->add_glyph(glyph->data, glyph->width, glyph->height)};
                if(!rect)
                {
                    throw full_font_atlas{};
                }

                info.rect = rect.value();

                if(rect->width != glyph->width)
                {
                    info.flipped = true;
                }
            }

            return m_glyphs.emplace(key, info).first->second;
        }
    }

    return it->second;
}


float text_drawer::word_width(std::u32string_view word, std::uint64_t font_size, bool embolden, codepoint_t last, float base_shift)
{
    float current_x{base_shift};
    float lowest_x{base_shift};
    float greatest_x{base_shift};

    for(const auto codepoint : word)
    {
        const vec2f kerning{m_font.kerning(last, codepoint)};
        const auto  key    {make_key(codepoint, font_size, adjust(m_adjustment, current_x + kerning.x()), embolden)};
        const auto& glyph  {load(key)};

        const float width{static_cast<float>(glyph.flipped ? glyph.rect.height : glyph.rect.width)};

        if(width > 0.0f)
        {
            const float x_padding{last != 0 ? glyph.origin.x() + kerning.x() : 0.0f};
            const float x{current_x + x_padding};

            lowest_x = std::min(lowest_x, x);
            greatest_x = std::max(greatest_x, x + width);
        }

        current_x += glyph.advance;
        last = codepoint;
    }

    return greatest_x - lowest_x;
}

text_drawer::line_width_info text_drawer::line_width(std::u32string_view line, std::uint64_t font_size, bool embolden, float line_width, float space)
{
    float current_x{};
    float greatest_x{};
    codepoint_t last{};
    line_width_info output{};

    for(auto&& [word, remainder] : split(line, U' '))
    {
        const float shift{current_x - std::floor(current_x)};
        current_x += word_width(word, font_size, embolden, last, shift);

        if(current_x > line_width)
        {
            break;
        }

        greatest_x = current_x;

        current_x += space;
        last = U' ';
        output.remainder = remainder;
    }

    output.line = line.substr(0, std::size(line) - std::size(output.remainder) - 1);
    output.width = greatest_x;

    return output;
}

}
