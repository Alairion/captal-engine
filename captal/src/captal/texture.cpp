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

static tph::texture make_texture(const tph::sampling_options& sampling, tph::texture_format format, tph::image image)
{
    tph::texture texture{cpt::engine::instance().renderer(), static_cast<std::uint32_t>(image.width()), static_cast<std::uint32_t>(image.height()), sampling, format, tph::texture_usage::sampled | tph::texture_usage::transfer_destination};

    auto&& [command_buffer, signal] = cpt::engine::instance().begin_transfer();

    tph::cmd::transition(command_buffer, texture, tph::resource_access::none, tph::resource_access::transfer_write, tph::pipeline_stage::top_of_pipe, tph::pipeline_stage::transfer, tph::texture_layout::undefined, tph::texture_layout::transfer_destination_optimal);
    tph::cmd::copy(command_buffer, image, texture);
    tph::cmd::transition(command_buffer, texture, tph::resource_access::transfer_write, tph::resource_access::shader_read, tph::pipeline_stage::transfer, tph::pipeline_stage::fragment_shader, tph::texture_layout::transfer_destination_optimal, tph::texture_layout::shader_read_only_optimal);

    signal.connect([image = std::move(image)](){});

    return texture;
}

template<typename... Args>
static tph::texture make_texture(const tph::sampling_options& sampling, tph::texture_format format, Args&&... args)
{
    tph::image image{cpt::engine::instance().renderer(), std::forward<Args>(args)..., tph::image_usage::transfer_source};

    return make_texture(sampling, format, std::move(image));
}


texture::texture(const std::filesystem::path& file, const tph::sampling_options& sampling, color_space space)
:m_texture{make_texture(sampling, format_from_color_space(space), file)}
{

}

texture::texture(std::span<const std::uint8_t> data, const tph::sampling_options& sampling, color_space space)
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

tph::renderer& texture::get_renderer() noexcept
{
    return engine::instance().renderer();
}


cpt::texture_ptr texture_pool::default_load_callback(const std::filesystem::path& path, const tph::sampling_options& sampling, color_space space)
{
    return make_texture(path, sampling, space);
}

texture_pool::texture_pool()
:m_load_callback{default_load_callback}
{

}

texture_pool::texture_pool(load_callback_t load_callback)
:m_load_callback{std::move(load_callback)}
{

}

cpt::texture_ptr texture_pool::load(const std::filesystem::path& path, const tph::sampling_options& sampling, color_space space)
{
    return load(path, m_load_callback, sampling, space);
}

cpt::texture_ptr texture_pool::load(const std::filesystem::path& path, const load_callback_t& load_callback, const tph::sampling_options& sampling, color_space space)
{
    const auto it{m_pool.find(path)};
    if(it != std::end(m_pool))
    {
        return it->second;
    }

    return m_pool.emplace(std::make_pair(path, load_callback(path, sampling, space))).first->second;
}

cpt::texture_weak_ptr texture_pool::weak_load(const std::filesystem::path& path) const
{
    const auto it{m_pool.find(path)};
    if(it != std::end(m_pool))
    {
        return cpt::texture_weak_ptr{it->second};
    }

    return cpt::texture_weak_ptr{};
}

std::pair<cpt::texture_ptr, bool> texture_pool::emplace(std::filesystem::path path, texture_ptr texture)
{
    auto [it, success] = m_pool.emplace(std::make_pair(std::move(path), std::move(texture)));

    return std::make_pair(it->second, success);
}

void texture_pool::clear(std::size_t threshold)
{
    auto it = std::begin(m_pool);
    while(it != std::end(m_pool))
    {
        if(static_cast<std::size_t>(it->second.use_count()) <= threshold)
        {
            it = m_pool.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void texture_pool::remove(const std::filesystem::path& path)
{
    const auto it{m_pool.find(path)};
    if(it != std::end(m_pool))
    {
        m_pool.erase(it);
    }
}

void texture_pool::remove(const texture_ptr& texture)
{
    const auto predicate = [&texture](const std::pair<std::filesystem::path, texture_ptr>& pair)
    {
        return pair.second == texture;
    };

    const auto it{std::find_if(std::begin(m_pool), std::end(m_pool), predicate)};
    if(it != std::end(m_pool))
    {
        m_pool.erase(it);
    }
}

}
