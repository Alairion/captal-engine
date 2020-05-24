#include "texture.hpp"

#include "engine.hpp"

namespace cpt
{

static tph::texture_format format_from_color_space(color_space space) noexcept
{
    switch(space)
    {
        case color_space::srgb:   return tph::texture_format::r8g8b8a8_srgb;
        case color_space::linear: return tph::texture_format::r8g8b8a8_unorm;
        default: std::terminate();
    }
}

template<typename... Args>
static tph::texture make_texture(const tph::sampling_options& sampling, tph::texture_format format, Args&&... args)
{
    tph::image image{cpt::engine::instance().renderer(), std::forward<Args>(args)..., tph::image_usage::transfer_source};
    tph::texture texture{cpt::engine::instance().renderer(), static_cast<std::uint32_t>(image.width()), static_cast<std::uint32_t>(image.height()), sampling, format, tph::texture_usage::sampled | tph::texture_usage::transfer_destination};

    auto&& [command_buffer, signal] = cpt::engine::instance().begin_transfer();

    tph::cmd::copy(command_buffer, image, texture);
    tph::cmd::prepare(command_buffer, texture, tph::pipeline_stage::fragment_shader);

    signal.connect([image = std::move(image)](){});

    return texture;
}

static tph::texture make_texture(const tph::sampling_options& sampling, tph::texture_format format, tph::image image)
{
    tph::texture texture{cpt::engine::instance().renderer(), static_cast<std::uint32_t>(image.width()), static_cast<std::uint32_t>(image.height()), sampling, format, tph::texture_usage::sampled | tph::texture_usage::transfer_destination};

    auto&& [command_buffer, signal] = cpt::engine::instance().begin_transfer();

    tph::cmd::copy(command_buffer, image, texture);
    tph::cmd::prepare(command_buffer, texture, tph::pipeline_stage::fragment_shader);

    signal.connect([image = std::move(image)](){});

    return texture;
}

texture::texture(std::uint32_t width, std::uint32_t height, tph::texture_usage usage, color_space space)
:m_texture{engine::instance().renderer(), width, height, format_from_color_space(space), usage}
{

}

texture::texture(std::uint32_t width, std::uint32_t height, tph::texture_usage usage, const tph::sampling_options& options, color_space space)
:m_texture{engine::instance().renderer(), width, height, options, format_from_color_space(space), usage}
{

}

texture::texture(const std::filesystem::path& file, const tph::sampling_options& sampling, color_space space)
:m_texture{make_texture(sampling, format_from_color_space(space), file)}
{

}

texture::texture(const std::string_view& data, const tph::sampling_options& sampling, color_space space)
:m_texture{make_texture(sampling, format_from_color_space(space), data)}
{

}

texture::texture(std::istream& stream, const tph::sampling_options& sampling, color_space space)
:m_texture{make_texture(sampling, format_from_color_space(space), stream)}
{

}

texture::texture(std::uint32_t width, std::uint32_t height, const std::uint8_t* rgba, const tph::sampling_options& sampling, color_space space)
:m_texture{make_texture(sampling, format_from_color_space(space), width, height, rgba)}
{

}

texture::texture(tph::image image, const tph::sampling_options& sampling, color_space space)
:m_texture{make_texture(sampling, format_from_color_space(space), std::move(image))}
{

}

texture::texture(tph::texture other)
:m_texture{std::move(other)}
{

}

}
