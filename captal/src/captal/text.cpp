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
#include <captal_foundation/stack_allocator.hpp>

#include "external/pugixml.hpp"

#include "engine.hpp"
#include "texture.hpp"
#include "algorithm.hpp"

namespace cpt
{

text::text(std::span<const std::uint32_t> indices, std::span<const vertex> vertices, std::weak_ptr<font_atlas> atlas, text_bounds bounds)
:basic_renderable{static_cast<std::uint32_t>(std::size(vertices)), static_cast<std::uint32_t>(std::size(indices)), 0}
,m_bounds{bounds}
,m_atlas{std::move(atlas)}
{
    set_indices(indices);
    set_vertices(vertices);
    set_binding(1, m_atlas.lock()->texture());

    connect();
}

text::text(text&& other) noexcept
:basic_renderable{std::move(other)}
,m_bounds{other.m_bounds}
,m_atlas{std::move(other.m_atlas)}
{
    other.m_connection.disconnect();
    connect();
}

text& text::operator=(text&& other) noexcept
{
    basic_renderable::operator=(std::move(other));
    m_bounds = other.m_bounds;
    m_atlas = std::move(other.m_atlas);

    other.m_connection.disconnect();
    connect();

    return *this;
}

void text::set_color(const cpt::color& color)
{
    const auto vertices    {basic_renderable::vertices()};
    const auto native_color{static_cast<vec4f>(color)};

    for(auto& vertex : vertices)
    {
        vertex.color = native_color;
    }
}

void text::connect()
{
    if(const auto atlas{m_atlas.lock()}; atlas)
    {
        m_connection = atlas->signal().connect([this](texture_ptr new_texture)
        {
            const auto old_texture{std::get<texture_ptr>(get_binding(1))};

            const auto old_width {static_cast<float>(old_texture->width())};
            const auto old_height{static_cast<float>(old_texture->height())};
            const auto new_width {static_cast<float>(new_texture->width())};
            const auto new_height{static_cast<float>(new_texture->height())};

            const vec2f factor{old_width / new_width, old_height / new_height};
            for(auto& vertex : basic_renderable::vertices())
            {
                vertex.texture_coord *= factor;
            }

            set_binding(1, new_texture);
        });
    }
}

static float compute_space(font& font)
{
    const auto space{font.load_no_render(' ')};
    if(space)
    {
        return space->advance;
    }

    return static_cast<float>(font.info().size) / 4.0f;
}

static std::uint64_t make_key(codepoint_t codepoint, std::uint64_t font_size, std::uint64_t outline, std::uint64_t adjustment, bool embolden, bool italic) noexcept
{
    //Key: [1 bit: italic][1 bit: bold][6 bits: ajustment][16 bits: outline][16 bits: font size][24 bits: codepoint]
    //Italic: 1 if the key represent a italic glyph
    //Bold: 1 if the key represent a bold glyph
    //Ajustments: subpixel adjustment in 26.6 format (64 = 1 pixel)
    //Outline: outline thickness in 26.6 format (64 = 1 pixel)
    //Font size: font size in pixels
    //Codepoint: the codepoint, use 24 bits even if only 21 are required
    //Parameters are assumed to fit in their limits, must be ensured from caller

    std::uint64_t output{};

    output |= static_cast<std::uint64_t>(italic) << 63;
    output |= static_cast<std::uint64_t>(embolden) << 62;
    output |= adjustment << 56;
    output |= outline << 40;
    output |= font_size << 24;
    output |= codepoint;

    return output;
}

static std::uint64_t make_base_key(std::uint64_t font_size, std::uint64_t outline, bool embolden, bool italic) noexcept
{
    std::uint64_t output{};

    output |= static_cast<std::uint64_t>(italic) << 63;
    output |= static_cast<std::uint64_t>(embolden) << 62;
    output |= outline << 40;
    output |= font_size << 24;

    return output;
}

static std::uint64_t combine_keys(std::uint64_t base, codepoint_t codepoint, std::uint64_t adjustment) noexcept
{
    base |= adjustment << 56;
    base |= codepoint;

    return base;
}

static constexpr std::array adjustment_steps{1.0f, 0.5f, 0.25f, 0.125f, 0.0625f, 0.03125f, 0.015625f};

static std::uint64_t adjust(subpixel_adjustment adjustment, float x) noexcept
{
    const float padding{x - std::floor(x)};
    const float step   {adjustment_steps[static_cast<std::uint32_t>(adjustment)]};
    const float shift  {std::round(padding / step) * step * 64.0f};

    //modulo so 64 == 0 (otherwise it break text rendering by adding 1px padding)
    return static_cast<std::uint64_t>(shift) % 64;
}

static void add_glyph(std::vector<vertex>& vertices, float x, float y, float width, float height, const vec4f& color, vec2f texpos, vec2f texsize, bool flipped)
{
    if(flipped)
    {
        vertices.emplace_back(vec3f{x, y, 0.0f}, color, texpos / texsize);
        vertices.emplace_back(vec3f{x + width, y, 0.0f}, color, vec2f{texpos.x(), texpos.y() + width} / texsize);
        vertices.emplace_back(vec3f{x + width, y + height, 0.0f}, color, vec2f{texpos.x() + height, texpos.y() + width} / texsize);
        vertices.emplace_back(vec3f{x, y + height, 0.0f}, color, vec2f{texpos.x() + height, texpos.y()} / texsize);
    }
    else
    {
        vertices.emplace_back(vec3f{x, y, 0.0f}, color, texpos / texsize);
        vertices.emplace_back(vec3f{x + width, y, 0.0f}, color, vec2f{texpos.x() + width, texpos.y()} / texsize);
        vertices.emplace_back(vec3f{x + width, y + height, 0.0f}, color, vec2f{texpos.x() + width, texpos.y() + height} / texsize);
        vertices.emplace_back(vec3f{x, y + height, 0.0f}, color, vec2f{texpos.x(), texpos.y() + height} / texsize);
    }
}

static void add_line(std::vector<vertex>& vertices, float x, float y, float width, float height, const vec4f& color, vec2f texpos, vec2f texsize, bool flipped)
{
    if(flipped)
    {
        vertices.emplace_back(vec3f{x, y, 0.0f}, color, texpos / texsize);
        vertices.emplace_back(vec3f{x + width, y, 0.0f}, color, vec2f{texpos.x(), texpos.y() + 1.0f} / texsize);
        vertices.emplace_back(vec3f{x + width, y + height, 0.0f}, color, vec2f{texpos.x() + height, texpos.y() + 1.0f} / texsize);
        vertices.emplace_back(vec3f{x, y + height, 0.0f}, color, vec2f{texpos.x() + height, texpos.y()} / texsize);
    }
    else
    {
        vertices.emplace_back(vec3f{x, y, 0.0f}, color, texpos / texsize);
        vertices.emplace_back(vec3f{x + width, y, 0.0f}, color, vec2f{texpos.x() + 1.0f, texpos.y()} / texsize);
        vertices.emplace_back(vec3f{x + width, y + height, 0.0f}, color, vec2f{texpos.x() + 1.0f, texpos.y() + height} / texsize);
        vertices.emplace_back(vec3f{x, y + height, 0.0f}, color, vec2f{texpos.x(), texpos.y() + height} / texsize);
    }
}

static std::vector<std::uint32_t> generate_indices(std::size_t codepoint_count)
{
    std::vector<std::uint32_t> indices{};
    indices.reserve(codepoint_count * 6);

    for(std::uint32_t i{}; i < codepoint_count; ++i)
    {
        const std::uint32_t shift{i * 4};

        indices.emplace_back(shift + 0);
        indices.emplace_back(shift + 1);
        indices.emplace_back(shift + 2);
        indices.emplace_back(shift + 2);
        indices.emplace_back(shift + 3);
        indices.emplace_back(shift + 0);
    }

    return indices;
}

text_drawer::text_drawer(font_set&& fonts, text_drawer_options options, glyph_format format, const tph::sampler_info& sampling)
:m_fonts{std::move(fonts)}
,m_format{format}
,m_sampling{sampling}
,m_options{options}
,m_spaces{compute_spaces()}
,m_atlas{std::make_shared<font_atlas>(m_format, m_sampling)}
{
    assert((m_fonts.regular || m_fonts.italic || m_fonts.bold || m_fonts.italic_bold) && "You must give at least one font to cpt::text_drawer");
}

void text_drawer::resize(uint32_t pixels_size)
{
    if(m_fonts.regular)
    {
        m_fonts.regular->resize(pixels_size);
    }

    if(m_fonts.italic)
    {
        m_fonts.italic->resize(pixels_size);
    }

    if(m_fonts.bold)
    {
        m_fonts.bold->resize(pixels_size);
    }

    if(m_fonts.italic_bold)
    {
        m_fonts.italic_bold->resize(pixels_size);
    }

    m_spaces = compute_spaces();
}

text_bounds text_drawer::bounds(std::string_view string, std::uint32_t line_width)
{
    const auto outline   {static_cast<std::uint64_t>(m_outline * 64.0f)};
    const auto bold      {static_cast<bool>(m_style & text_style::bold)};
    const auto italic    {static_cast<bool>(m_style & text_style::italic)};
    const auto codepoints{convert_to<utf32>(string)};

    auto& font{choose_font()};

    draw_line_state state
    {
        .font = font,
        .y = static_cast<float>(font.info().max_ascent),
        .lowest_y = static_cast<float>(font.info().max_glyph_height),
        .line_width = static_cast<float>(line_width),
        .space = choose_space(),
        .texture_size = vec2f{static_cast<float>(m_atlas->texture()->width()), static_cast<float>(m_atlas->texture()->height())},
        .base_key = make_base_key(font.info().size, outline, bold, italic),
        .codepoints = codepoints
    };

    for(auto&& [line, _] : split(state.codepoints, U'\n'))
    {
        bounds(line, state);
    }

    state.lowest_x = std::floor(state.lowest_x);
    state.lowest_y = std::floor(state.lowest_y);
    state.greatest_x = std::ceil(state.greatest_x);
    state.greatest_y = std::ceil(state.greatest_y);

    const auto text_width {static_cast<std::uint32_t>(state.greatest_x - state.lowest_x)};
    const auto text_height{static_cast<std::uint32_t>(state.greatest_y - state.lowest_y)};

    return text_bounds{text_width, text_height};
}

text text_drawer::draw(std::string_view string, std::uint32_t line_width)
{
    const auto outline   {static_cast<std::uint64_t>(m_outline * 64.0f)};
    const auto bold      {static_cast<bool>(m_style & text_style::bold)};
    const auto italic    {static_cast<bool>(m_style & text_style::italic)};
    const auto codepoints{convert_to<utf32>(string)};

    auto& font{choose_font()};

    draw_line_state state
    {
        .font = font,
        .y = static_cast<float>(font.info().max_ascent),
        .lowest_y = static_cast<float>(font.info().max_glyph_height),
        .line_width = static_cast<float>(line_width),
        .space = choose_space(),
        .texture_size = vec2f{static_cast<float>(m_atlas->texture()->width()), static_cast<float>(m_atlas->texture()->height())},
        .base_key = make_base_key(font.info().size, outline, bold, italic),
        .codepoints = codepoints
    };

    state.vertices.reserve(std::size(state.codepoints) * 4);

    if(static_cast<bool>(m_style & (text_style::underlined | text_style::strikethrough)))
    {
        state.lines.reserve(4 * 32);
    }

    for(auto&& [line, _] : split(state.codepoints, U'\n'))
    {
        draw(line, state);
    }

    state.vertices.insert(std::end(state.vertices), std::begin(state.lines), std::end(state.lines));

    state.lowest_x = std::floor(state.lowest_x);
    state.lowest_y = std::floor(state.lowest_y);
    state.greatest_x = std::ceil(state.greatest_x);
    state.greatest_y = std::ceil(state.greatest_y);

    const vec3f shift{-state.lowest_x, -state.lowest_y, 0.0f};
    for(auto& vertex : state.vertices)
    {
        vertex.position += shift;
    }

    const auto indices{generate_indices(std::size(state.vertices) / 4u)};

    const auto text_width {static_cast<std::uint32_t>(state.greatest_x - state.lowest_x)};
    const auto text_height{static_cast<std::uint32_t>(state.greatest_y - state.lowest_y)};

    return text{indices, state.vertices, m_atlas, text_bounds{text_width, text_height}};
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

    m_atlas->set_name(m_name + " atlas");
}
#endif

text_drawer::font_data<float> text_drawer::compute_spaces()
{
    font_data<float> output{};

    if(m_fonts.regular)
    {
        output.regular = compute_space(m_fonts.regular.value());
    }

    if(m_fonts.italic)
    {
        output.italic = compute_space(m_fonts.italic.value());
    }

    if(m_fonts.bold)
    {
        output.bold = compute_space(m_fonts.bold.value());
    }

    if(m_fonts.italic_bold)
    {
        output.italic_bold = compute_space(m_fonts.italic_bold.value());
    }

    return output;
}

cpt::font& text_drawer::choose_font() noexcept
{
    const auto bold  {static_cast<bool>(m_style & text_style::bold)};
    const auto italic{static_cast<bool>(m_style & text_style::italic)};

    if(bold && italic && m_fonts.italic_bold)
    {
        return m_fonts.italic_bold.value();
    }
    else if(bold && m_fonts.bold)
    {
        return m_fonts.bold.value();
    }
    else if(italic && m_fonts.italic)
    {
        return m_fonts.italic.value();
    }
    else
    {
        return m_fonts.regular.value();
    }
}

float text_drawer::choose_space() noexcept
{
    const auto bold  {static_cast<bool>(m_style & text_style::bold)};
    const auto italic{static_cast<bool>(m_style & text_style::italic)};

    if(bold && italic && m_fonts.italic_bold)
    {
        return m_spaces.italic_bold;
    }
    else if(bold && m_fonts.bold)
    {
        return m_spaces.bold;
    }
    else if(italic && m_fonts.italic)
    {
        return m_spaces.italic;
    }
    else
    {
        return m_spaces.regular;
    }
}

void text_drawer::bounds(std::u32string_view line, draw_line_state& state)
{
    switch(m_align)
    {
        case text_align::left:
            [[fallthrough]];

        case text_align::right:
            [[fallthrough]];

        case text_align::center:
            bounds_default(line, state);
            break;

        case text_align::justify:
            bounds_justify(line, state);
            break;
    }
}

void text_drawer::bounds_default(std::u32string_view line, draw_line_state& state)
{
    const bool underlined   {static_cast<bool>(m_style & text_style::underlined)};
    const bool strikethrough{static_cast<bool>(m_style & text_style::strikethrough)};

    const float line_height{state.font.info().underline_thickness};
    const float underline_y{state.font.info().underline_position};
    const float strikeout_y{state.font.info().strikeout_position};

    state.lowest_x = static_cast<float>(state.line_width);

    line_bbox_info bbox{.remainder = line};

    do
    {
        bbox = line_bbox(state.font, bbox.remainder, state.base_key, state.space, state.line_width);

        state.lowest_x = std::min(state.lowest_x, bbox.lowest_x);
        state.lowest_y = std::min(state.lowest_y, state.y + bbox.lowest_y);

        if(strikethrough)
        {
            state.lowest_y = std::min(state.lowest_y, state.y - strikeout_y - (line_height / 2.0f));
        }

        state.greatest_x = std::max(state.greatest_x, bbox.greatest_x);

        if(underlined)
        {
            state.greatest_y = std::max(state.greatest_y, state.y + bbox.greatest_y + underline_y + line_height);
        }
        else
        {
            state.greatest_y = std::max(state.greatest_y, state.y + bbox.greatest_y);
        }

        state.y += state.font.info().line_height;

    } while(!std::empty(bbox.line) && !std::empty(bbox.remainder));
}

void text_drawer::bounds_justify(std::u32string_view line, draw_line_state& state)
{
    const bool underlined   {static_cast<bool>(m_style & text_style::underlined)};
    const bool strikethrough{static_cast<bool>(m_style & text_style::strikethrough)};

    const float line_height{state.font.info().underline_thickness};
    const float underline_y{state.font.info().underline_position};
    const float strikeout_y{state.font.info().strikeout_position};

    state.lowest_x = static_cast<float>(state.line_width);

    line_bbox_info bbox{.remainder = line};

    do
    {
        bbox = line_bbox(state.font, bbox.remainder, state.base_key, state.space, state.line_width);

        state.lowest_x = std::min(state.lowest_x, bbox.lowest_x);
        state.lowest_y = std::min(state.lowest_y, state.y + bbox.lowest_y);

        if(strikethrough)
        {
            state.lowest_y = std::min(state.lowest_y, state.y - strikeout_y - (line_height / 2.0f));
        }

        if(std::empty(bbox.remainder))
        {
            state.greatest_x = std::max(state.greatest_x, bbox.greatest_x);
        }
        else
        {
            state.greatest_x = std::max(state.greatest_x, state.line_width);
        }

        if(underlined)
        {
            state.greatest_y = std::max(state.greatest_y, state.y + bbox.greatest_y + underline_y + line_height);
        }
        else
        {
            state.greatest_y = std::max(state.greatest_y, state.y + bbox.greatest_y);
        }

        state.y += state.font.info().line_height;

    } while(!std::empty(bbox.line) && !std::empty(bbox.remainder));
}

void text_drawer::draw(std::u32string_view line, draw_line_state& state)
{
    switch(m_align)
    {
        case text_align::left:
            draw_left_aligned(line, state);
            break;

        case text_align::right:
            draw_right_aligned(line, state);
            break;

        case text_align::center:
            draw_center_aligned(line, state);
            break;

        case text_align::justify:
            draw_justify_aligned(line, state);
            break;
    }
}

void text_drawer::draw_left_aligned(std::u32string_view line, draw_line_state& state)
{
    const bool underlined   {static_cast<bool>(m_style & text_style::underlined)};
    const bool strikethrough{static_cast<bool>(m_style & text_style::strikethrough)};

    codepoint_t last{};
    float line_width{};

    for(auto&& [word, _] : split(line, U' '))
    {
        const auto shift{state.x - std::floor(state.x)};
        if(state.x + word_width(state.font, word, state.base_key, last, shift).width > state.line_width)
        {
            state.x = 0.0f;

            if(underlined)
            {
                add_underline(line_width, state);
            }

            if(strikethrough)
            {
                add_strikeline(line_width, state);
            }

            state.y += state.font.info().line_height;

            last = 0;
            line_width = 0.0f;
        }

        for(const auto codepoint : word)
        {
            const vec2f kerning{get_kerning(state.font, last, codepoint)};
            const auto  key    {combine_keys(state.base_key, codepoint, adjust(m_adjustment, state.x + kerning.x()))};
            const auto& glyph  {load(state.font, key)};

            const vec2f texpos{static_cast<float>(glyph.rect.x), static_cast<float>(glyph.rect.y)};
            const float width {static_cast<float>(glyph.flipped ? glyph.rect.height : glyph.rect.width)};
            const float height{static_cast<float>(glyph.flipped ? glyph.rect.width : glyph.rect.height)};

            if(width > 0.0f)
            {
                const float x_padding{last != 0 ? glyph.origin.x() + kerning.x() : 0.0f};

                const float x{state.x + x_padding};
                const float y{state.y + glyph.origin.y() + kerning.y()};

                add_glyph(state.vertices, std::floor(x), y, width, height, m_color, texpos, state.texture_size, glyph.flipped);

                state.lowest_x = std::min(state.lowest_x, x);
                state.lowest_y = std::min(state.lowest_y, y);
                state.greatest_x = std::max(state.greatest_x, x + width);
                state.greatest_y = std::max(state.greatest_y, y + height);

                line_width = std::max(line_width, x + width);
            }

            state.x += glyph.advance;
            last = codepoint;
        }

        state.x += state.space;
        last = U' ';
    }

    state.x = 0.0f;

    if(underlined)
    {
        add_underline(line_width, state);
    }

    if(strikethrough)
    {
        add_strikeline(line_width, state);
    }

    state.y += state.font.info().line_height;
}

void text_drawer::draw_right_aligned(std::u32string_view line, draw_line_state& state)
{
    const bool underlined   {static_cast<bool>(m_style & text_style::underlined)};
    const bool strikethrough{static_cast<bool>(m_style & text_style::strikethrough)};

    state.lowest_x = state.line_width;
    state.greatest_x = state.line_width;

    std::size_t begin{std::size(state.vertices)};
    std::size_t count{};
    float lowest_x{};
    float greatest_x{};
    codepoint_t last{};

    auto do_shift = [this, &state, underlined, strikethrough](std::size_t begin, std::size_t count, float lowest_x, float greatest_x) mutable
    {
        const float width{std::floor(greatest_x - lowest_x)};
        const float shift{state.line_width - width};

        for(auto& vertex : std::span{std::begin(state.vertices) + begin, count * 4})
        {
            vertex.position.x() += shift;
        }

        state.x = shift;

        if(underlined)
        {
            add_underline(width, state);
        }

        if(strikethrough)
        {
            add_strikeline(width, state);
        }

        state.lowest_x = std::min(state.lowest_x, shift);

        state.x = 0.0f;
        state.y += state.font.info().line_height;
    };

    for(auto&& [word, _] : split(line, U' '))
    {
        const auto shift{state.x - std::floor(state.x)};
        if(state.x + word_width(state.font, word, state.base_key, last, shift).width > state.line_width)
        {
            do_shift(begin, count, lowest_x, greatest_x);

            begin = std::size(state.vertices);
            count = 0;
            lowest_x = 0.0f;
            greatest_x = 0.0f;
            last = 0;
        }

        for(const auto codepoint : word)
        {
            const vec2f kerning{get_kerning(state.font, last, codepoint)};
            const auto  key    {combine_keys(state.base_key, codepoint, adjust(m_adjustment, state.x + kerning.x()))};
            const auto& glyph  {load(state.font, key)};

            const vec2f texpos{static_cast<float>(glyph.rect.x), static_cast<float>(glyph.rect.y)};
            const float width {static_cast<float>(glyph.flipped ? glyph.rect.height : glyph.rect.width)};
            const float height{static_cast<float>(glyph.flipped ? glyph.rect.width : glyph.rect.height)};

            if(width > 0.0f)
            {
                const float x_padding{last != 0 ? glyph.origin.x() + kerning.x() : 0.0f};

                const float x{state.x + x_padding};
                const float y{state.y + glyph.origin.y() + kerning.y()};

                add_glyph(state.vertices, std::floor(x), y, width, height, m_color, texpos, state.texture_size, glyph.flipped);

                lowest_x = std::min(lowest_x, x);
                greatest_x = std::max(greatest_x, x + width);

                state.lowest_y = std::min(state.lowest_y, y);
                state.greatest_y = std::max(state.greatest_y, y + height);
            }

            state.x += glyph.advance;
            count += 1;
            last = codepoint;
        }

        state.x += state.space;
        count += 1;
        last = U' ';
    }

    do_shift(begin, count, lowest_x, greatest_x);
}

void text_drawer::draw_center_aligned(std::u32string_view line, draw_line_state& state)
{
    const bool underlined   {static_cast<bool>(m_style & text_style::underlined)};
    const bool strikethrough{static_cast<bool>(m_style & text_style::strikethrough)};

    state.lowest_x = static_cast<float>(state.line_width);

    codepoint_t last{};
    line_width_info line_info{.remainder = line};

    do
    {
        line_info = line_width(state.font, line_info.remainder, state.base_key, state.space, state.line_width);

        const float base_x{(state.line_width - line_info.width) / 2.0f};

        state.x = base_x;

        for(auto&& [word, _] : split(line_info.line, U' '))
        {
            for(const auto codepoint : word)
            {
                const vec2f kerning{get_kerning(state.font, last, codepoint)};
                const auto  key    {combine_keys(state.base_key, codepoint, adjust(m_adjustment, state.x + kerning.x()))};
                const auto& glyph  {load(state.font, key)};

                const vec2f texpos{static_cast<float>(glyph.rect.x), static_cast<float>(glyph.rect.y)};
                const float width {static_cast<float>(glyph.flipped ? glyph.rect.height : glyph.rect.width)};
                const float height{static_cast<float>(glyph.flipped ? glyph.rect.width : glyph.rect.height)};

                if(width > 0.0f)
                {
                    const float x_padding{last != 0 ? glyph.origin.x() + kerning.x() : 0.0f};

                    const float x{state.x + x_padding};
                    const float y{state.y + glyph.origin.y() + kerning.y()};

                    add_glyph(state.vertices, std::floor(x), y, width, height, m_color, texpos, state.texture_size, glyph.flipped);

                    state.lowest_x = std::min(state.lowest_x, x);
                    state.lowest_y = std::min(state.lowest_y, y);
                    state.greatest_x = std::max(state.greatest_x, x + width);
                    state.greatest_y = std::max(state.greatest_y, y + height);
                }

                state.x += glyph.advance;
                last = codepoint;
            }

            state.x += state.space;
            last = U' ';
        }

        state.x = base_x;

        if(underlined)
        {
            add_underline(line_info.width, state);
        }

        if(strikethrough)
        {
            add_strikeline(line_info.width, state);
        }

        state.y += state.font.info().line_height;
        last = 0;

    } while(!std::empty(line_info.line) && !std::empty(line_info.remainder));

    state.x = 0.0f;
}

void text_drawer::draw_justify_aligned(std::u32string_view line, draw_line_state& state)
{
    const bool underlined   {static_cast<bool>(m_style & text_style::underlined)};
    const bool strikethrough{static_cast<bool>(m_style & text_style::strikethrough)};

    state.lowest_x = static_cast<float>(state.line_width);

    codepoint_t last{};
    line_width_info line_info{.remainder = line};

    const auto adjust_space = [&line_info, &state]
    {
        if(std::empty(line_info.remainder))
        {
            return state.space;
        }

        const float word_count{static_cast<float>(std::count(std::begin(line_info.line), std::end(line_info.line), U' '))};

        return (state.line_width - line_info.width + state.space * word_count) / word_count;
    };

    do
    {
        line_info = line_width(state.font, line_info.remainder, state.base_key, state.space, state.line_width);

        for(auto&& [word, _] : split(line_info.line, U' '))
        {
            for(const auto codepoint : word)
            {
                const vec2f kerning{get_kerning(state.font, last, codepoint)};
                const auto  key    {combine_keys(state.base_key, codepoint, adjust(m_adjustment, state.x + kerning.x()))};
                const auto& glyph  {load(state.font, key)};

                const vec2f texpos{static_cast<float>(glyph.rect.x), static_cast<float>(glyph.rect.y)};
                const float width {static_cast<float>(glyph.flipped ? glyph.rect.height : glyph.rect.width)};
                const float height{static_cast<float>(glyph.flipped ? glyph.rect.width : glyph.rect.height)};

                if(width > 0.0f)
                {
                    const float x_padding{last != 0 ? glyph.origin.x() + kerning.x() : 0.0f};

                    const float x{state.x + x_padding};
                    const float y{state.y + glyph.origin.y() + kerning.y()};

                    add_glyph(state.vertices, std::floor(x), y, width, height, m_color, texpos, state.texture_size, glyph.flipped);

                    state.lowest_x = std::min(state.lowest_x, x);
                    state.lowest_y = std::min(state.lowest_y, y);
                    state.greatest_x = std::max(state.greatest_x, x + width);
                    state.greatest_y = std::max(state.greatest_y, y + height);
                }

                state.x += glyph.advance;
                last = codepoint;
            }

            state.x += adjust_space();
            last = U' ';
        }

        state.x = 0.0f;

        if(underlined)
        {
            const float real_width{std::empty(line_info.remainder) ? line_info.width : state.line_width};
            add_underline(real_width, state);
        }

        if(strikethrough)
        {
            const float real_width{std::empty(line_info.remainder) ? line_info.width : state.line_width};
            add_strikeline(real_width, state);
        }

        state.y += state.font.info().line_height;
        last = 0;

    } while(!std::empty(line_info.line) && !std::empty(line_info.remainder));
}

void text_drawer::add_underline(float line_width, draw_line_state& state)
{
    const float line_height{state.font.info().underline_thickness};
    const float underline_y{state.font.info().underline_position};

    const float y    {state.y + underline_y};
    const auto& glyph{load_line_filler(state.font, state.base_key, y)};

    const vec2f texpos{static_cast<float>(glyph.rect.x), static_cast<float>(glyph.rect.y)};
    const float height{static_cast<float>(glyph.flipped ? glyph.rect.width : glyph.rect.height)};

    add_line(state.lines, std::floor(state.x), std::floor(y), line_width, height, m_underline_color, texpos, state.texture_size, glyph.flipped);

    state.greatest_y = std::max(state.greatest_y, state.y + underline_y + line_height);
}

void text_drawer::add_strikeline(float line_width, draw_line_state& state)
{
    const float line_height{state.font.info().underline_thickness};
    const float strikeout_y{state.font.info().strikeout_position};

    const float y    {state.y - strikeout_y - (line_height / 2.0f)};
    const auto& glyph{load_line_filler(state.font, state.base_key, y)};

    const vec2f texpos{static_cast<float>(glyph.rect.x), static_cast<float>(glyph.rect.y)};
    const float height{static_cast<float>(glyph.flipped ? glyph.rect.width : glyph.rect.height)};

    add_line(state.lines, std::floor(state.x), std::floor(y), line_width, height, m_color, texpos, state.texture_size, glyph.flipped);

    state.lowest_y = std::min(state.lowest_y, y);
}

const text_drawer::glyph_info& text_drawer::load(cpt::font& font, std::uint64_t key, bool deferred)
{
    const auto codepoint{static_cast<codepoint_t>(key & 0x00FFFFFFu)};

    const auto outline{(key >> 40u) & 0xFFFFu};
    const auto adjust {(key >> 56u) & 0x3Fu};
    const auto bold   {(key >> 62u) & 0x01u};
    const auto italic {(key >> 63u) & 0x01u};

    const auto need_embolden{!static_cast<bool>(font.info().category & font_category::bold)   && bold};
    const auto need_italic  {!static_cast<bool>(font.info().category & font_category::italic) && italic};

    const auto lean{need_italic ? 0.2f : 0.0f};

    const auto it{m_glyphs.find(key)};
    if(it == std::end(m_glyphs))
    {
        if(!font.has(codepoint))
        {
            //Load the fallback in case the requested codepoint does not have a glyph inside the font
            if(codepoint != m_fallback)
            {
                return load(font, make_key(m_fallback, font.info().size, outline, adjust, bold, italic), deferred);
            }
            else
            {
                throw std::runtime_error{"Can not render text, '" + convert_to<narrow>(std::u32string_view{&codepoint, 1}) + "' is not available nor is '" + convert_to<narrow>(std::u32string_view{&m_fallback, 1}) + "'"};
            }
        }

        glyph_info info{};

        if(deferred)
        {
            const auto glyph{font.load_no_render(codepoint, need_embolden, static_cast<float>(outline) / 64.0f, lean, static_cast<float>(adjust) / 64.0f)};

            info.origin = glyph->origin;
            info.advance = glyph->advance;
            info.rect.width = glyph->width;
            info.rect.height = glyph->height;
            info.deferred = true;
        }
        else
        {
            const auto glyph{font.load(codepoint, m_format, need_embolden, static_cast<float>(outline) / 64.0f, lean, static_cast<float>(adjust) / 64.0f)};

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
        }

        return m_glyphs.emplace(key, info).first->second;
    }

    if(!deferred && it->second.deferred)
    {
        const auto glyph{font.load_render(codepoint, m_format, need_embolden, static_cast<float>(outline) / 64.0f, lean, static_cast<float>(adjust) / 64.0f)};

        if(glyph->width != 0)
        {
            const auto rect{m_atlas->add_glyph(glyph->data, glyph->width, glyph->height)};

            if(!rect)
            {
                throw full_font_atlas{};
            }

            it->second.rect = rect.value();

            if(rect->width != glyph->width)
            {
                it->second.flipped = true;
            }
        }

        it->second.deferred = false;
    }

    return it->second;
}

const text_drawer::glyph_info& text_drawer::load_line_filler(cpt::font& font, std::uint64_t base_key, float shift)
{
    const auto adjustment{adjust(m_line_adjustment, shift)};

    const auto key{combine_keys(base_key, line_filler_codepoint, adjustment)};
    const auto it {m_glyphs.find(key)};

    if(it == std::end(m_glyphs))
    {
        const auto line{std::max(font.info().underline_thickness, 0.25f)};

        const auto upshift  {static_cast<float>(adjustment) / 64.0f};
        const auto fheight  {upshift + line};
        const auto downshift{fheight - std::floor(fheight)};
        const auto height   {static_cast<std::uint32_t>(std::ceil(fheight))};

        stack_memory_pool<128> pool{};
        auto glyph{make_stack_vector<std::uint8_t>(pool)};

        //Generate line image
        if(m_format == glyph_format::color)
        {
            glyph.resize(4 * height);
            std::fill(std::begin(glyph), std::end(glyph), 255);

            glyph[3] = static_cast<std::uint8_t>((1.0f - upshift) * 255.0f); //top

            if(height > 1) //bottom
            {
                glyph.back() = static_cast<std::uint8_t>(downshift * 255.0f);
            }
        }
        else
        {
            glyph.resize(height);
            std::fill(std::begin(glyph), std::end(glyph), 255);

            glyph.front() = static_cast<std::uint8_t>((1.0f - upshift) * 255.0f); //top

            if(height > 1) //bottom
            {
                glyph.back() = static_cast<std::uint8_t>(downshift * 255.0f);
            }
        }

        glyph_info info{};

        const auto rect{m_atlas->add_glyph(glyph, 1, height)};
        if(!rect)
        {
            throw full_font_atlas{};
        }

        info.rect = rect.value();

        if(rect->width != 1)
        {
            info.flipped = true;
        }

        return m_glyphs.emplace(key, info).first->second;
    }

    return it->second;
}

text_drawer::word_width_info text_drawer::word_width(cpt::font& font, std::u32string_view word, std::uint64_t base_key, codepoint_t last, float base_shift)
{
    float current_x {base_shift};
    float lowest_x  {base_shift};
    float greatest_x{base_shift};

    for(const auto codepoint : word)
    {
        const vec2f kerning{get_kerning(font, last, codepoint)};
        const auto  key    {combine_keys(base_key, codepoint, adjust(m_adjustment, current_x + kerning.x()))};
        const auto& glyph  {load(font, key, true)};

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

    return word_width_info{greatest_x - lowest_x, current_x - lowest_x};
}

text_drawer::line_width_info text_drawer::line_width(cpt::font& font, std::u32string_view line, std::uint64_t base_key, float space, float line_width)
{
    float current_x{};
    float greatest_x{};
    codepoint_t last{};

    line_width_info output{};

    for(auto&& [word, remainder] : split(line, U' '))
    {
        const float shift{current_x - std::floor(current_x)};
        const auto word_info{word_width(font, word, base_key, last, shift)};

        if(current_x + word_info.width > line_width)
        {
            break;
        }

        greatest_x = current_x + word_info.width;

        current_x += word_info.advance + space;
        last = U' ';
        output.remainder = remainder;
    }

    if(!std::empty(output.remainder))
    {
        output.line = line.substr(0, std::size(line) - std::size(output.remainder) - 1);
    }
    else
    {
        output.line = line;
    }

    output.width = greatest_x;

    return output;
}

text_drawer::word_bbox_info text_drawer::word_bbox(cpt::font& font, std::u32string_view word, std::uint64_t base_key, codepoint_t last, float base_shift)
{
    float current_x{base_shift};

    word_bbox_info output{base_shift, base_shift, base_shift, base_shift};

    for(const auto codepoint : word)
    {
        const vec2f kerning{get_kerning(font, last, codepoint)};
        const auto  key    {combine_keys(base_key, codepoint, adjust(m_adjustment, current_x + kerning.x()))};
        const auto& glyph  {load(font, key, true)};

        const float width {static_cast<float>(glyph.flipped ? glyph.rect.height : glyph.rect.width)};
        const float height{static_cast<float>(glyph.flipped ? glyph.rect.width : glyph.rect.height)};

        if(width > 0.0f)
        {
            const float x_padding{last != 0 ? glyph.origin.x() + kerning.x() : 0.0f};
            const float x{current_x + x_padding};
            const float y{glyph.origin.y() + kerning.y()};

            output.lowest_x = std::min(output.lowest_x, x);
            output.lowest_y = std::min(output.lowest_y, y);
            output.greatest_x = std::max(output.greatest_x, x + width);
            output.greatest_y = std::max(output.greatest_y, y + height);
        }

        current_x += glyph.advance;
        last = codepoint;
    }

    output.advance = current_x - output.lowest_x;

    return output;
}

text_drawer::line_bbox_info text_drawer::line_bbox(cpt::font& font, std::u32string_view line, std::uint64_t base_key, float space, float line_width)
{
    float current_x{};
    codepoint_t last{};

    line_bbox_info output{};

    for(auto&& [word, remainder] : split(line, U' '))
    {
        const float shift{current_x - std::floor(current_x)};
        const auto  bbox {word_bbox(font, word, base_key, last, shift)};
        const float width{bbox.greatest_x - bbox.lowest_x};

        if(current_x + width > line_width)
        {
            break;
        }

        //lowest_x is 0
        output.lowest_y = std::min(bbox.lowest_y, output.lowest_y);
        output.greatest_x = current_x + width;
        output.greatest_y = std::max(bbox.greatest_y, output.greatest_y);

        current_x += bbox.advance + space;

        last = U' ';
        output.remainder = remainder;
    }

    if(!std::empty(output.remainder))
    {
        output.line = line.substr(0, std::size(line) - std::size(output.remainder) - 1);
    }
    else
    {
        output.line = line;
    }

    return output;
}

vec2f text_drawer::get_kerning(cpt::font& font, codepoint_t left, codepoint_t right) const noexcept
{
    if(!static_cast<bool>(m_options & text_drawer_options::no_kerning))
    {
        return font.kerning(left, right);
    }

    return vec2f{};
}

}
