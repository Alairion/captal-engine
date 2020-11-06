#include "text.hpp"

#include <cassert>
#include <algorithm>
#include <fstream>

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

text_drawer::text_drawer(cpt::font font, text_drawer_options options, const tph::sampling_options& sampling)
:m_font{std::move(font)}
,m_options{options}
,m_sampling{sampling}
{
    m_atlases.emplace_back(atlas_info{std::make_shared<font_atlas>(m_font.info().format, m_sampling)});
}

text_bounds text_drawer::bounds(std::string_view string, text_style style)
{
    auto& atlas{ensure(string, style)};

    const auto font_size{static_cast<std::uint64_t>(m_font.info().size)};

    float current_x{};
    float current_y{static_cast<float>(m_font.info().max_ascent)};
    float lowest_x{static_cast<float>(m_font.info().max_glyph_width)};
    float lowest_y{static_cast<float>(m_font.info().max_glyph_height)};
    float greatest_x{};
    float greatest_y{};
    codepoint_t last{};

    for(auto codepoint : decode<utf8>(string))
    {
        if(codepoint == U'\n')
        {
            current_x = 0;
            current_y += m_font.info().line_height;
            last = 0;
        }

        const std::uint64_t key{(font_size << 32) | codepoint};
        const glyph_info& glyph{atlas.glyphs.at(key)};

        const vec2f texpos{static_cast<float>(glyph.rect.x), static_cast<float>(glyph.rect.y)};
        const float width {static_cast<float>(glyph.flipped ? glyph.rect.height : glyph.rect.width)};
        const float height{static_cast<float>(glyph.flipped ? glyph.rect.width  : glyph.rect.height)};

        if(width > 0 && height > 0)
        {
            const vec2f kerning{m_font.kerning(last, codepoint)};
            const float x{current_x + glyph.origin.x() + kerning.x()};
            const float y{current_y + glyph.origin.y() + kerning.y()};

            lowest_x = std::min(lowest_x, x);
            lowest_y = std::min(lowest_y, y);
            greatest_x = std::max(greatest_x, x + width);
            greatest_y = std::max(greatest_y, y + height);
        }

        current_x += glyph.advance;
        last = codepoint;
    }

    const auto text_width{static_cast<std::uint32_t>(greatest_x - lowest_x)};
    const auto text_height{static_cast<std::uint32_t>(greatest_y - lowest_y)};

    return text_bounds{text_width, text_height};
}
/*
text_bounds text_drawer::bounds(std::string_view string, std::uint32_t line_width, text_align align)
{

}
*/
text text_drawer::draw(std::string_view string, text_style style, const color& color)
{
    auto& atlas{ensure(string, style)};

    const auto  texture_width{static_cast<float>(atlas.atlas->texture()->width())};
    const auto  texture_height{static_cast<float>(atlas.atlas->texture()->height())};
    const vec2f texsize{texture_width, texture_height};
    const auto  codepoint_count{utf8::count(std::begin(string), std::end(string))};
    const auto  font_size{static_cast<std::uint64_t>(m_font.info().size)};

    std::vector<vertex> vertices{};
    vertices.reserve(codepoint_count * 4);

    float current_x{};
    float current_y{static_cast<float>(m_font.info().max_ascent)};
    float lowest_x{static_cast<float>(m_font.info().max_glyph_width)};
    float lowest_y{static_cast<float>(m_font.info().max_glyph_height)};
    float greatest_x{};
    float greatest_y{};
    codepoint_t last{};

    for(auto codepoint : decode<utf8>(string))
    {
        if(codepoint == U'\n')
        {
            current_x = 0.0f;
            current_y += m_font.info().line_height;
            last = 0;

            add_placeholder(vertices);
        }
        else
        {
            const std::uint64_t key{(font_size << 32) | codepoint};
            const glyph_info& glyph{atlas.glyphs.at(key)};

            const vec2f texpos{static_cast<float>(glyph.rect.x), static_cast<float>(glyph.rect.y)};
            const float width {static_cast<float>(glyph.flipped ? glyph.rect.height : glyph.rect.width)};
            const float height{static_cast<float>(glyph.flipped ? glyph.rect.width : glyph.rect.height)};

            if(width > 0 && height > 0)
            {
                const vec2f kerning{m_font.kerning(last, codepoint)};
                const float x{current_x + glyph.origin.x() + kerning.x()};
                const float y{current_y + glyph.origin.y() + kerning.y()};

                add_glyph(vertices, x, y, width, height, static_cast<vec4f>(color), texpos, texsize, glyph.flipped);

                lowest_x = std::min(lowest_x, x);
                lowest_y = std::min(lowest_y, y);
                greatest_x = std::max(greatest_x, x + width);
                greatest_y = std::max(greatest_y, y + height);
            }
            else
            {
                add_placeholder(vertices);
            }

            current_x += glyph.advance;
            last = codepoint;
        }
    }

    const vec3f shift{-lowest_x, -lowest_y, 0.0f};
    for(auto& vertex : vertices)
    {
        vertex.position += shift;
    }

    const auto indices{generate_indices(codepoint_count)};
    const auto text_width{static_cast<std::uint32_t>(greatest_x - lowest_x)};
    const auto text_height{static_cast<std::uint32_t>(greatest_y - lowest_y)};

    return text{indices, vertices, atlas.atlas, style, text_width, text_height, std::size(string)};
}

text text_drawer::draw(std::string_view string, std::uint32_t line_width, text_align align, text_style style, const color& color)
{
    auto& atlas{ensure(string, style)};

    if(!load(atlas, U' ', m_font.info().size, false)) //ensure that the space glyph is loaded (in case the string doesn't contains any)
    {
        throw std::runtime_error{"How did you find a font without the space character ?"};
    }

    const auto texture_width{static_cast<float>(atlas.atlas->texture()->width())};
    const auto texture_height{static_cast<float>(atlas.atlas->texture()->height())};
    const auto codepoint_count{utf8::count(std::begin(string), std::end(string))};

    std::vector<vertex> vertices{};
    vertices.reserve(codepoint_count * 4);

    draw_line_state state{};
    state.current_y = static_cast<float>(m_font.info().max_ascent);
    state.lowest_x = static_cast<float>(m_font.info().max_glyph_width);
    state.lowest_y = static_cast<float>(m_font.info().max_glyph_height);
    state.texture_size = vec2f{texture_width, texture_height};
    state.line_width = line_width;
    state.font_size = m_font.info().size;

    for(auto&& line : split(string, '\n'))
    {
        draw_line(atlas, line, align, state, vertices, color);

        state.current_x = 0.0f;
        state.current_y += m_font.info().line_height;

        add_placeholder(vertices);
    }

    //There is an additionnal newline at the end
    vertices.resize(std::size(vertices) - 4);

    const vec3f shift{-state.lowest_x, -state.lowest_y, 0.0f};
    for(auto& vertex : vertices)
    {
        vertex.position += shift;
    }

    const auto indices{generate_indices(codepoint_count)};
    const auto text_width{static_cast<std::uint32_t>(state.greatest_x - state.lowest_x)};
    const auto text_height{static_cast<std::uint32_t>(state.greatest_y - state.lowest_y)};

    return text{indices, vertices, atlas.atlas, style, text_width, text_height, std::size(string)};
}

void text_drawer::upload()
{
    for(auto& atlas : m_atlases)
    {
        if(atlas.atlas->need_upload())
        {
            atlas.atlas->upload();
        }
    }
}

void text_drawer::draw_line(atlas_info& atlas, std::string_view line, text_align align, draw_line_state& state, std::vector<vertex>& vertices, const color& color)
{
    if(align == text_align::left)
    {
        draw_left_aligned(atlas, line, state, vertices, color);
    }
    else
    {
        assert(false && "only cpt::text_align::left is supported yet");
    }
}

void text_drawer::draw_left_aligned(atlas_info& atlas, std::string_view line, draw_line_state& state, std::vector<vertex>& vertices, const color& color)
{
    const std::uint64_t space_key{(state.font_size << 32) | U' '};
    const glyph_info& space_glyph{atlas.glyphs.at(space_key)};

    for(auto word : split(line, ' '))
    {
        codepoint_t last{};

        float word_advance{};
        for(auto codepoint : decode<utf8>(word))
        {
            const std::uint64_t key{(state.font_size << 32) | codepoint};
            const glyph_info& glyph{atlas.glyphs.at(key)};

            word_advance += glyph.advance;
        }

        if(static_cast<std::uint32_t>(state.current_x + word_advance) > state.line_width)
        {
            state.current_x = 0.0f;
            state.current_y += m_font.info().line_height;
            last = 0;
        }

        for(auto codepoint : decode<utf8>(word))
        {
            const std::uint64_t key{(state.font_size << 32) | codepoint};
            const glyph_info& glyph{atlas.glyphs.at(key)};

            const vec2f texpos{static_cast<float>(glyph.rect.x), static_cast<float>(glyph.rect.y)};
            const float width {static_cast<float>(glyph.flipped ? glyph.rect.height : glyph.rect.width)};
            const float height{static_cast<float>(glyph.flipped ? glyph.rect.width : glyph.rect.height)};

            if(width > 0 && height > 0)
            {
                const vec2f kerning{m_font.kerning(last, codepoint)};
                const float x{state.current_x + glyph.origin.x() + kerning.x()};
                const float y{state.current_y + glyph.origin.y() + kerning.y()};

                add_glyph(vertices, x, y, width, height, static_cast<vec4f>(color), texpos, state.texture_size, glyph.flipped);

                state.lowest_x = std::min(state.lowest_x, x);
                state.lowest_y = std::min(state.lowest_y, y);
                state.greatest_x = std::max(state.greatest_x, x + width);
                state.greatest_y = std::max(state.greatest_y, y + height);
            }
            else
            {
                add_placeholder(vertices);
            }

            state.current_x += glyph.advance;
            last = codepoint;
        }

        add_placeholder(vertices);

        state.current_x += space_glyph.advance;
    }

    //There is an additionnal space at the end
    vertices.resize(std::size(vertices) - 4);
}

text_drawer::atlas_info& text_drawer::ensure(std::string_view string, text_style style)
{
    std::u32string codepoints{convert_to<utf32>(string)};
    std::sort(std::begin(codepoints), std::end(codepoints));
    codepoints.erase(std::unique(std::begin(codepoints), std::end(codepoints)), std::end(codepoints));

    const auto font_size{static_cast<std::uint64_t>(m_font.info().size)};
    const bool need_embolden{!static_cast<bool>(m_font.info().category & font_category::bold) && static_cast<bool>(style & text_style::bold)};

    for(auto& atlas : m_atlases)
    {
        std::size_t available{};

        for(auto codepoint : codepoints)
        {
            if(!load(atlas, codepoint, font_size, need_embolden))
            {
                break;
            }

            ++available;
        }

        if(available == std::size(codepoints))
        {
            return atlas;
        }
    }

    auto& atlas{m_atlases.emplace_back(atlas_info{std::make_shared<font_atlas>(m_font.info().format, m_sampling)})};

    for(auto codepoint : codepoints)
    {
        if(!load(atlas, codepoint, font_size, need_embolden))
        {
            throw std::runtime_error{"The text can not be rendered, no font atlas can hold every glyphs, try to split up the text."};
        }
    }

    return atlas;
}

bool text_drawer::load(atlas_info& atlas, codepoint_t codepoint, std::uint64_t font_size, bool embolden, bool fallback)
{
    const std::uint64_t key{(font_size << 32) | codepoint};

    if(atlas.glyphs.find(key) == std::end(atlas.glyphs))
    {
        const auto glyph{m_font.load_image(codepoint, embolden)};
        if(glyph)
        {
            glyph_info info{};
            info.origin = glyph->origin;
            info.advance = glyph->advance;
            info.ascent = glyph->ascent;
            info.descent = glyph->descent;

            if(glyph->width != 0 && glyph->height != 0)
            {
                const auto rect{atlas.atlas->add_glyph(glyph->data, glyph->width, glyph->height)};
                if(!rect)
                {
                    return false;
                }

                info.rect = rect.value();

                if(rect->width != glyph->width)
                {
                    info.flipped = true;
                }
            }

            atlas.glyphs.emplace(key, info);
        }
        else if(fallback)
        {
            return load(atlas, m_fallback, font_size, embolden, false);
        }
        else
        {
            return false;
        }
    }

    return true;
}
/*
text draw_text(cpt::font& font, std::string_view string, const color& color, text_drawer_options options)
{
    text_drawer drawer{std::move(font), options};
    text text{drawer.draw(string, color)};

    font = drawer.drain_font();

    return text;
}

text draw_text(cpt::font&& font, std::string_view string, const color& color, text_drawer_options options)
{
    text_drawer drawer{std::move(font), options};
    text text{drawer.draw(string, color)};

    font = drawer.drain_font();

    return text;
}

text draw_text(cpt::font& font, std::string_view string, std::uint32_t line_width, text_align align, const color& color, text_drawer_options options)
{
    text_drawer drawer{std::move(font), options};
    text text{drawer.draw(string, line_width, align, color)};

    font = drawer.drain_font();

    return text;
}

text draw_text(cpt::font&& font, std::string_view string, std::uint32_t line_width, text_align align, const color& color, text_drawer_options options)
{
    text_drawer drawer{std::move(font), options};
    text text{drawer.draw(string, line_width, align, color)};

    font = drawer.drain_font();

    return text;
}
*/
}
