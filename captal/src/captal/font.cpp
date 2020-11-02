#include "font.hpp"

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

std::optional<bin_packer::rect> font_atlas::add_glyph(std::span<const uint8_t> image, std::uint32_t width, std::uint32_t height)
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

void font::freetype_deleter::operator()(void* ptr) noexcept
{
    std::lock_guard lock{engine::instance().font_engine().mutex()};

    FT_Done_Face(reinterpret_cast<FT_Face>(ptr));
}

font::font(std::span<const std::uint8_t> data, std::uint32_t initial_size, glyph_format format)
:m_data{std::begin(data), std::end(data)}
{
    init(initial_size, format);
}

font::font(const std::filesystem::path& file, std::uint32_t initial_size, glyph_format format)
:m_data{read_file<std::vector<std::uint8_t>>(file)}
{
    init(initial_size, format);
}

font::font(std::istream& stream, std::uint32_t initial_size, glyph_format format)
{
    assert(stream && "Invalid stream.");

    m_data = std::vector<std::uint8_t>{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};

    init(initial_size, format);
}

std::optional<glyph> font::load(codepoint_t codepoint)
{
    const auto face{reinterpret_cast<FT_Face>(m_loader.get())};

    if(FT_Load_Char(face, codepoint, m_info.format == glyph_format::color ? FT_LOAD_COLOR : FT_LOAD_DEFAULT))
    {
        return std::nullopt;
    }

    glyph output{};
    output.origin = vec2f{face->glyph->metrics.horiBearingX / 64.0f, -face->glyph->metrics.horiBearingY / 64.0f};
    output.advance = face->glyph->metrics.horiAdvance / 64.0f;
    output.ascent = face->glyph->metrics.horiBearingY / 64.0f;
    output.descent = (face->glyph->metrics.height / 64.0f) - (face->glyph->metrics.horiBearingY / 64.0f);

    return std::make_optional(std::move(output));
}

std::optional<glyph> font::load_image(codepoint_t codepoint, bool embolden)
{
    const auto library{reinterpret_cast<FT_Library>(engine::instance().font_engine().handle())};
    const auto face{reinterpret_cast<FT_Face>(m_loader.get())};

    auto output{load(codepoint)};
    if(!output)
    {
        return std::nullopt;
    }

    if(face->glyph->format == FT_GLYPH_FORMAT_OUTLINE && embolden)
    {
        FT_Outline_Embolden(&face->glyph->outline, 64);
    }

    if(FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
    {
        return std::nullopt;
    }

    if(face->glyph->format == FT_GLYPH_FORMAT_BITMAP && embolden)
    {
        FT_Bitmap_Embolden(library, &face->glyph->bitmap, 64, 64);
    }

    const FT_Bitmap& bitmap{face->glyph->bitmap};
    output->width = bitmap.width;
    output->height = bitmap.rows;

    if(m_info.format == glyph_format::gray)
    {
        output->data.resize(output->height * output->width);
    }
    else
    {
        output->data.resize(output->height * output->width * 4);
        std::fill(std::begin(output->data), std::end(output->data), 255);
    }

    if(output->width > 0 && output->height > 0)
    {
        const std::uint8_t* data{bitmap.buffer};

        if(bitmap.pixel_mode == FT_PIXEL_MODE_GRAY)
        {
            if(m_info.format == glyph_format::gray)
            {
                for(std::size_t y{}; y < output->height; ++y)
                {
                    for(std::size_t x{}; x < output->width; ++x)
                    {
                        output->data[y * bitmap.width + x] = data[x];
                    }

                    data += bitmap.pitch;
                }
            }
            else
            {
                for(std::size_t y{}; y < output->height; ++y)
                {
                    for(std::size_t x{}; x < output->width; ++x)
                    {
                        output->data[y * bitmap.width + x + 3] = data[x];
                    }

                    data += bitmap.pitch;
                }
            }
        }
        else if(bitmap.pixel_mode == FT_PIXEL_MODE_BGRA)
        {
            for(std::size_t y{}; y < output->height; ++y)
            {
                for(std::size_t x{}; x < output->width; ++x)
                {
                    output->data[y * bitmap.width + x + 0] = data[x + 2];
                    output->data[y * bitmap.width + x + 1] = data[x + 1];
                    output->data[y * bitmap.width + x + 2] = data[x + 0];
                    output->data[y * bitmap.width + x + 3] = data[x + 3];
                }

                data += bitmap.pitch;
            }
        }
    }

    return output;
}

bool font::has(codepoint_t codepoint) const noexcept
{
    const auto face{reinterpret_cast<FT_Face>(m_loader.get())};

    return FT_Get_Char_Index(face, codepoint);
}

vec2f font::kerning(codepoint_t left, codepoint_t right)
{
    const auto face{reinterpret_cast<FT_Face>(m_loader.get())};

    FT_Vector output{};

    if(FT_Get_Kerning(face, FT_Get_Char_Index(face, left), FT_Get_Char_Index(face, right), FT_KERNING_DEFAULT, &output))
    {
        return vec2f{};
    }

    return vec2f{output.x / (FT_IS_SCALABLE(face) ? 64.0f : 1.0f), output.y / (FT_IS_SCALABLE(face) ? 64.0f : 1.0f)};
}

void font::resize(std::uint32_t pixels_size)
{
    const auto face{reinterpret_cast<FT_Face>(m_loader.get())};

    if(pixels_size != m_info.size)
    {
        if(FT_Set_Pixel_Sizes(face, 0, pixels_size))
            throw std::runtime_error{"Can not set font size."};

        m_info.size = pixels_size;
        m_info.line_height = face->size->metrics.height / 64.0f;
        m_info.max_glyph_width = FT_MulFix(face->bbox.xMax - face->bbox.xMin, face->size->metrics.x_scale) / 64 + 1;
        m_info.max_glyph_height = FT_MulFix(face->bbox.yMax - face->bbox.yMin, face->size->metrics.y_scale) / 64 + 1;
        m_info.max_ascent = FT_MulFix(face->ascender, face->size->metrics.y_scale) / 64 + 1;
        m_info.underline_position = -FT_MulFix(face->underline_position, face->size->metrics.y_scale) / 64.0f;
        m_info.underline_thickness = -FT_MulFix(face->underline_thickness, face->size->metrics.y_scale) / 64.0f;
        m_info.strikeout_position = static_cast<float>(m_info.max_ascent / 3);
    }
}

void font::init(std::uint32_t initial_size, glyph_format format)
{
    const auto library{reinterpret_cast<FT_Library>(engine::instance().font_engine().handle())};
    FT_Face face{};

    std::lock_guard lock{engine::instance().font_engine().mutex()};
    if(FT_New_Memory_Face(library, reinterpret_cast<const FT_Byte*>(std::data(m_data)), static_cast<FT_Long>(std::size(m_data)), 0, &face))
        throw std::runtime_error{"Can not init freetype font face."};

    m_loader = handle_type{face};

    if(FT_Select_Charmap(face, FT_ENCODING_UNICODE))
        throw std::runtime_error{"Can not set font charmap."};

    m_info.format = format;
    m_info.family = std::string{face->family_name};
    m_info.glyph_count = face->num_glyphs;
    m_info.category = static_cast<font_category>(face->style_flags);
    m_info.features = static_cast<font_features>(face->face_flags);

    resize(initial_size);
}

}
