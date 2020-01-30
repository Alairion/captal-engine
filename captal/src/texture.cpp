#include "texture.hpp"

#include "engine.hpp"

namespace cpt
{

static tph::texture make_texture(std::string_view file, tph::load_from_file_t, tph::sampling_options sampling)
{
    tph::image image{cpt::engine::instance().renderer(), file, tph::load_from_file, tph::image_usage::transfer_source};
    tph::texture texture{cpt::engine::instance().renderer(), static_cast<std::uint32_t>(image.width()), static_cast<std::uint32_t>(image.height()), sampling, tph::texture_usage::sampled | tph::texture_usage::transfer_destination};

    auto&& [command_buffer, signal] = cpt::engine::instance().begin_transfer();

    tph::cmd::copy(command_buffer, image, texture);
    tph::cmd::prepare(command_buffer, texture, tph::pipeline_stage::fragment_shader);

    signal.connect([image = std::move(image)](){});

    return texture;
}

static tph::texture make_texture(std::string_view data, tph::load_from_memory_t, tph::sampling_options sampling)
{
    tph::image image{cpt::engine::instance().renderer(), data, tph::load_from_memory, tph::image_usage::transfer_source};
    tph::texture texture{cpt::engine::instance().renderer(), static_cast<std::uint32_t>(image.width()), static_cast<std::uint32_t>(image.height()), sampling, tph::texture_usage::sampled | tph::texture_usage::transfer_destination};

    auto&& [command_buffer, signal] = cpt::engine::instance().begin_transfer();

    tph::cmd::copy(command_buffer, image, texture);
    tph::cmd::prepare(command_buffer, texture, tph::pipeline_stage::fragment_shader);

    signal.connect([image = std::move(image)](){});

    return texture;
}

static tph::texture make_texture(std::uint32_t width, std::uint32_t height, const std::uint8_t* rgba, tph::sampling_options sampling)
{
    tph::image image{cpt::engine::instance().renderer(), width, height, rgba, tph::image_usage::transfer_source};
    tph::texture texture{cpt::engine::instance().renderer(), static_cast<std::uint32_t>(image.width()), static_cast<std::uint32_t>(image.height()), sampling, tph::texture_usage::sampled | tph::texture_usage::transfer_destination};

    auto&& [command_buffer, signal] = cpt::engine::instance().begin_transfer();

    tph::cmd::copy(command_buffer, image, texture);
    tph::cmd::prepare(command_buffer, texture, tph::pipeline_stage::fragment_shader);

    signal.connect([image = std::move(image)](){});

    return texture;
}

static tph::texture make_texture(tph::image image, tph::sampling_options sampling)
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

texture::texture(std::string_view file, tph::load_from_file_t, tph::sampling_options sampling)
:m_texture{make_texture(file, tph::load_from_file, sampling)}
{

}

texture::texture(std::string_view data, tph::load_from_memory_t, tph::sampling_options sampling)
:m_texture{make_texture(data, tph::load_from_memory, sampling)}
{

}

texture::texture(std::uint32_t width, std::uint32_t height, const std::uint8_t* rgba, tph::sampling_options sampling)
:m_texture{make_texture(width, height, rgba, sampling)}
{

}

texture::texture(tph::image image, tph::sampling_options sampling)
:m_texture{make_texture(std::move(image), sampling)}
{

}


}
