#include "texture.hpp"

#include "engine.hpp"

namespace cpt
{

template<typename... Args>
static tph::texture make_texture(const tph::sampling_options& sampling, Args&&... args)
{
    tph::image image{cpt::engine::instance().renderer(), std::forward<Args>(args)..., tph::image_usage::transfer_source};
    tph::texture texture{cpt::engine::instance().renderer(), static_cast<std::uint32_t>(image.width()), static_cast<std::uint32_t>(image.height()), sampling, tph::texture_usage::sampled | tph::texture_usage::transfer_destination};

    auto&& [command_buffer, signal] = cpt::engine::instance().begin_transfer();

    tph::cmd::copy(command_buffer, image, texture);
    tph::cmd::prepare(command_buffer, texture, tph::pipeline_stage::fragment_shader);

    signal.connect([image = std::move(image)](){});

    return texture;
}

static tph::texture make_texture(const tph::sampling_options& sampling, tph::image image)
{
    tph::texture texture{cpt::engine::instance().renderer(), static_cast<std::uint32_t>(image.width()), static_cast<std::uint32_t>(image.height()), sampling, tph::texture_usage::sampled | tph::texture_usage::transfer_destination};

    auto&& [command_buffer, signal] = cpt::engine::instance().begin_transfer();

    tph::cmd::copy(command_buffer, image, texture);
    tph::cmd::prepare(command_buffer, texture, tph::pipeline_stage::fragment_shader);

    signal.connect([image = std::move(image)](){});

    return texture;
}

texture::texture(std::uint32_t width, std::uint32_t height, tph::texture_usage usage)
:m_texture{engine::instance().renderer(), width, height, usage}
{

}

texture::texture(std::uint32_t width, std::uint32_t height, const tph::sampling_options& options, tph::texture_usage usage)
:m_texture{engine::instance().renderer(), width, height, options, usage}
{

}

texture::texture(std::uint32_t width, std::uint32_t height, std::uint32_t depth, tph::texture_usage usage)
:m_texture{engine::instance().renderer(), width, height, depth, usage}
{

}

texture::texture(std::uint32_t width, std::uint32_t height, std::uint32_t depth, const tph::sampling_options& options, tph::texture_usage usage)
:m_texture{engine::instance().renderer(), width, height, depth, options, usage}
{

}

texture::texture(tph::texture other)
:m_texture{std::move(other)}
{

}

texture::texture(const std::filesystem::path& file, const tph::sampling_options& sampling)
:m_texture{make_texture(sampling, file)}
{

}

texture::texture(std::string_view data, const tph::sampling_options& sampling)
:m_texture{make_texture(sampling, data)}
{

}

texture::texture(std::istream& stream, const tph::sampling_options& sampling)
:m_texture{make_texture(sampling, stream)}
{

}

texture::texture(std::uint32_t width, std::uint32_t height, const std::uint8_t* rgba, const tph::sampling_options& sampling)
:m_texture{make_texture(sampling, width, height, rgba)}
{

}

texture::texture(tph::image image, const tph::sampling_options& sampling)
:m_texture{make_texture(sampling, std::move(image))}
{

}


}
