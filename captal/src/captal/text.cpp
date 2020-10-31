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

void font_engine::freetype_deleter::operator()(void* ptr) noexcept
{
    FT_Done_FreeType(reinterpret_cast<FT_Library>(ptr));
}

font_engine::font_engine()
{
    FT_Library library{};
    if(FT_Init_FreeType(&library))
        throw std::runtime_error{"Can not init freetype library."};

    m_library = handle_type{library};
}

static constexpr tph::component_mapping red_to_alpha_mapping
{
    tph::component_swizzle::one,
    tph::component_swizzle::one,
    tph::component_swizzle::one,
    tph::component_swizzle::r,
};

constexpr auto font_atlas_usage{tph::texture_usage::sampled | tph::texture_usage::transfer_destination | tph::texture_usage::transfer_source};

font_atlas::font_atlas(glyph_format format, const tph::sampling_options& sampling)
:m_format{format}
,m_sampling{sampling}
,m_packer{512, 512}
,m_max_size{engine::instance().graphics_device().limits().max_2d_texture_size}
{
    if(m_format == glyph_format::gray)
    {
        m_texture = make_texture(512, 512, tph::texture_info{tph::texture_format::r8_unorm, font_atlas_usage, red_to_alpha_mapping}, m_sampling);
    }
    else
    {
        m_texture = make_texture(512, 512, tph::texture_info{tph::texture_format::r8g8b8a8_srgb, font_atlas_usage}, m_sampling);
    }

    m_buffers.reserve(64);
    m_buffer_data.reserve(1024 * 8);
}

std::optional<bin_packer::rect> font_atlas::add_glyph(const std::vector<std::uint8_t>& image, std::uint32_t width, std::uint32_t height)
{
    const bool has_padding{m_sampling.magnification_filter != tph::filter::nearest || m_sampling.minification_filter != tph::filter::nearest};
    const std::uint32_t padding{has_padding ? 2u : 0u};

    auto rect{m_packer.append(width + padding, height + padding)};
    while(!rect.has_value())
    {
        if(m_packer.width() * 2 > m_max_size)
        {
            return std::nullopt;
        }

        m_packer.resize(m_packer.width() * 2, m_packer.height() * 2);
        m_resized = true;

        rect = m_packer.append(width + padding, height + padding);
    }

    const auto begin{std::size(m_buffer_data)};
    m_buffer_data.resize(begin + std::size(image));
    std::copy(std::begin(image), std::end(image), std::begin(m_buffer_data) + begin);

    rect->x += padding / 2;
    rect->y += padding / 2;
    rect->width -= padding;
    rect->height -= padding;

    m_buffers.emplace_back(transfer_buffer{begin, rect.value()});

    return rect;
}

void font_atlas::upload()
{
    auto&& [buffer, signal, keeper] = engine::instance().begin_transfer();

    if(std::exchange(m_resized, false))
    {
        resize(buffer, keeper);
    }
    else
    {
        tph::cmd::transition(buffer, m_texture->get_texture(),
                             tph::resource_access::none, tph::resource_access::transfer_write,
                             tph::pipeline_stage::top_of_pipe, tph::pipeline_stage::transfer,
                             tph::texture_layout::shader_read_only_optimal, tph::texture_layout::transfer_destination_optimal);
    }

    std::vector<tph::buffer_texture_copy> copies{};
    copies.reserve(std::size(m_buffers));

    for(auto&& buffer : m_buffers)
    {
        tph::buffer_texture_copy copy{};
        copy.buffer_offset = buffer.begin;
        copy.buffer_image_width = buffer.rect.width;
        copy.buffer_image_height = buffer.rect.height;
        copy.texture_offset.x = buffer.rect.x;
        copy.texture_offset.y = buffer.rect.y;
        copy.texture_size.width = buffer.rect.width;
        copy.texture_size.height = buffer.rect.height;
        copy.texture_size.width = 1;

        copies.emplace_back(copy);
    }

    tph::buffer staging_buffer{engine::instance().renderer(), std::size(m_buffer_data), tph::buffer_usage::staging | tph::buffer_usage::transfer_source};
    std::memcpy(staging_buffer.map(), std::data(m_buffer_data), std::size(m_buffer_data));
    staging_buffer.unmap();

    tph::cmd::copy(buffer, staging_buffer, m_texture->get_texture(), copies);

    tph::cmd::transition(buffer, m_texture->get_texture(),
                         tph::resource_access::transfer_write, tph::resource_access::shader_read,
                         tph::pipeline_stage::transfer, tph::pipeline_stage::fragment_shader,
                         tph::texture_layout::transfer_destination_optimal, tph::texture_layout::shader_read_only_optimal);

    keeper.keep(m_texture);
    signal.connect([buffer = std::move(staging_buffer)](){});

    m_buffers.clear();
    m_buffer_data.clear();
}

void font_atlas::resize(tph::command_buffer& buffer, asynchronous_resource_keeper& keeper)
{
    texture_ptr new_texture{};
    if(m_format == glyph_format::gray)
    {
        new_texture = make_texture(m_packer.width(), m_packer.height(), tph::texture_info{tph::texture_format::r8_unorm, font_atlas_usage, red_to_alpha_mapping}, m_sampling);
    }
    else
    {
        new_texture = make_texture(m_packer.width(), m_packer.height(), tph::texture_info{tph::texture_format::r8g8b8a8_srgb, font_atlas_usage}, m_sampling);
    }

    tph::cmd::transition(buffer, m_texture->get_texture(),
                         tph::resource_access::none, tph::resource_access::transfer_read,
                         tph::pipeline_stage::top_of_pipe, tph::pipeline_stage::transfer,
                         tph::texture_layout::shader_read_only_optimal, tph::texture_layout::transfer_source_optimal);

    tph::cmd::transition(buffer, new_texture->get_texture(),
                         tph::resource_access::none, tph::resource_access::transfer_write,
                         tph::pipeline_stage::top_of_pipe, tph::pipeline_stage::transfer,
                         tph::texture_layout::undefined, tph::texture_layout::transfer_destination_optimal);

    tph::cmd::copy(buffer, m_texture->get_texture(), new_texture->get_texture());

    keeper.keep(std::exchange(m_texture, new_texture));

    m_signal();
}

struct font::freetype_info
{
    FT_Face face{};
};

void font::freetype_deleter::operator()(freetype_info* ptr) noexcept
{
    if(ptr->face)
    {
        FT_Done_Face(ptr->face);
    }

    delete ptr;
}

font::font(std::span<const std::uint8_t> data, std::uint32_t initial_size)
:m_data{std::begin(data), std::end(data)}
,m_loader{new freetype_info{}}
{
    init(initial_size);
}

font::font(const std::filesystem::path& file, std::uint32_t initial_size)
:m_data{read_file<std::vector<std::uint8_t>>(file)}
,m_loader{new freetype_info{}}
{
    init(initial_size);
}

font::font(std::istream& stream, std::uint32_t initial_size)
:m_loader{new freetype_info{}}
{
    assert(stream && "Invalid stream.");

    m_data = std::vector<std::uint8_t>{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};

    init(initial_size);
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
        m_info.strikeout_position = m_info.max_ascent / 3;
    }
}

std::optional<glyph> font::load(codepoint_t codepoint)
{
    if(FT_Load_Char(m_loader->face, codepoint, FT_HAS_COLOR(m_loader->face) ? FT_LOAD_COLOR : FT_LOAD_DEFAULT))
    {
        return std::nullopt;
    }

    if(m_loader->face->glyph->format == FT_GLYPH_FORMAT_OUTLINE && static_cast<bool>(m_info.style & font_style::bold))
    {
        FT_Outline_Embolden(&m_loader->face->glyph->outline, 64);
    }

    if(FT_Render_Glyph(m_loader->face->glyph, FT_RENDER_MODE_NORMAL))
    {
        return std::nullopt;
    }

    if(m_loader->face->glyph->format == FT_GLYPH_FORMAT_OUTLINE && static_cast<bool>(m_info.style & font_style::bold))
    {
        FT_Bitmap_Embolden(m_loader->library, &m_loader->face->glyph->bitmap, 64, 64);
    }

    const FT_Bitmap& bitmap{m_loader->face->glyph->bitmap};

    glyph output{};
    output.origin = vec2f{m_loader->face->glyph->metrics.horiBearingX / 64.0f, -m_loader->face->glyph->metrics.horiBearingY / 64.0f};
    output.advance = m_loader->face->glyph->metrics.horiAdvance / 64.0f;
    output.ascent = m_loader->face->glyph->metrics.horiBearingY / 64.0f;
    output.descent = (m_loader->face->glyph->metrics.height / 64.0f) - (m_loader->face->glyph->metrics.horiBearingY / 64.0f);
    output.width = bitmap.width;
    output.height = bitmap.rows;

    if(output.width > 0 && output.height > 0)
    {
        const std::uint8_t* data{bitmap.buffer};

        if(bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
        {
            output.data.resize(output.height * output.width);
            output.format = glyph_format::gray;

            for(std::size_t y{}; y < output.height; ++y)
            {
                for(std::size_t x{}; x < output.width; ++x)
                {
                    const auto shift{7u - x % 8u};
                    const auto bit{(data[x / 8u] & (1u << shift)) >> shift};

                    output.data[y * bitmap.width + x] = static_cast<std::uint8_t>(bit * 255u);
                }

                data += bitmap.pitch;
            }
        }
        else if(bitmap.pixel_mode == FT_PIXEL_MODE_GRAY)
        {
            output.data.resize(output.height * output.width);
            output.format = glyph_format::gray;

            for(std::size_t y{}; y < output.height; ++y)
            {
                for(std::size_t x{}; x < output.width; ++x)
                {
                    output.data[y * bitmap.width + x] = data[x];
                }

                data += bitmap.pitch;
            }
        }
        else if(bitmap.pixel_mode == FT_PIXEL_MODE_BGRA)
        {
            output.data.resize(output.height * output.width * 4);
            output.format = glyph_format::color;

            for(std::size_t y{}; y < output.height; ++y)
            {
                for(std::size_t x{}; x < output.width; ++x)
                {
                    output.data[y * bitmap.width + x + 0] = data[x + 2];
                    output.data[y * bitmap.width + x + 1] = data[x + 1];
                    output.data[y * bitmap.width + x + 2] = data[x + 0];
                    output.data[y * bitmap.width + x + 3] = data[x + 3];
                }

                data += bitmap.pitch;
            }
        }
    }

    return std::make_optional(std::move(output));
}

float font::kerning(codepoint_t left, codepoint_t right)
{
    FT_Vector output{};

    if(FT_Get_Kerning(m_loader->face, FT_Get_Char_Index(m_loader->face, left), FT_Get_Char_Index(m_loader->face, right), FT_KERNING_DEFAULT, &output))
        return 0.0f;

    return output.x / (FT_IS_SCALABLE(m_loader->face) ? 64.0f : 1.0f);
}

void font::init(std::uint32_t initial_size)
{
    if(FT_New_Memory_Face(m_loader->library, reinterpret_cast<const FT_Byte*>(std::data(m_data)), static_cast<FT_Long>(std::size(m_data)), 0, &m_loader->face))
        throw std::runtime_error{"Can not init freetype font face."};

    if(FT_Select_Charmap(m_loader->face, FT_ENCODING_UNICODE))
        throw std::runtime_error{"Can not set font charmap."};

    m_info.family = std::string{m_loader->face->family_name};
    m_info.glyph_count = m_loader->face->num_glyphs;

    if(m_loader->face->style_flags & FT_STYLE_FLAG_BOLD)
    {
        m_info.style |= font_style::bold;
    }

    if(m_loader->face->style_flags & FT_STYLE_FLAG_ITALIC)
    {
        m_info.style |= font_style::italic;
    }

    resize(initial_size);
}

text::text(std::span<const std::uint32_t> indices, std::span<const vertex> vertices, texture_ptr texture, std::uint32_t width, std::uint32_t height, std::size_t count)
:renderable{static_cast<std::uint32_t>(std::size(indices)), static_cast<std::uint32_t>(std::size(vertices))}
,m_width{width}
,m_height{height}
,m_count{count}
{
    set_indices(indices);
    set_vertices(vertices);
    set_texture(std::move(texture));
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

text_bounds text_drawer::bounds(std::string_view string)
{
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

        const auto& glyph {*load_glyph(codepoint)};
        const float width {static_cast<float>(glyph.width)};
        const float height{static_cast<float>(glyph.height)};

        if(width > 0 && height > 0)
        {
            const float kerning{last != 0 && static_cast<bool>(m_options & text_drawer_options::kerning) ? m_font.kerning(last, codepoint) : 0.0f};
            const float x{current_x + glyph.origin.x() + kerning};
            const float y{current_y + glyph.origin.y()};

            lowest_x = std::min(lowest_x, x);
            lowest_y = std::min(lowest_y, y);
            greatest_x = std::max(greatest_x, x + width);
            greatest_y = std::max(greatest_y, y + height);
        }

        current_x += glyph.advance;
        last = codepoint;
    }

    return text_bounds{static_cast<std::uint32_t>(greatest_x - lowest_x), static_cast<std::uint32_t>(greatest_y - lowest_y)};
}

text text_drawer::draw(std::string_view string, const color& color)
{
    auto&& [command_buffer, signal, keeper] = cpt::engine::instance().begin_transfer();

    std::unordered_map<codepoint_t, std::pair<std::shared_ptr<glyph>, vec2f>> cache{};
    texture_ptr texture{make_texture(string, cache, command_buffer)};

    const float texture_width{static_cast<float>(texture->width())};
    const float texture_height{static_cast<float>(texture->height())};
    const std::size_t codepoint_count{utf8::count(std::begin(string), std::end(string))};

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

            vertices.emplace_back(vertex{});
            vertices.emplace_back(vertex{});
            vertices.emplace_back(vertex{});
            vertices.emplace_back(vertex{});
        }
        else
        {
            auto& slot{cache.at(codepoint)};

            const glyph& glyph{*slot.first};
            const vec2f& texture_pos{slot.second};

            const float width {static_cast<float>(glyph.width)};
            const float height{static_cast<float>(glyph.height)};

            if(width > 0 && height > 0)
            {
                const float kerning{last != 0 && static_cast<bool>(m_options & text_drawer_options::kerning) ? m_font.kerning(last, codepoint) : 0.0f};
                const float x{current_x + glyph.origin.x() + kerning};
                const float y{current_y + glyph.origin.y()};

                vertices.emplace_back(vertex{vec3f{x, y, 0.0f}, static_cast<vec4f>(color), vec2f{(texture_pos.x()) / texture_width, (texture_pos.y()) / texture_height}});
                vertices.emplace_back(vertex{vec3f{x + width, y, 0.0f}, static_cast<vec4f>(color), vec2f{(texture_pos.x() + width) / texture_width, (texture_pos.y()) / texture_height}});
                vertices.emplace_back(vertex{vec3f{x + width, y + height, 0.0f}, static_cast<vec4f>(color), vec2f{(texture_pos.x() + width) / texture_width, (texture_pos.y() + height) / texture_height}});
                vertices.emplace_back(vertex{vec3f{x, y + height, 0.0f}, static_cast<vec4f>(color), vec2f{(texture_pos.x()) / texture_width, (texture_pos.y() + height) / texture_height}});

                lowest_x = std::min(lowest_x, x);
                lowest_y = std::min(lowest_y, y);
                greatest_x = std::max(greatest_x, x + width);
                greatest_y = std::max(greatest_y, y + height);
            }
            else
            {
                vertices.emplace_back(vertex{});
                vertices.emplace_back(vertex{});
                vertices.emplace_back(vertex{});
                vertices.emplace_back(vertex{});
            }

            current_x += glyph.advance;
            last = codepoint;
       }
    }

    signal.connect([cache = std::move(cache)](){});
    keeper.keep(texture);

    std::vector<std::uint32_t> indices{};
    indices.reserve(codepoint_count * 6);

    for(std::size_t i{}; i < codepoint_count; ++i)
    {
        const std::size_t shift{i * 4};

        indices.emplace_back(static_cast<std::uint32_t>(shift + 0));
        indices.emplace_back(static_cast<std::uint32_t>(shift + 1));
        indices.emplace_back(static_cast<std::uint32_t>(shift + 2));
        indices.emplace_back(static_cast<std::uint32_t>(shift + 2));
        indices.emplace_back(static_cast<std::uint32_t>(shift + 3));
        indices.emplace_back(static_cast<std::uint32_t>(shift + 0));
    }

    const vec3f shift{-lowest_x, -lowest_y, 0.0f};
    for(auto& vertex : vertices)
    {
        vertex.position += shift;
    }

    return text{indices, vertices, std::move(texture), static_cast<std::uint32_t>(greatest_x - lowest_x), static_cast<std::uint32_t>(greatest_y - lowest_y), std::size(string)};
}

text text_drawer::draw(std::string_view string, std::uint32_t line_width, text_align align, const color& color)
{
    auto&& [command_buffer, signal, keeper] = cpt::engine::instance().begin_transfer();

    std::unordered_map<codepoint_t, std::pair<std::shared_ptr<glyph>, vec2f>> cache{};
    texture_ptr texture{make_texture(string, cache, command_buffer)};

    const std::size_t codepoint_count{utf8::count(std::begin(string), std::end(string))};
    std::vector<vertex> vertices{};
    vertices.reserve(codepoint_count * 4);

    draw_line_state state{};
    state.current_y = static_cast<float>(m_font.info().max_ascent);
    state.lowest_x = static_cast<float>(m_font.info().max_glyph_width);
    state.lowest_y = static_cast<float>(m_font.info().max_glyph_height);
    state.texture_width = static_cast<float>(texture->width());
    state.texture_height = static_cast<float>(texture->height());

    for(auto&& line : split(string, '\n'))
    {
        draw_line(line, line_width, align, state, vertices, cache, color);

        state.current_x = 0.0f;
        state.current_y += m_font.info().line_height;

        vertices.emplace_back(vertex{});
        vertices.emplace_back(vertex{});
        vertices.emplace_back(vertex{});
        vertices.emplace_back(vertex{});
    }

    signal.connect([cache = std::move(cache)](){});
    keeper.keep(texture);

    std::vector<std::uint32_t> indices{};
    indices.reserve(codepoint_count * 6);
    for(std::size_t i{}; i < codepoint_count; ++i)
    {
        const std::size_t shift{i * 4};

        indices.emplace_back(static_cast<std::uint32_t>(shift + 0));
        indices.emplace_back(static_cast<std::uint32_t>(shift + 1));
        indices.emplace_back(static_cast<std::uint32_t>(shift + 2));
        indices.emplace_back(static_cast<std::uint32_t>(shift + 2));
        indices.emplace_back(static_cast<std::uint32_t>(shift + 3));
        indices.emplace_back(static_cast<std::uint32_t>(shift + 0));
    }

    const vec3f shift{-state.lowest_x, -state.lowest_y, 0.0f};
    for(auto& vertex : vertices)
    {
        vertex.position += shift;
    }

    const std::uint32_t text_width{static_cast<std::uint32_t>(state.greatest_x - state.lowest_x)};
    const std::uint32_t text_height{static_cast<std::uint32_t>(state.greatest_y - state.lowest_y)};

    return text{indices, vertices, std::move(texture), text_width, text_height, std::size(string)};
}

void text_drawer::draw_line(std::string_view line, std::uint32_t line_width, text_align align, draw_line_state& state, std::vector<vertex>& vertices, const std::unordered_map<codepoint_t, std::pair<std::shared_ptr<glyph>, vec2f>>& cache, const color& color)
{
    if(align == text_align::left)
    {
        const auto& space_glyph{*load_glyph(U' ')};

        for(auto word : split(line, ' '))
        {
            codepoint_t last{};

            float word_advance{};
            for(auto codepoint : word)
            {
                word_advance += cache.at(codepoint).first->advance;
            }

            if(static_cast<std::uint32_t>(state.current_x + word_advance) > line_width)
            {
                state.current_x = 0.0f;
                state.current_y += m_font.info().line_height;
                last = 0;
            }

            for(auto codepoint : word)
            {
                auto& slot{cache.at(codepoint)};

                const glyph& glyph{*slot.first};
                const vec2f texture_pos{slot.second};

                const float width {static_cast<float>(glyph.width)};
                const float height{static_cast<float>(glyph.height)};

                if(width > 0 && height > 0)
                {
                    const float kerning{last != 0 && static_cast<bool>(m_options & text_drawer_options::kerning) ? m_font.kerning(last, codepoint) : 0.0f};
                    const float x{state.current_x + glyph.origin.x() + kerning};
                    const float y{state.current_y + glyph.origin.y()};

                    vertices.emplace_back(vertex{vec3f{x, y, 0.0f}, static_cast<vec4f>(color), vec2f{(texture_pos.x()) / state.texture_width, (texture_pos.y()) / state.texture_height}});
                    vertices.emplace_back(vertex{vec3f{x + width, y, 0.0f}, static_cast<vec4f>(color), vec2f{(texture_pos.x() + width) / state.texture_width, (texture_pos.y()) / state.texture_height}});
                    vertices.emplace_back(vertex{vec3f{x + width, y + height, 0.0f}, static_cast<vec4f>(color), vec2f{(texture_pos.x() + width) / state.texture_width, (texture_pos.y() + height) / state.texture_height}});
                    vertices.emplace_back(vertex{vec3f{x, y + height, 0.0f}, static_cast<vec4f>(color), vec2f{(texture_pos.x()) / state.texture_width, (texture_pos.y() + height) / state.texture_height}});

                    state.lowest_x = std::min(state.lowest_x, x);
                    state.lowest_y = std::min(state.lowest_y, y);
                    state.greatest_x = std::max(state.greatest_x, x + width);
                    state.greatest_y = std::max(state.greatest_y, y + height);
                }
                else
                {
                    vertices.emplace_back();
                    vertices.emplace_back();
                    vertices.emplace_back();
                    vertices.emplace_back();
                }

                state.current_x += glyph.advance;
                last = codepoint;
            }

            vertices.emplace_back();
            vertices.emplace_back();
            vertices.emplace_back();
            vertices.emplace_back();

            state.current_x += space_glyph.advance;
        }

        //There is an additionnal space at the end
        vertices.erase(std::end(vertices) - 4, std::end(vertices));
    }
    else
    {
        assert(false && "only cpt::text_align::left is supported yet");
    }
}

texture_ptr text_drawer::make_texture(std::string_view string, std::unordered_map<codepoint_t, std::pair<std::shared_ptr<glyph>, vec2f>>& cache, tph::command_buffer& command_buffer)
{
    constexpr std::uint32_t max_texture_width{4096};

    std::uint32_t current_x{};
    std::uint32_t current_y{};
    std::uint32_t texture_width{};
    std::uint32_t texture_height{};

    for(auto codepoint : decode<utf8>(string))
    {
        if(cache.find(codepoint) != std::end(cache))
            continue;

        std::shared_ptr<glyph> character_glyph{load_glyph(codepoint)};

        if(current_x + character_glyph->width > max_texture_width)
        {
            current_x = 0;
            current_y += texture_height;
        }

        const vec2f texture_pos{static_cast<float>(current_x), static_cast<float>(current_y)};

        current_x += character_glyph->width;
        texture_width = std::max(current_x, texture_width);
        texture_height = std::max(static_cast<std::uint32_t>(current_y + character_glyph->height), texture_height);

        cache.emplace(std::make_pair(codepoint, std::make_pair(std::move(character_glyph), texture_pos)));
    }

    constexpr auto format{tph::texture_format::r8g8b8a8_srgb};
    constexpr auto usage{tph::texture_usage::transfer_destination | tph::texture_usage::sampled};
    auto texture{cpt::make_texture(texture_width, texture_height, tph::texture_info{format, usage}, tph::sampling_options{})};

    tph::cmd::transition(command_buffer, texture->get_texture(),
                         tph::resource_access::none, tph::resource_access::transfer_write,
                         tph::pipeline_stage::top_of_pipe, tph::pipeline_stage::transfer,
                         tph::texture_layout::undefined, tph::texture_layout::transfer_destination_optimal);

    for(auto&& [codepoint, slot] : cache)
    {
        auto&& [glyph, position] = slot;

        if(glyph->image.width() > 0 && glyph->image.height() > 0)
        {
            tph::image_texture_copy copy_region{};
            copy_region.texture_offset.x = static_cast<std::int32_t>(position.x());
            copy_region.texture_offset.y = static_cast<std::int32_t>(position.y());
            copy_region.texture_size.width = static_cast<std::uint32_t>(glyph->image.width());
            copy_region.texture_size.height = static_cast<std::uint32_t>(glyph->image.height());

            tph::cmd::copy(command_buffer, glyph->image, texture->get_texture(), copy_region);
        }
    }

    tph::cmd::transition(command_buffer, texture->get_texture(),
                         tph::resource_access::transfer_write, tph::resource_access::shader_read,
                         tph::pipeline_stage::transfer, tph::pipeline_stage::fragment_shader,
                         tph::texture_layout::transfer_destination_optimal, tph::texture_layout::shader_read_only_optimal);

    return texture;
}

const std::shared_ptr<glyph>& text_drawer::load_glyph(codepoint_t codepoint)
{
    auto it{m_cache.find(codepoint)};
    if(it == std::end(m_cache))
    {
        it = m_cache.emplace(std::make_pair(codepoint, std::make_shared<glyph>(m_font.load(codepoint).value_or(glyph{})))).first;
    }

    return it->second;
}

text draw_text(cpt::font& font, std::string_view string, const color& color, text_drawer_options options)
{
    text_drawer drawer{std::move(font), options};
    text text{drawer.draw(string, color)};

    font = std::move(drawer.font());

    return text;
}

text draw_text(cpt::font&& font, std::string_view string, const color& color, text_drawer_options options)
{
    text_drawer drawer{std::move(font), options};
    text text{drawer.draw(string, color)};

    font = std::move(drawer.font());

    return text;
}

text draw_text(cpt::font& font, std::string_view string, std::uint32_t line_width, text_align align, const color& color, text_drawer_options options)
{
    text_drawer drawer{std::move(font), options};
    text text{drawer.draw(string, line_width, align, color)};

    font = std::move(drawer.font());

    return text;
}

text draw_text(cpt::font&& font, std::string_view string, std::uint32_t line_width, text_align align, const color& color, text_drawer_options options)
{
    text_drawer drawer{std::move(font), options};
    text text{drawer.draw(string, line_width, align, color)};

    font = std::move(drawer.font());

    return text;
}

}
