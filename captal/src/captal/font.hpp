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

#ifndef CAPTAL_FONT_HPP_INCLUDED
#define CAPTAL_FONT_HPP_INCLUDED

#include "config.hpp"

#include <memory>
#include <string_view>
#include <filesystem>
#include <istream>
#include <unordered_map>
#include <mutex>

#include <captal_foundation/math.hpp>
#include <captal_foundation/encoding.hpp>

#include "texture.hpp"
#include "signal.hpp"
#include "bin_packing.hpp"

namespace cpt
{

class CAPTAL_API font_engine
{
    struct CAPTAL_API freetype_deleter
    {
        void operator()(void* ptr) noexcept;
    };

public:
    using handle_type = std::shared_ptr<void>;
    using weak_handle_type = std::weak_ptr<void>;

public:
    font_engine() = default;
    ~font_engine() = default;
    font_engine(const font_engine&) = delete;
    font_engine& operator=(const font_engine&) = delete;
    font_engine(font_engine&&) noexcept = delete;
    font_engine& operator=(font_engine&&) noexcept = delete;

    handle_type handle(std::thread::id thread);
    void clean() noexcept;

private:
    std::unordered_map<std::thread::id, weak_handle_type> m_libraries{};
    std::mutex m_mutex{};
};

enum class glyph_format : std::uint32_t
{
    gray  = 0,
    color = 1
};

using font_atlas_resize_signal = cpt::signal<texture_ptr>;

class CAPTAL_API font_atlas
{
public:
    font_atlas() = default;
    explicit font_atlas(glyph_format format, const tph::sampler_info& sampling = tph::sampler_info{});

    ~font_atlas() = default;
    font_atlas(const font_atlas&) = delete;
    font_atlas& operator=(const font_atlas&) = delete;
    font_atlas(font_atlas&&) noexcept = default;
    font_atlas& operator=(font_atlas&&) noexcept = default;

    std::optional<bin_packer::rect> add_glyph(std::span<const std::uint8_t> image, std::uint32_t width, std::uint32_t height);
    void upload();

    const texture_ptr& texture() const noexcept
    {
        return m_texture;
    }

    font_atlas_resize_signal& signal() noexcept
    {
        return m_signal;
    }

    bool need_upload() const noexcept
    {
        return !std::empty(m_buffers);
    }

    bool has_padding() const noexcept
    {
        return m_sampling.mag_filter != tph::filter::nearest || m_sampling.min_filter != tph::filter::nearest;
    }

#ifdef CAPTAL_DEBUG
    void set_name(std::string_view name);
#else
    void set_name(std::string_view name [[maybe_unused]]) const noexcept
    {

    }
#endif

private:
    void resize(tph::command_buffer& buffer, asynchronous_resource_keeper& keeper);

private:
    struct transfer_buffer
    {
        std::size_t begin{};
        bin_packer::rect rect{};
    };

private:
    glyph_format m_format{};
    texture_ptr m_texture{};
    tph::sampler_info m_sampling{};
    font_atlas_resize_signal m_signal{};
    bin_packer m_packer{};
    std::vector<transfer_buffer> m_buffers{};
    std::vector<std::uint8_t> m_buffer_data{};
    std::uint32_t m_max_size{};
    bool m_resized{};
    bool m_first_upload{true};
    bool m_grow{};
#ifdef CAPTAL_DEBUG
    std::string m_name{};
#endif
};

enum class font_category : std::uint32_t
{
    regular = 0x00,
    italic = 0x01,
    bold = 0x02,
};

enum class font_features : std::uint32_t
{
    scalable = 0x0001,
    fixed_sizes = 0x0002,
    fixed_width = 0x0004,
    horizontal = 0x0010,
    vertical = 0x0020,
    kerning = 0x0040,
    glyph_names = 0x0100,
    hinter = 0x0400,
    color = 0x2000
};

struct font_info
{
    std::string family{};
    std::uint32_t glyph_count{};
    font_category category{};
    font_features features{};
    std::uint32_t size{};
    std::uint32_t max_glyph_height{};
    std::uint32_t max_glyph_width{};
    std::uint32_t max_ascent{};
    float line_height{};
    float underline_position{};
    float underline_thickness{};
    float strikeout_position{};
};

struct glyph
{
    vec2f origin{};
    float advance{};
    float ascent{};
    float descent{};
    std::uint32_t width{};
    std::uint32_t height{};
    std::vector<std::uint8_t> data{};
};

class CAPTAL_API font
{
    struct CAPTAL_API face_deleter
    {
        void operator()(void* ptr) const noexcept;
    };

    struct CAPTAL_API stroker_deleter
    {
        void operator()(void* ptr) const noexcept;
    };

    using face_handle_type    = std::unique_ptr<void, face_deleter>;
    using stroker_handle_type = std::unique_ptr<void, stroker_deleter>;

public:
    font() = default;
    explicit font(const std::filesystem::path& file, std::uint32_t initial_size);
    explicit font(std::span<const std::uint8_t> data, std::uint32_t initial_size);
    explicit font(std::istream& stream, std::uint32_t initial_size);

    ~font() = default;
    font(const font&) = delete;
    font& operator=(const font&) = delete;
    font(font&&) noexcept = default;
    font& operator=(font&&) noexcept = default;

    std::optional<glyph> load_no_render(codepoint_t codepoint, bool embolden = false, float outline = 0.0f, float lean = 0.0f, float shift = 0.0f);
    std::optional<glyph> load_render(codepoint_t codepoint, glyph_format format, bool embolden = false, float outline = 0.0f, float lean = 0.0f, float shift = 0.0f);
    std::optional<glyph> load(codepoint_t codepoint, glyph_format format, bool embolden = false, float outline = 0.0f, float lean = 0.0f, float shift = 0.0f);

    void resize(std::uint32_t pixels_size);

    bool has(codepoint_t codepoint) const noexcept;
    vec2f kerning(codepoint_t left, codepoint_t right) const noexcept;

    const font_info& info() const noexcept
    {
        return m_info;
    }

private:
    void init(std::uint32_t initial_size);

private:
    font_engine::handle_type m_engine{};
    face_handle_type m_face{};
    stroker_handle_type m_stroker{};
    std::vector<std::uint8_t> m_data{};
    font_info m_info{};
};

}

template<> struct cpt::enable_enum_operations<cpt::font_category> {static constexpr bool value{true};};
template<> struct cpt::enable_enum_operations<cpt::font_features> {static constexpr bool value{true};};

#endif
