#include "text.hpp"

#include <cassert>
#include <algorithm>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H

#include <glm/vec2.hpp>

#include "engine.hpp"
#include "algorithm.hpp"
#include "encoding.hpp"
#include "texture.hpp"

namespace cpt
{

struct font::freetype_info
{
    FT_Library library{};
    FT_Face face{};
};

void font::freetype_deleter::operator()(freetype_info* ptr) noexcept
{
    if(ptr->face)
        FT_Done_Face(ptr->face);
    if(ptr->library)
        FT_Done_FreeType(ptr->library);
}

font::font(std::string_view data, load_from_memory_t, std::uint32_t initial_size)
:m_loader{new freetype_info{}, freetype_deleter{}}
{
    if(FT_Init_FreeType(&m_loader->library))
        throw std::runtime_error{"Can not init freetype library."};
    if(FT_New_Memory_Face(m_loader->library, reinterpret_cast<const FT_Byte*>(std::data(data)), static_cast<FT_Long>(std::size(data)), 0, &m_loader->face))
        throw std::runtime_error{"Can not init freetype font face."};
    if(FT_Select_Charmap(m_loader->face, FT_ENCODING_UNICODE))
        throw std::runtime_error{"Can not set font charmap."};

    m_info.family = std::string{m_loader->face->family_name};
    m_info.glyph_count = m_loader->face->num_glyphs;
    m_info.style = font_style::regular;

    resize(initial_size);
}

font::font(std::string_view file, load_from_file_t, std::uint32_t initial_size)
:m_loader{new freetype_info{}, freetype_deleter{}}
{
    const std::string cfile{file};

    if(FT_Init_FreeType(&m_loader->library))
        throw std::runtime_error{"Can not init freetype library."};
    if(FT_New_Face(m_loader->library, std::data(cfile), 0, &m_loader->face))
        throw std::runtime_error{"Can not init freetype font face."};
    if(FT_Select_Charmap(m_loader->face, FT_ENCODING_UNICODE))
        throw std::runtime_error{"Can not set font charmap."};

    m_info.family = std::string{m_loader->face->family_name};
    m_info.glyph_count = m_loader->face->num_glyphs;
    m_info.style = font_style::regular;

    resize(initial_size);
}

void font::set_style(font_style style) noexcept
{
    m_info.style = style;
}

void font::resize(std::uint32_t pixels_size)
{
    if(pixels_size != m_info.size)
    {
        if(FT_Set_Pixel_Sizes(m_loader->face, 0, pixels_size))
            throw std::runtime_error{"Can not set font size."};

        m_info.size = pixels_size;
        m_info.line_height = m_loader->face->size->metrics.height / 64.0f;
        m_info.max_glyph_width = FT_MulFix(m_loader->face->bbox.xMax - m_loader->face->bbox.xMin, m_loader->face->size->metrics.x_scale) / 64 + 1;
        m_info.max_glyph_height = FT_MulFix(m_loader->face->bbox.yMax - m_loader->face->bbox.yMin, m_loader->face->size->metrics.y_scale) / 64 + 1;
        m_info.max_ascent = FT_MulFix(m_loader->face->ascender, m_loader->face->size->metrics.y_scale) / 64 + 1;
        m_info.underline_position = -FT_MulFix(m_loader->face->underline_position, m_loader->face->size->metrics.y_scale) / 64.0f;
        m_info.underline_thickness = -FT_MulFix(m_loader->face->underline_thickness, m_loader->face->size->metrics.y_scale) / 64.0f;
    }
}

std::optional<glyph> font::load(char32_t codepoint)
{
    if(FT_Load_Char(m_loader->face, codepoint, FT_HAS_COLOR(m_loader->face) ? FT_LOAD_COLOR : FT_LOAD_DEFAULT))
       return std::nullopt;

    if(m_loader->face->glyph->format == FT_GLYPH_FORMAT_OUTLINE && static_cast<bool>(m_info.style & font_style::bold))
        FT_Outline_Embolden(&m_loader->face->glyph->outline, 64);

    if(FT_Render_Glyph(m_loader->face->glyph, FT_RENDER_MODE_NORMAL))
        return std::nullopt;

    FT_Bitmap& bitmap = m_loader->face->glyph->bitmap;

    if(m_loader->face->glyph->format == FT_GLYPH_FORMAT_OUTLINE && static_cast<bool>(m_info.style & font_style::bold))
        FT_Bitmap_Embolden(m_loader->library, &bitmap, 64, 64);

    glyph output{};
    output.origin = glm::vec2{m_loader->face->glyph->metrics.horiBearingX / 64.0f, -m_loader->face->glyph->metrics.horiBearingY / 64.0f};
    output.advance = m_loader->face->glyph->metrics.horiAdvance / 64.0f;
    output.ascent = m_loader->face->glyph->metrics.horiBearingY / 64.0f;
    output.descent = (m_loader->face->glyph->metrics.height / 64.0f) - (m_loader->face->glyph->metrics.horiBearingY / 64.0f);

    if(bitmap.width > 0 && bitmap.rows > 0)
    {
        output.image = tph::image{engine::instance().renderer(), static_cast<std::uint32_t>(m_loader->face->glyph->metrics.width / 64), static_cast<std::uint32_t>(m_loader->face->glyph->metrics.height / 64), tph::image_usage::transfer_source | tph::image_usage::host_access};
        output.image.map();

        if(bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
        {
            const std::uint8_t* data{bitmap.buffer};

            for(std::size_t y{}; y < output.image.height(); ++y)
            {
                for(std::size_t x{}; x <  output.image.width(); ++x)
                {
                    if(data[x / 8] & (1 << (7 - (x % 8))))
                        output.image(x, y) = tph::pixel{255, 255, 255, 255};
                    else
                        output.image(x, y) = tph::pixel{255, 255, 255, 0};
                }

                data += bitmap.pitch;
            }
        }
        else if(bitmap.pixel_mode == FT_PIXEL_MODE_GRAY)
        {
            const std::uint8_t* data{bitmap.buffer};

            for(std::size_t y{}; y < output.image.height(); ++y)
            {
                for(std::size_t x{}; x <  output.image.width(); ++x)
                {
                    output.image(x, y) = tph::pixel{255, 255, 255, data[x]};
                }

                data += bitmap.pitch;
            }
        }
        else if(bitmap.pixel_mode == FT_PIXEL_MODE_BGRA)
        {
            const tph::pixel* pixels{reinterpret_cast<const tph::pixel*>(bitmap.buffer)};

            for(std::size_t y{}; y < output.image.height(); ++y)
            {
                for(std::size_t x{}; x <  output.image.width(); ++x)
                {
                    output.image(x, y) = tph::pixel{pixels[x].blue, pixels[x].green, pixels[x].red, pixels[x].alpha};
                }

                pixels += bitmap.pitch / 4;
            }
        }

        output.image.unmap();
    }

    return output;
}

float font::kerning(char32_t left, char32_t right)
{
    FT_Vector output{};

    if(FT_Get_Kerning(m_loader->face, FT_Get_Char_Index(m_loader->face, left), FT_Get_Char_Index(m_loader->face, right), FT_KERNING_DEFAULT, &output))
        return 0.0f;

    return output.x / (FT_IS_SCALABLE(m_loader->face) ? 64.0f : 1.0f);
}

text::text(const std::vector<std::uint16_t>& indices, const std::vector<vertex>& vertices, texture_ptr texture, std::uint32_t width, std::uint32_t height)
:renderable{static_cast<std::uint32_t>(std::size(indices)), static_cast<std::uint32_t>(std::size(vertices))}
,m_width{width}
,m_height{height}
{
    set_indices(indices);
    set_vertices(vertices);
    set_texture(std::move(texture));
}

void text::set_color(const color& color)
{
    vertex* const vertices{get_vertices()};
    const glm::vec4 native_color{static_cast<glm::vec4>(color)};

    for(std::uint32_t i{}; i < vertex_count(); ++i)
        vertices[i].color = native_color;

    update();
}

void text::set_color(std::uint32_t character_index, const color& color)
{
    vertex* const current{get_vertices() + character_index * 4};
    const glm::vec4 native_color{static_cast<glm::vec4>(color)};

    current[0].color = native_color;
    current[1].color = native_color;
    current[2].color = native_color;
    current[3].color = native_color;

    update();
}

void text::set_color(std::uint32_t first, std::uint32_t count, const color& color)
{
    vertex* const vertices{get_vertices() + first * 4};
    const glm::vec4 native_color{static_cast<glm::vec4>(color)};

    for(std::uint32_t i{}; i < count; ++i)
    {
        vertex* const current{vertices + i * 4};

        current[0].color = native_color;
        current[1].color = native_color;
        current[2].color = native_color;
        current[3].color = native_color;
    }

    update();
}

text_drawer::text_drawer(cpt::font font, text_drawer_options options)
:m_font{std::move(font)}
,m_options{options}
{

}

void text_drawer::set_font(cpt::font font) noexcept
{
    m_font = std::move(font);
    m_cache.clear();
}

void text_drawer::set_style(font_style style) noexcept
{
    m_font.set_style(style);
    m_cache.clear();
}

void text_drawer::resize(std::uint32_t pixels_size)
{
    m_font.resize(pixels_size);
    m_cache.clear();
}

text_ptr text_drawer::draw(std::string_view u8string, const color& color)
{
    std::u32string u32string{};
    u32string.reserve(utf8::count(std::begin(u8string), std::end(u8string)));

    convert<utf8, utf32>(std::begin(u8string), std::end(u8string), std::back_inserter(u32string));

    return draw(std::move(u32string), color);
}

text_ptr text_drawer::draw(std::u32string u32string, const color& color)
{
    if(static_cast<bool>(m_options & text_drawer_options::cached))
    {
        return draw_cached(u32string, static_cast<glm::vec4>(color));
    }
    else
    {
        return draw_uncached(u32string, static_cast<glm::vec4>(color));
    }
}

text_ptr text_drawer::draw_cached(std::u32string u32string, const glm::vec4& color)
{
    auto&& [command_buffer, signal] = cpt::engine::instance().begin_transfer();
    std::unordered_map<char32_t, std::pair<std::shared_ptr<glyph>, glm::vec2>> cache{};
    texture_ptr texture{make_cached_texture(u32string, cache, command_buffer)};
    const float texture_width{static_cast<float>(texture->width())};
    const float texture_height{static_cast<float>(texture->height())};

    std::vector<vertex> vertices{};
    vertices.reserve(std::size(u32string) * 4);

    float current_x{};
    float current_y{static_cast<float>(m_font.info().max_ascent)};
    float lowest_x{static_cast<float>(m_font.info().max_glyph_width)};
    float lowest_y{static_cast<float>(m_font.info().max_glyph_height)};
    float greatest_x{};
    float greatest_y{};
    char32_t last{};

    for(auto c : u32string)
    {
        if(c == U'\n')
        {
            current_x = 0;
            current_y += m_font.info().line_height;
            last = 0;
        }
        else
        {
            auto& slot{cache.at(c)};

            const glyph& glyph{*slot.first};
            const glm::vec2& texture_pos{slot.second};

            const float width {static_cast<float>(glyph.image.width())};
            const float height{static_cast<float>(glyph.image.height())};

            if(width > 0 && height > 0)
            {
                const float kerning{last != 0 && static_cast<bool>(m_options & text_drawer_options::kerning) ? m_font.kerning(last, c) : 0.0f};
                const float x{current_x + glyph.origin.x + kerning};
                const float y{current_y + glyph.origin.y};

                vertices.push_back(vertex{{x, y, 0.0f}, color, {(texture_pos.x) / texture_width, (texture_pos.y) / texture_height}});
                vertices.push_back(vertex{{x + width, y, 0.0f}, color, {(texture_pos.x + width) / texture_width, (texture_pos.y) / texture_height}});
                vertices.push_back(vertex{{x + width, y + height, 0.0f}, color, {(texture_pos.x + width) / texture_width, (texture_pos.y + height) / texture_height}});
                vertices.push_back(vertex{{x, y + height, 0.0f}, color, {(texture_pos.x) / texture_width, (texture_pos.y + height) / texture_height}});

                lowest_x = std::min(lowest_x, x);
                lowest_y = std::min(lowest_y, y);
                greatest_x = std::max(greatest_x, x + width);
                greatest_y = std::max(greatest_y, y + height);
            }
            else
            {
                vertices.push_back(vertex{});
                vertices.push_back(vertex{});
                vertices.push_back(vertex{});
                vertices.push_back(vertex{});
            }

            current_x += glyph.advance;
            last = c;
       }
    }

    tph::cmd::prepare(command_buffer, texture->get_texture(), tph::pipeline_stage::fragment_shader);

    signal.connect([cache = std::move(cache)](){});

    std::vector<std::uint16_t> indices{};
    indices.reserve(std::size(u32string) * 6);
    for(std::size_t i{}; i < std::size(u32string); ++i)
    {
        const std::size_t shift{i * 4};

        indices.push_back(static_cast<std::uint16_t>(shift + 0));
        indices.push_back(static_cast<std::uint16_t>(shift + 1));
        indices.push_back(static_cast<std::uint16_t>(shift + 2));
        indices.push_back(static_cast<std::uint16_t>(shift + 2));
        indices.push_back(static_cast<std::uint16_t>(shift + 3));
        indices.push_back(static_cast<std::uint16_t>(shift + 0));
    }

    const glm::vec3 shift{-lowest_x, -lowest_y, 0.0f};
    for(auto& vertex : vertices)
        vertex.position += shift;

    return std::make_shared<text>(indices, vertices, std::move(texture), static_cast<std::uint32_t>(greatest_x - lowest_x), static_cast<std::uint32_t>(greatest_y - lowest_y));
}

texture_ptr text_drawer::make_cached_texture(std::u32string string, std::unordered_map<char32_t, std::pair<std::shared_ptr<glyph>, glm::vec2>>& cache, tph::command_buffer& command_buffer)
{
    constexpr std::uint32_t max_texture_width{4096};

    std::sort(std::begin(string), std::end(string));
    string.erase(std::unique(std::begin(string), std::end(string)), std::end(string));

    std::uint32_t current_x{};
    std::uint32_t current_y{};
    std::uint32_t texture_width{};
    std::uint32_t texture_height{};

    for(auto c : string)
    {
        auto it{m_cache.find(c)};
        if(it == std::end(m_cache))
        {
            it = m_cache.emplace(std::make_pair(c, std::make_shared<glyph>(m_font.load(c).value_or(glyph{})))).first;
        }

        glyph& character_glyph{*it->second};

        if(current_x + character_glyph.image.width() > max_texture_width)
        {
            current_x = 0;
            current_y += texture_height;
        }

        const glm::vec2 texture_pos{static_cast<float>(current_x), static_cast<float>(current_y)};

        current_x += character_glyph.image.width();
        texture_width = std::max(current_x, texture_width);
        texture_height = std::max(static_cast<std::uint32_t>(current_y + character_glyph.image.height()), texture_height);

        cache.emplace(std::make_pair(c, std::make_pair(it->second, texture_pos)));
    }

    texture_ptr texture{make_texture(texture_width, texture_height, tph::sampling_options{}, tph::texture_usage::transfer_destination | tph::texture_usage::sampled)};

    for(auto c : string)
    {
        auto& slot{cache.at(c)};

        glyph& glyph{*slot.first};
        const glm::vec2& texture_pos{slot.second};

        if(glyph.image.width() > 0 && glyph.image.height() > 0)
        {
            tph::image_texture_copy copy_region{};
            copy_region.texture_offset.x = static_cast<std::int32_t>(texture_pos.x);
            copy_region.texture_offset.y = static_cast<std::int32_t>(texture_pos.y);
            copy_region.texture_size.width = glyph.image.width();
            copy_region.texture_size.height = glyph.image.height();

            tph::cmd::copy(command_buffer, glyph.image, texture->get_texture(), copy_region);
        }
    }

    return texture;
}

text_ptr text_drawer::draw_uncached(std::u32string u32string, const glm::vec4& color)
{
    auto&& [command_buffer, signal] = cpt::engine::instance().begin_transfer();
    std::unordered_map<char32_t, std::pair<glyph, glm::vec2>> cache{};
    texture_ptr texture{make_uncached_texture(u32string, cache, command_buffer)};
    const float texture_width{static_cast<float>(texture->width())};
    const float texture_height{static_cast<float>(texture->height())};

    std::vector<vertex> vertices{};
    vertices.reserve(std::size(u32string) * 4);

    float current_x{};
    float current_y{static_cast<float>(m_font.info().max_ascent)};
    float lowest_x{static_cast<float>(m_font.info().max_glyph_width)};
    float lowest_y{static_cast<float>(m_font.info().max_glyph_height)};
    float greatest_x{};
    float greatest_y{};
    char32_t last{};

    for(auto c : u32string)
    {
        if(c == U'\n')
        {
            current_x = 0;
            current_y += m_font.info().line_height;
            last = 0;
        }
        else
        {
            auto& slot{cache.at(c)};

            const glyph& glyph{slot.first};
            const glm::vec2& texture_pos{slot.second};

            const float width {static_cast<float>(glyph.image.width())};
            const float height{static_cast<float>(glyph.image.height())};

            if(width > 0 && height > 0)
            {
                const float kerning{last != 0 && static_cast<bool>(m_options & text_drawer_options::kerning) ? m_font.kerning(last, c) : 0.0f};
                const float x{current_x + glyph.origin.x + kerning};
                const float y{current_y + glyph.origin.y};

                vertices.push_back(vertex{{x, y, 0.0f}, color, {(texture_pos.x) / texture_width, (texture_pos.y) / texture_height}});
                vertices.push_back(vertex{{x + width, y, 0.0f}, color, {(texture_pos.x + width) / texture_width, (texture_pos.y) / texture_height}});
                vertices.push_back(vertex{{x + width, y + height, 0.0f}, color, {(texture_pos.x + width) / texture_width, (texture_pos.y + height) / texture_height}});
                vertices.push_back(vertex{{x, y + height, 0.0f}, color, {(texture_pos.x) / texture_width, (texture_pos.y + height) / texture_height}});

                lowest_x = std::min(lowest_x, x);
                lowest_y = std::min(lowest_y, y);
                greatest_x = std::max(greatest_x, x + width);
                greatest_y = std::max(greatest_y, y + height);
            }
            else
            {
                vertices.push_back(vertex{});
                vertices.push_back(vertex{});
                vertices.push_back(vertex{});
                vertices.push_back(vertex{});
            }

            current_x += glyph.advance;
            last = c;
       }
    }

    tph::cmd::prepare(command_buffer, texture->get_texture(), tph::pipeline_stage::fragment_shader);

    signal.connect([cache = std::move(cache)](){});

    std::vector<std::uint16_t> indices{};
    indices.reserve(std::size(u32string) * 6);
    for(std::size_t i{}; i < std::size(u32string); ++i)
    {
        const std::size_t shift{i * 4};

        indices.push_back(static_cast<std::uint16_t>(shift + 0));
        indices.push_back(static_cast<std::uint16_t>(shift + 1));
        indices.push_back(static_cast<std::uint16_t>(shift + 2));
        indices.push_back(static_cast<std::uint16_t>(shift + 2));
        indices.push_back(static_cast<std::uint16_t>(shift + 3));
        indices.push_back(static_cast<std::uint16_t>(shift + 0));
    }

    const glm::vec3 shift{-lowest_x, -lowest_y, 0.0f};
    for(auto& vertex : vertices)
        vertex.position += shift;

    return std::make_shared<text>(indices, vertices, std::move(texture), static_cast<std::uint32_t>(greatest_x - lowest_x), static_cast<std::uint32_t>(greatest_y - lowest_y));
}

texture_ptr text_drawer::make_uncached_texture(std::u32string string, std::unordered_map<char32_t, std::pair<glyph, glm::vec2>>& cache, tph::command_buffer& command_buffer)
{
    constexpr std::uint32_t max_texture_width{4096};

    std::sort(std::begin(string), std::end(string));
    string.erase(std::unique(std::begin(string), std::end(string)), std::end(string));

    std::uint32_t current_x{};
    std::uint32_t current_y{};
    std::uint32_t texture_width{};
    std::uint32_t texture_height{};

    for(auto c : string)
    {
        glyph character_glyph{m_font.load(c).value_or(glyph{})};

        if(current_x + character_glyph.image.width() > max_texture_width)
        {
            current_x = 0;
            current_y += texture_height;
        }

        const glm::vec2 texture_pos{static_cast<float>(current_x), static_cast<float>(current_y)};

        current_x += character_glyph.image.width();
        texture_width = std::max(current_x, texture_width);
        texture_height = std::max(static_cast<std::uint32_t>(current_y + character_glyph.image.height()), texture_height);

        cache.emplace(std::make_pair(c, std::make_pair(std::move(character_glyph), texture_pos)));
    }

    texture_ptr texture{make_texture(texture_width, texture_height, tph::sampling_options{}, tph::texture_usage::transfer_destination | tph::texture_usage::sampled)};

    for(auto c : string)
    {
        auto& slot{cache.at(c)};

        glyph& glyph{slot.first};
        const glm::vec2& texture_pos{slot.second};

        if(glyph.image.width() > 0 && glyph.image.height() > 0)
        {
            tph::image_texture_copy copy_region{};
            copy_region.texture_offset.x = static_cast<std::int32_t>(texture_pos.x);
            copy_region.texture_offset.y = static_cast<std::int32_t>(texture_pos.y);
            copy_region.texture_size.width = glyph.image.width();
            copy_region.texture_size.height = glyph.image.height();

            tph::cmd::copy(command_buffer, glyph.image, texture->get_texture(), copy_region);
        }
    }

    return texture;
}

text_ptr draw_text(cpt::font& font, std::string_view u8string, const glm::vec4& color, text_drawer_options options)
{
    text_drawer drawer{std::move(font), options};
    text_ptr text{drawer.draw(u8string, color)};

    font = std::move(drawer.font());
    return text;
}

text_ptr draw_text(cpt::font&& font, std::string_view u8string, const glm::vec4& color, text_drawer_options options)
{
    text_drawer drawer{std::move(font), options};
    text_ptr text{drawer.draw(u8string, color)};

    font = std::move(drawer.font());
    return text;
}

}
