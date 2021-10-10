#include "font.hpp"

#include <cassert>
#include <algorithm>
#include <fstream>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H
#include FT_STROKER_H

#include <captal_foundation/utility.hpp>

#include "engine.hpp"

namespace cpt
{

void font_engine::freetype_deleter::operator()(void* ptr) noexcept
{
    FT_Done_FreeType(reinterpret_cast<FT_Library>(ptr));
}

font_engine::handle_type font_engine::handle(std::thread::id thread)
{
    std::lock_guard lock{m_mutex};

    const auto it{m_libraries.find(thread)};
    if(it != std::end(m_libraries))
    {
        auto handle{it->second.lock()};
        if(handle)
        {
            return handle;
        }

        m_libraries.erase(it); //expired, erase it then fallback on initialization
    }

    FT_Library library{};
    if(FT_Init_FreeType(&library))
        throw std::runtime_error{"Can not init freetype library"};

    handle_type handle{library, freetype_deleter{}};
    m_libraries.emplace(thread, weak_handle_type{handle});

    return handle;
}

void font_engine::clean() noexcept
{
    std::erase_if(m_libraries, [](const auto& item)
    {
        return item.second.expired();
    });
}

static constexpr std::uint32_t default_size{256};
static constexpr tph::component_mapping red_to_alpha_mapping{tph::component_swizzle::one, tph::component_swizzle::one, tph::component_swizzle::one, tph::component_swizzle::r};
static constexpr auto font_atlas_usage{tph::texture_usage::sampled | tph::texture_usage::transfer_destination | tph::texture_usage::transfer_source};

font_atlas::font_atlas(glyph_format format, const tph::sampler_info& sampling)
:m_format{format}
,m_sampling{sampling}
,m_packer{default_size, default_size}
,m_max_size{engine::instance().graphics_device().limits().max_2d_texture_size}
{
    if(m_format == glyph_format::gray)
    {
        m_texture = make_texture(m_sampling, red_to_alpha_mapping, default_size, default_size, tph::texture_info{tph::texture_format::r8_unorm, font_atlas_usage});
    }
    else
    {
        m_texture = make_texture(m_sampling, default_size, default_size, tph::texture_info{tph::texture_format::r8g8b8a8_srgb, font_atlas_usage});
    }

    m_buffers.reserve(64);
    m_buffer_data.reserve(1024 * 8);
}

std::optional<bin_packer::rect> font_atlas::add_glyph(std::span<const uint8_t> image, std::uint32_t width, std::uint32_t height)
{
    const std::uint32_t padding{has_padding() ? 2u : 0u};

    auto rect{m_packer.append(width + padding, height + padding)};
    while(!rect.has_value())
    {
        if(m_packer.width() == m_packer.height() && m_packer.width() * 2 > m_max_size)
        {
            return std::nullopt;
        }

        if(m_grow)
        {
            m_packer.grow(m_packer.width() * 2, 0);
            m_grow = false;
        }
        else
        {
            m_packer.grow(0, m_packer.height() * 2);
            m_grow = true;
        }

        m_resized = true;

        rect = m_packer.append(width + padding, height + padding);
    }

    rect->x += padding / 2;
    rect->y += padding / 2;
    rect->width -= padding;
    rect->height -= padding;

    const auto flipped{rect->width != width};
    const auto begin{std::size(m_buffer_data)};
    m_buffer_data.resize(begin + std::size(image));

    if(flipped)
    {
        if(m_format == glyph_format::gray)
        {
            const auto it{std::begin(m_buffer_data) + begin};

            for(std::size_t y{}; y < height; ++y)
            {
                for(std::size_t x{}; x < width; ++x)
                {
                    it[x * height + y] = image[y * width + x];
                }
            }
        }
        else
        {
            const auto it{std::begin(m_buffer_data) + begin};

            for(std::size_t y{}; y < height; ++y)
            {
                const auto base_padding{y * width * 4};
                const auto flipped_index{y * 4};

                for(std::size_t x{}; x < width; ++x)
                {
                    const auto base_index{x * 4};
                    const auto flipped_padding{x * height * 4};

                    const auto base_begin{base_padding + base_index};
                    const auto flipped_begin{flipped_padding + flipped_index};

                    it[flipped_begin + 0] = image[base_begin + 0];
                    it[flipped_begin + 1] = image[base_begin + 1];
                    it[flipped_begin + 2] = image[base_begin + 2];
                    it[flipped_begin + 3] = image[base_begin + 3];
                }
            }
        }
    }
    else
    {
        std::copy(std::begin(image), std::end(image), std::begin(m_buffer_data) + begin);
    }

    m_buffers.emplace_back(transfer_buffer{begin, *rect});

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
        if(std::exchange(m_first_upload, false))
        {
            tph::texture_memory_barrier barrier{m_texture->get_texture()};
            barrier.source_access      = tph::resource_access::none;
            barrier.destination_access = tph::resource_access::transfer_write;
            barrier.old_layout         = tph::texture_layout::undefined;
            barrier.new_layout         = tph::texture_layout::transfer_destination_optimal;

            tph::cmd::pipeline_barrier(buffer, tph::pipeline_stage::top_of_pipe, tph::pipeline_stage::transfer, tph::dependency_flags::none, {}, {}, std::span{&barrier, 1});
        }
        else
        {
            tph::texture_memory_barrier barrier{m_texture->get_texture()};
            barrier.source_access      = tph::resource_access::none;
            barrier.destination_access = tph::resource_access::transfer_write;
            barrier.old_layout         = tph::texture_layout::shader_read_only_optimal;
            barrier.new_layout         = tph::texture_layout::transfer_destination_optimal;

            tph::cmd::pipeline_barrier(buffer, tph::pipeline_stage::top_of_pipe, tph::pipeline_stage::transfer, tph::dependency_flags::none, {}, {}, std::span{&barrier, 1});
        }
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
        copy.texture_size.depth = 1;

        copies.emplace_back(copy);
    }

    tph::buffer staging_buffer{engine::instance().renderer(), std::size(m_buffer_data), tph::buffer_usage::staging | tph::buffer_usage::transfer_source};
    std::memcpy(staging_buffer.map(), std::data(m_buffer_data), std::size(m_buffer_data));
    staging_buffer.unmap();

#ifdef CAPTAL_DEBUG
    if(!std::empty(m_name))
    {
        tph::set_object_name(engine::instance().renderer(), staging_buffer, m_name + " staging buffer (frame: " + std::to_string(engine::instance().frame()) + ")");
    }
#endif

    tph::cmd::copy(buffer, staging_buffer, m_texture->get_texture(), copies);

    tph::texture_memory_barrier barrier{m_texture->get_texture()};
    barrier.source_access      = tph::resource_access::transfer_write;
    barrier.destination_access = tph::resource_access::shader_read;
    barrier.old_layout         = tph::texture_layout::transfer_destination_optimal;
    barrier.new_layout         = tph::texture_layout::shader_read_only_optimal;

    tph::cmd::pipeline_barrier(buffer, tph::pipeline_stage::transfer, tph::pipeline_stage::fragment_shader, tph::dependency_flags::none, {}, {}, std::span{&barrier, 1});

    keeper.keep(m_texture);
    signal.connect([buffer = std::move(staging_buffer)](){});

    m_buffers.clear();
    m_buffer_data.clear();
}

#ifdef CAPTAL_DEBUG
void font_atlas::set_name(std::string_view name)
{
    m_name = name;

    m_texture->set_name(m_name + " texture");
}
#endif

void font_atlas::resize(tph::command_buffer& buffer, asynchronous_resource_keeper& keeper)
{
    texture_ptr new_texture{};
    if(m_format == glyph_format::gray)
    {
        new_texture = make_texture(m_sampling, red_to_alpha_mapping, m_packer.width(), m_packer.height(), tph::texture_info{tph::texture_format::r8_unorm, font_atlas_usage});
    }
    else
    {
        new_texture = make_texture(m_sampling, m_packer.width(), m_packer.height(), tph::texture_info{tph::texture_format::r8g8b8a8_srgb, font_atlas_usage});
    }

#ifdef CAPTAL_DEBUG
    if(!std::empty(m_name))
    {
        m_texture->set_name(m_name + " old texture (frame: " + std::to_string(engine::instance().frame()) + ")");
        new_texture->set_name(m_name + " texture");
    }
#endif

    if(std::exchange(m_first_upload, false))
    {
        tph::texture_memory_barrier barrier{m_texture->get_texture()};
        barrier.source_access      = tph::resource_access::none;
        barrier.destination_access = tph::resource_access::transfer_read;
        barrier.old_layout         = tph::texture_layout::undefined;
        barrier.new_layout         = tph::texture_layout::transfer_source_optimal;

        tph::cmd::pipeline_barrier(buffer, tph::pipeline_stage::top_of_pipe, tph::pipeline_stage::transfer, tph::dependency_flags::none, {}, {}, std::span{&barrier, 1});
    }
    else
    {
        tph::texture_memory_barrier barrier{m_texture->get_texture()};
        barrier.source_access      = tph::resource_access::none;
        barrier.destination_access = tph::resource_access::transfer_read;
        barrier.old_layout         = tph::texture_layout::shader_read_only_optimal;
        barrier.new_layout         = tph::texture_layout::transfer_source_optimal;

        tph::cmd::pipeline_barrier(buffer, tph::pipeline_stage::top_of_pipe, tph::pipeline_stage::transfer, tph::dependency_flags::none, {}, {}, std::span{&barrier, 1});
    }

    tph::texture_memory_barrier barrier{new_texture->get_texture()};
    barrier.source_access      = tph::resource_access::none;
    barrier.destination_access = tph::resource_access::transfer_write;
    barrier.old_layout         = tph::texture_layout::undefined;
    barrier.new_layout         = tph::texture_layout::transfer_destination_optimal;

    tph::cmd::pipeline_barrier(buffer, tph::pipeline_stage::top_of_pipe, tph::pipeline_stage::transfer, tph::dependency_flags::none, {}, {}, std::span{&barrier, 1});

    tph::texture_copy region{};
    region.size.width  = m_texture->width();
    region.size.height = m_texture->height();

    tph::cmd::copy(buffer, m_texture->get_texture(), new_texture->get_texture(), region);

    keeper.keep(std::exchange(m_texture, std::move(new_texture)));
    m_signal(m_texture);
}

struct glyph_keeper
{
    glyph_keeper(const FT_Glyph& glyph)
    :m_glyph{glyph}
    {

    }

    ~glyph_keeper()
    {
        FT_Done_Glyph(m_glyph);
    }

    glyph_keeper(const glyph_keeper&) = delete;
    glyph_keeper& operator=(const glyph_keeper&) = delete;
    glyph_keeper(glyph_keeper&&) noexcept = delete;
    glyph_keeper& operator=(glyph_keeper&&) noexcept = delete;

private:
    const FT_Glyph& m_glyph;
};

static glyph make_glyph(FT_Glyph_Metrics metrics) noexcept
{
    glyph output{};
    output.origin  = vec2f{static_cast<float>(metrics.horiBearingX) / 64.0f, -static_cast<float>(metrics.horiBearingY) / 64.0f};
    output.advance = static_cast<float>(metrics.horiAdvance) / 64.0f;
    output.ascent  = static_cast<float>(metrics.horiBearingY) / 64.0f;
    output.descent = (static_cast<float>(metrics.height) / 64.0f) - (static_cast<float>(metrics.horiBearingY) / 64.0f);

    return output;
}

static std::vector<std::uint8_t> convert_bitmap(glyph_format format, std::uint32_t width, std::uint32_t height, const FT_Bitmap& bitmap)
{
    std::vector<std::uint8_t> output{};

    if(format == glyph_format::gray)
    {
        output.resize(height * width);
    }
    else
    {
        output.resize(height * width * 4);
        std::fill(std::begin(output), std::end(output), 255);
    }

    const std::uint8_t* data{bitmap.buffer};

    if(bitmap.pixel_mode == FT_PIXEL_MODE_GRAY)
    {
        if(format == glyph_format::gray)
        {
            for(std::size_t y{}; y < height; ++y)
            {
                const auto padding{y * bitmap.width};

                for(std::size_t x{}; x < width; ++x)
                {
                    output[padding + x] = data[x];
                }

                data += bitmap.pitch;
            }
        }
        else
        {
            for(std::size_t y{}; y < height; ++y)
            {
                const auto padding{y * 4 * bitmap.width};

                for(std::size_t x{}; x < width; ++x)
                {
                    output[padding + x * 4 + 3] = data[x];
                }

                data += bitmap.pitch;
            }
        }
    }
    else if(bitmap.pixel_mode == FT_PIXEL_MODE_BGRA)
    {
        for(std::size_t y{}; y < height; ++y)
        {
            const auto padding{y * 4 * bitmap.width};

            for(std::size_t x{}; x < width; ++x)
            {
                const auto begin{padding + x};

                output[begin + 0] = data[x + 2];
                output[begin + 1] = data[x + 1];
                output[begin + 2] = data[x + 0];
                output[begin + 3] = data[x + 3];
            }

            data += bitmap.pitch;
        }
    }

    return output;
}

void font::face_deleter::operator()(void* ptr) const noexcept
{
    FT_Done_Face(reinterpret_cast<FT_Face>(ptr));
}

void font::stroker_deleter::operator()(void* ptr) const noexcept
{
    FT_Stroker_Done(reinterpret_cast<FT_Stroker>(ptr));
}

font::font(std::span<const std::uint8_t> data, std::uint32_t initial_size)
:m_data{std::begin(data), std::end(data)}
{
    init(initial_size);
}

font::font(const std::filesystem::path& file, std::uint32_t initial_size)
:m_data{read_file< std::vector<std::uint8_t> >(file)}
{
    init(initial_size);
}

font::font(std::istream& stream, std::uint32_t initial_size)
{
    assert(stream && "Invalid stream.");

    m_data = std::vector<std::uint8_t>{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};

    init(initial_size);
}

std::optional<glyph> font::load(codepoint_t codepoint, glyph_format format, bool embolden, float outline, float lean, float shift)
{
    assert(outline >= 0.0f && "cpt::font::load called with outline not in range [0; +inf]");
    assert((0.0f <= lean    && lean    <= 1.0f) && "cpt::font::load called with lean not in range [0; 1]");
    assert((0.0f <= shift   && shift   <= 1.0f) && "cpt::font::load called with shift not in range [0; 1]");

    const auto library{reinterpret_cast<FT_Library>(m_engine.get())};
    const auto face   {reinterpret_cast<FT_Face>(m_face.get())};
    const auto stroker{reinterpret_cast<FT_Stroker>(m_stroker.get())};

    std::int32_t flags{};

    if(format == glyph_format::color)
    {
        flags |= FT_LOAD_COLOR;
    }

    if(outline > 0.0f || lean > 0.0f || shift > 0.0f)
    {
        flags |= FT_LOAD_NO_BITMAP;
    }

    if(FT_Load_Char(face, codepoint, flags))
    {
        return std::nullopt;
    }

    glyph output{make_glyph(face->glyph->metrics)};

    FT_Glyph glyph;
    if(FT_Get_Glyph(face->glyph, &glyph))
    {
        return std::nullopt;
    }

    const glyph_keeper keeper{glyph};

    std::optional<std::int32_t> old_xmin{};

    if(glyph->format == FT_GLYPH_FORMAT_OUTLINE)
    {
        if(embolden)
        {
            FT_Outline_Embolden(&reinterpret_cast<FT_OutlineGlyph>(glyph)->outline, m_info.size);
        }

        if(outline > 0.0f)
        {
            FT_Stroker_Set(stroker, static_cast<FT_Fixed>(outline * 64.0f), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
            FT_Glyph_Stroke(&glyph, stroker, static_cast<FT_Bool>(true));

            output.advance += outline;
        }

        if(lean > 0.0f)
        {
             FT_Matrix matrix{65536, static_cast<FT_Fixed>(lean * 65536.0f), 0, 65536};
             FT_Glyph_Transform(glyph, &matrix, nullptr);
        }

        if(shift > 0.0f)
        {
            FT_BBox bbox;
            FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_PIXELS, &bbox);
            old_xmin = bbox.xMin;

            FT_Outline_Translate(&reinterpret_cast<FT_OutlineGlyph>(glyph)->outline, static_cast<FT_Pos>(shift * 64.0f), 0);
        }

        if(FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, static_cast<FT_Bool>(true)))
        {
            return std::nullopt;
        }
    }
    else
    {
        if(embolden)
        {
            FT_Bitmap_Embolden(library, &reinterpret_cast<FT_BitmapGlyph>(glyph)->bitmap, m_info.size, m_info.size);
        }
    }

    const FT_Bitmap& bitmap{reinterpret_cast<FT_BitmapGlyph>(glyph)->bitmap};
    output.width = bitmap.width;
    output.height = bitmap.rows;

    if(output.width > 0 && output.height > 0)
    {
        FT_BBox bbox;
        FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_PIXELS, &bbox);

        if(old_xmin && *old_xmin != bbox.xMin) //Subpixel adjustment correction
        {
            output.origin.x() += 1.0f;
        }

        output.data = convert_bitmap(format, output.width, output.height, bitmap);
    }

    return std::make_optional(std::move(output));
}

std::optional<glyph> font::load_no_render(codepoint_t codepoint, bool embolden, float outline, float lean, float shift)
{
    assert(outline >= 0.0f && "cpt::font::load called with outline not in range [0; +inf]");
    assert((0.0f <= lean    && lean    <= 1.0f) && "cpt::font::load called with lean not in range [0; 1]");
    assert((0.0f <= shift   && shift   <= 1.0f) && "cpt::font::load called with shift not in range [0; 1]");

    const auto face   {reinterpret_cast<FT_Face>(m_face.get())};
    const auto stroker{reinterpret_cast<FT_Stroker>(m_stroker.get())};

    std::int32_t flags{};

    if(outline > 0.0f || lean > 0.0f || shift > 0.0f)
    {
        flags |= FT_LOAD_NO_BITMAP;
    }
    else
    {
        flags |= FT_LOAD_BITMAP_METRICS_ONLY;
    }

    if(FT_Load_Char(face, codepoint, flags))
    {
        return std::nullopt;
    }

    glyph output{make_glyph(face->glyph->metrics)};

    FT_Glyph glyph;
    if(FT_Get_Glyph(face->glyph, &glyph))
    {
        return std::nullopt;
    }

    const glyph_keeper keeper{glyph};

    if(glyph->format == FT_GLYPH_FORMAT_OUTLINE)
    {
        if(embolden)
        {
            FT_Outline_Embolden(&reinterpret_cast<FT_OutlineGlyph>(glyph)->outline, m_info.size);
        }

        if(outline > 0.0f)
        {
            FT_Stroker_Set(stroker, static_cast<FT_Fixed>(outline * 64.0f), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
            FT_Glyph_Stroke(&glyph, stroker, static_cast<FT_Bool>(true));
        }

        if(lean > 0.0f)
        {
             FT_Matrix matrix{65536, static_cast<FT_Fixed>(lean * 65536.0f), 0, 65536};
             FT_Glyph_Transform(glyph, &matrix, nullptr);
        }

        FT_BBox bbox;
        FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_PIXELS, &bbox);
        const auto old_xmin{bbox.xMin};

        if(shift > 0.0f)
        {
            FT_Outline_Translate(&reinterpret_cast<FT_OutlineGlyph>(glyph)->outline, static_cast<FT_Pos>(shift * 64.0f), 0);

            FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_PIXELS, &bbox);

            if(old_xmin != bbox.xMin)
            {
                output.origin.x() += 1.0f;
            }
        }

        output.width = static_cast<std::uint32_t>(bbox.xMax - bbox.xMin);
        output.height = static_cast<std::uint32_t>(bbox.yMax - bbox.yMin);
    }
    else
    {
        output.width = reinterpret_cast<FT_BitmapGlyph>(glyph)->bitmap.width;
        output.height = reinterpret_cast<FT_BitmapGlyph>(glyph)->bitmap.rows;

        if(embolden)
        {
            const auto increase{((m_info.size + 32u) & 0xFFFFFFC0u) >> 6u};

            output.width += increase;
            output.height += increase;
        }
    }

    return std::make_optional(std::move(output));
}

std::optional<glyph> font::load_render(codepoint_t codepoint, glyph_format format, bool embolden, float outline, float lean, float shift)
{
    assert(outline >= 0.0f && "cpt::font::load called with outline not in range [0; +inf]");
    assert((0.0f <= lean    && lean    <= 1.0f) && "cpt::font::load called with lean not in range [0; 1]");
    assert((0.0f <= shift   && shift   <= 1.0f) && "cpt::font::load called with shift not in range [0; 1]");

    const auto library{reinterpret_cast<FT_Library>(m_engine.get())};
    const auto face   {reinterpret_cast<FT_Face>(m_face.get())};
    const auto stroker{reinterpret_cast<FT_Stroker>(m_stroker.get())};

    std::int32_t flags{};

    if(format == glyph_format::color)
    {
        flags |= FT_LOAD_COLOR;
    }

    if(outline > 0.0f || lean > 0.0f || shift > 0.0f)
    {
        flags |= FT_LOAD_NO_BITMAP;
    }

    if(FT_Load_Char(face, codepoint, flags))
    {
        return std::nullopt;
    }

    glyph output{};

    FT_Glyph glyph;
    if(FT_Get_Glyph(face->glyph, &glyph))
    {
        return std::nullopt;
    }

    const glyph_keeper keeper{glyph};

    if(glyph->format == FT_GLYPH_FORMAT_OUTLINE)
    {
        if(embolden)
        {
            FT_Outline_Embolden(&reinterpret_cast<FT_OutlineGlyph>(glyph)->outline, m_info.size);
        }

        if(outline > 0.0f)
        {
            FT_Stroker_Set(stroker, static_cast<FT_Fixed>(outline * 64.0f), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
            FT_Glyph_Stroke(&glyph, stroker, static_cast<FT_Bool>(true));
        }

        if(lean > 0.0f)
        {
             FT_Matrix matrix{65536, static_cast<FT_Fixed>(lean * 65536.0f), 0, 65536};
             FT_Glyph_Transform(glyph, &matrix, nullptr);
        }

        if(shift > 0.0f)
        {
            FT_Outline_Translate(&reinterpret_cast<FT_OutlineGlyph>(glyph)->outline, static_cast<FT_Pos>(shift * 64.0f), 0);
        }

        if(FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, static_cast<FT_Bool>(true)))
        {
            return std::nullopt;
        }
    }
    else
    {
        if(embolden)
        {
            FT_Bitmap_Embolden(library, &reinterpret_cast<FT_BitmapGlyph>(glyph)->bitmap, m_info.size, m_info.size);
        }
    }

    const FT_Bitmap& bitmap{reinterpret_cast<FT_BitmapGlyph>(glyph)->bitmap};
    output.width = bitmap.width;
    output.height = bitmap.rows;

    if(output.width > 0 && output.height > 0)
    {
        output.data = convert_bitmap(format, output.width, output.height, bitmap);
    }

    return std::make_optional(std::move(output));
}

bool font::has(codepoint_t codepoint) const noexcept
{
    const auto face{reinterpret_cast<FT_Face>(m_face.get())};

    return FT_Get_Char_Index(face, codepoint) != 0;
}

vec2f font::kerning(codepoint_t left, codepoint_t right) const noexcept
{
    const auto face{reinterpret_cast<FT_Face>(m_face.get())};

    FT_Vector output{};

    if(FT_Get_Kerning(face, FT_Get_Char_Index(face, left), FT_Get_Char_Index(face, right), FT_KERNING_UNFITTED, &output))
    {
        return vec2f{};
    }

    const float factor{FT_IS_SCALABLE(face) ? 1.0f / 64.0f : 1.0f};

    return vec2f{static_cast<float>(output.x) * factor, static_cast<float>(output.y) * factor};
}

void font::resize(std::uint32_t pixels_size)
{
    const auto face{reinterpret_cast<FT_Face>(m_face.get())};

    if(pixels_size != m_info.size)
    {
        if(FT_Set_Pixel_Sizes(face, 0, pixels_size))
            throw std::runtime_error{"Can not set font size."};

        m_info.size                = pixels_size;
        m_info.max_glyph_width     = FT_MulFix(face->bbox.xMax - face->bbox.xMin, face->size->metrics.x_scale) / 64 + 1;
        m_info.max_glyph_height    = FT_MulFix(face->bbox.yMax - face->bbox.yMin, face->size->metrics.y_scale) / 64 + 1;
        m_info.max_ascent          = FT_MulFix(face->bbox.yMax, face->size->metrics.y_scale) / 64 + 1;
        m_info.line_height         = std::floor(static_cast<float>(FT_MulFix(face->height, face->size->metrics.y_scale)) / 64.0f);
        m_info.underline_position  = static_cast<float>(-FT_MulFix(face->underline_position, face->size->metrics.y_scale)) / 64.0f;
        m_info.underline_thickness = static_cast<float>(FT_MulFix(face->underline_thickness, face->size->metrics.y_scale)) / 64.0f;

        const auto x_glyph{load_no_render(U'x')};

        if(x_glyph)
        {
            m_info.strikeout_position = static_cast<float>(x_glyph->height) / 2.0f;
        }
        else
        {
            m_info.strikeout_position = static_cast<float>(m_info.size) / 3.0f;
        }
    }
}

void font::init(std::uint32_t initial_size)
{
    m_engine = engine::instance().font_engine().handle(std::this_thread::get_id());

    const auto library{reinterpret_cast<FT_Library>(m_engine.get())};

    FT_Face face{};
    if(FT_New_Memory_Face(library, reinterpret_cast<const FT_Byte*>(std::data(m_data)), static_cast<FT_Long>(std::size(m_data)), 0, &face))
        throw std::runtime_error{"Can not init freetype font face."};

    m_face = face_handle_type{face};

    FT_Stroker stroker{};
    if(FT_Stroker_New(library, &stroker))
        throw std::runtime_error{"Can not init freetype font stroker."};

    m_stroker = stroker_handle_type{stroker};

    if(FT_Select_Charmap(face, FT_ENCODING_UNICODE))
        throw std::runtime_error{"Can not set font charmap."};

    m_info.family = std::string{face->family_name};
    m_info.glyph_count = face->num_glyphs;
    m_info.category = static_cast<font_category>(face->style_flags);
    m_info.features = static_cast<font_features>(face->face_flags);

    resize(initial_size);
}

}
