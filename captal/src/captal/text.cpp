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

static float compute_space(cpt::font& font)
{
    const auto space{font.load_no_render(' ')};
    if(space)
    {
        return space->advance;
    }

    return font.info().size / 4.0f;
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

static std::uint64_t adjust(text_subpixel_adjustment adjustment, float x) noexcept
{
    const float padding{x - std::floor(x)};
    const float step{adjustment_steps[static_cast<std::uint32_t>(adjustment)]};
    const float shift{std::round(padding / step) * step * 64.0f};

    //modulo so 64 == 0 (otherwise it break text rendering by adding 1px padding)
    return static_cast<std::uint64_t>(shift) % 64;
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

text_drawer::text_drawer(cpt::font font, text_drawer_options options, text_subpixel_adjustment adjustment, const tph::sampling_options& sampling)
:m_font{std::move(font)}
,m_sampling{sampling}
,m_options{options}
,m_adjustment{adjustment}
,m_space{compute_space(m_font)}
,m_atlas{std::make_shared<font_atlas>(m_font.info().format, m_sampling)}
{

}

void text_drawer::set_font(cpt::font font) noexcept
{
    m_font = std::move(font);
    m_space = compute_space(m_font);
    m_atlas = std::make_shared<font_atlas>(m_font.info().format, m_sampling);
    m_glyphs.clear();
}

void text_drawer::set_fallback(codepoint_t codepoint)
{
    if(!m_font.has(codepoint))
    {
        throw std::runtime_error{"Can not set fallback '" + convert_to<narrow>(std::u32string_view{&codepoint, 1}) + "'"};
    }

    m_fallback = codepoint;
}

void text_drawer::resize(uint32_t pixels_size)
{
    m_font.resize(pixels_size);
    m_space = compute_space(m_font);
}

text_bounds text_drawer::bounds(std::string_view string, std::uint32_t line_width)
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

text text_drawer::draw(std::string_view string, std::uint32_t line_width)
{
    const auto outline{static_cast<std::uint64_t>(m_outline * 64.0f)};
    const auto bold{static_cast<bool>(m_style & text_style::bold)};
    const auto italic{static_cast<bool>(m_style & text_style::italic)};

    const auto codepoints{convert_to<utf32>(string)};

    draw_line_state state
    {
        .y = static_cast<float>(m_font.info().max_ascent),
        .lowest_y = static_cast<float>(m_font.info().max_glyph_height),
        .line_width = static_cast<float>(line_width),
        .texture_size = vec2f{static_cast<float>(m_atlas->texture()->width()), static_cast<float>(m_atlas->texture()->height())},
        .base_key = make_base_key(m_font.info().size, outline, bold, italic),
        .codepoints = codepoints
    };

    state.vertices.reserve(std::size(state.codepoints) * 4 + 4);

    for(auto&& [line, _] : split(state.codepoints, U'\n'))
    {
        draw_line(line, state);

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

    const auto indices{generate_indices(std::size(state.codepoints))};
    const auto text_width{static_cast<std::uint32_t>(state.greatest_x - state.lowest_x)};
    const auto text_height{static_cast<std::uint32_t>(state.greatest_y - state.lowest_y)};

    return text{indices, state.vertices, m_atlas, m_style, text_width, text_height, std::size(state.vertices)};
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

void text_drawer::draw_line(std::u32string_view line, draw_line_state& state)
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

        default:
            std::terminate();
    }
}

void text_drawer::draw_left_aligned(std::u32string_view line, draw_line_state& state)
{
    codepoint_t last{};
    for(auto&& [word, _] : split(line, U' '))
    {
        const auto shift{state.x - std::floor(state.x)};
        if(state.x + word_width(word, state.base_key, last, shift).width > state.line_width)
        {
            state.x = 0.0f;
            state.y += m_font.info().line_height;
            last = 0;
        }

        for(const auto codepoint : word)
        {
            const vec2f kerning{m_font.kerning(last, codepoint)};
            const auto  key    {combine_keys(state.base_key, codepoint, adjust(m_adjustment, state.x + kerning.x()))};
            const auto& glyph  {load(key)};

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
            else
            {
                add_placeholder(state.vertices);
            }

            state.x += glyph.advance;
            last = codepoint;
        }

        add_placeholder(state.vertices);

        state.x += m_space;
        last = U' ';
    }

    //There is an additionnal space at the end
    state.vertices.resize(std::size(state.vertices) - 4);
}

void text_drawer::draw_right_aligned(std::u32string_view line, draw_line_state& state)
{
    state.lowest_x = state.line_width;
    state.greatest_x = state.line_width;

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
        if(state.x + word_width(word, state.base_key, last, shift).width > state.line_width)
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
            const auto  key    {combine_keys(state.base_key, codepoint, adjust(m_adjustment, state.x + kerning.x()))};
            const auto& glyph  {load(key)};

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
            else
            {
                add_placeholder(state.vertices);
            }

            state.x += glyph.advance;
            count += 1;
            last = codepoint;
        }

        add_placeholder(state.vertices);

        state.x += m_space;
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

    codepoint_t last{};
    line_width_info line_info{.remainder = line};

    do
    {
        line_info = line_width(line_info.remainder, state.base_key, state.line_width);

        state.x = (state.line_width - line_info.width) / 2.0f;

        for(auto&& [word, _] : split(line_info.line, U' '))
        {
            for(const auto codepoint : word)
            {
                const vec2f kerning{m_font.kerning(last, codepoint)};
                const auto  key    {combine_keys(state.base_key, codepoint, adjust(m_adjustment, state.x + kerning.x()))};
                const auto& glyph  {load(key)};

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
                else
                {
                    add_placeholder(state.vertices);
                }

                state.x += glyph.advance;
                last = codepoint;
            }

            add_placeholder(state.vertices);

            state.x += m_space;
            last = U' ';
        }

        state.y += m_font.info().line_height;
        last = 0;

    } while(!std::empty(line_info.line) && !std::empty(line_info.remainder));
}

void text_drawer::draw_justify_aligned(std::u32string_view line, draw_line_state& state)
{
    state.lowest_x = static_cast<float>(state.line_width);

    codepoint_t last{};
    line_width_info line_info{.remainder = line};

    do
    {
        line_info = line_width(line_info.remainder, state.base_key, state.line_width);

        state.x = 0.0f;

        const float word_count{static_cast<float>(std::count(std::begin(line_info.line), std::end(line_info.line), U' '))};
        const float adjusted_space{std::empty(line_info.remainder) ? m_space : (state.line_width - line_info.width + m_space * word_count) / word_count};

        for(auto&& [word, _] : split(line_info.line, U' '))
        {
            for(const auto codepoint : word)
            {
                const vec2f kerning{m_font.kerning(last, codepoint)};
                const auto  key    {combine_keys(state.base_key, codepoint, adjust(m_adjustment, state.x + kerning.x()))};
                const auto& glyph  {load(key)};

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
                else
                {
                    add_placeholder(state.vertices);
                }

                state.x += glyph.advance;
                last = codepoint;
            }

            add_placeholder(state.vertices);

            state.x += adjusted_space;
            last = U' ';
        }

        state.y += m_font.info().line_height;
        last = 0;

    } while(!std::empty(line_info.line) && !std::empty(line_info.remainder));
}

void text_drawer::line_bounds(std::u32string_view line, draw_line_state& state)
{
    switch(m_align)
    {
        case text_align::left:  [[fallthrough]];
        case text_align::right: [[fallthrough]];
        case text_align::center:
           default_bounds(line, state);
           break;

        case text_align::justify:
            assert(false && "cpt::text_align::justify is not supported yet");
            [[fallthrough]];

        default:
            std::terminate();
    }
}

void text_drawer::default_bounds(std::u32string_view line, draw_line_state& state)
{/*
    codepoint_t last{};
    for(auto&& [word, _] : split(line, U' '))
    {
        const auto shift{state.x - std::floor(state.x)};
        if(state.x + word_width(word, state.font_size, embolden, last, shift).width > state.line_width)
        {
            state.x = 0.0f;
            state.y += m_font.info().line_height;
            last = 0;
        }

        for(const auto codepoint : word)
        {
            const auto  key  {make_key(codepoint, state.font_size, adjust(m_adjustment, state.x), embolden, italic)};
            const auto& glyph{load(key, true)};

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

        state.x += m_space;
        last = U' ';
    }*/
}

const text_drawer::glyph_info& text_drawer::load(std::uint64_t key, bool deferred)
{
    const auto codepoint{static_cast<codepoint_t>(key & 0x00FFFFFF)};

    const auto outline{(key >> 40) & 0xFFFF};
    const auto adjust {(key >> 56) & 0x3F};
    const auto bold   {(key >> 62) & 0x02};
    const auto italic {(key >> 63) & 0x01};

    const auto need_embolden{!static_cast<bool>(m_font.info().category & font_category::bold)   && bold};
    const auto need_italic  {!static_cast<bool>(m_font.info().category & font_category::italic) && italic};

    const auto lean{need_italic ? 0.2f : 0.0f};

    const auto it{m_glyphs.find(key)};
    if(it == std::end(m_glyphs))
    {
        if(!m_font.has(codepoint))
        {
            //Load the fallback in case the requested codepoint does not have a glyph inside the font
            if(codepoint != m_fallback)
            {
                return load(make_key(m_fallback, m_font.info().size, outline, adjust, bold, italic));
            }
            else
            {
                throw std::runtime_error{"Can not render text, '" + convert_to<narrow>(std::u32string_view{&codepoint, 1}) + "' is not available nor is '" + convert_to<narrow>(std::u32string_view{&m_fallback, 1}) + "'"};
            }
        }

        glyph_info info{};

        if(deferred)
        {
            const auto glyph{m_font.load_no_render(codepoint, need_embolden, outline / 64.0f, lean, adjust / 64.0f)};

            info.origin = glyph->origin;
            info.advance = glyph->advance;
            info.rect.width = glyph->width;
            info.rect.height = glyph->height;
            info.deferred = true;
        }
        else
        {
            const auto glyph{m_font.load(codepoint, need_embolden, outline / 64.0f, lean, adjust / 64.0f)};

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
        const auto glyph{m_font.load_render(codepoint, need_embolden, outline / 64.0f, lean, adjust / 64.0f)};

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

text_drawer::word_width_info text_drawer::word_width(std::u32string_view word, std::uint64_t base_key, codepoint_t last, float base_shift)
{
    float current_x{base_shift};
    float lowest_x{base_shift};
    float greatest_x{base_shift};

    for(const auto codepoint : word)
    {
        const vec2f kerning{m_font.kerning(last, codepoint)};
        const auto  key    {combine_keys(base_key, codepoint, adjust(m_adjustment, current_x + kerning.x()))};
        const auto& glyph  {load(key, true)};

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

text_drawer::line_width_info text_drawer::line_width(std::u32string_view line, std::uint64_t base_key, float line_width)
{
    float current_x{};
    float greatest_x{};
    codepoint_t last{};
    line_width_info output{};

    for(auto&& [word, remainder] : split(line, U' '))
    {
        const float shift{current_x - std::floor(current_x)};
        const auto word_info{word_width(word, base_key, last, shift)};

        if(current_x + word_info.width > line_width)
        {
            break;
        }

        current_x += word_info.advance;
        greatest_x = current_x;

        current_x += m_space;
        last = U' ';
        output.remainder = remainder;
    }

    if(std::empty(output.remainder))
    {
        output.line = line;
    }
    else
    {
        output.line = line.substr(0, std::size(line) - std::size(output.remainder) - 1);
    }

    output.width = greatest_x;

    return output;
}

}
