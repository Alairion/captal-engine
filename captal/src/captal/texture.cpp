#include "texture.hpp"

#include "engine.hpp"

namespace cpt
{

#ifdef CAPTAL_DEBUG
void texture::set_name(std::string_view name)
{
    tph::set_object_name(engine::instance().renderer(), m_texture, std::string{name});
}
#endif

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
static tph::image make_image(Args&&... args)
{
    return tph::image{cpt::engine::instance().renderer(), std::forward<Args>(args)..., tph::image_usage::transfer_source};
}

static texture_ptr make_texture_impl(const tph::sampling_options& sampling, tph::texture_format format, tph::image image)
{
    const tph::texture_info info{format, tph::texture_usage::sampled | tph::texture_usage::transfer_destination};
    texture_ptr texture{make_texture(static_cast<std::uint32_t>(image.width()), static_cast<std::uint32_t>(image.height()), info, sampling)};

    auto&& [command_buffer, signal, keeper] = cpt::engine::instance().begin_transfer();

    tph::cmd::transition(command_buffer, texture->get_texture(),
                         tph::resource_access::none, tph::resource_access::transfer_write,
                         tph::pipeline_stage::top_of_pipe, tph::pipeline_stage::transfer,
                         tph::texture_layout::undefined, tph::texture_layout::transfer_destination_optimal);

    tph::cmd::copy(command_buffer, image, texture->get_texture());

    tph::cmd::transition(command_buffer, texture->get_texture(),
                         tph::resource_access::transfer_write, tph::resource_access::shader_read,
                         tph::pipeline_stage::transfer, tph::pipeline_stage::fragment_shader,
                         tph::texture_layout::transfer_destination_optimal,  tph::texture_layout::shader_read_only_optimal);

    signal.connect([image = std::move(image)](){});
    keeper.keep(texture);

    return texture;
}

texture_ptr make_texture(const std::filesystem::path& file, const tph::sampling_options& sampling, color_space space)
{
    return make_texture_impl(sampling, format_from_color_space(space), make_image(file));
}

texture_ptr make_texture(std::span<const std::uint8_t> data, const tph::sampling_options& sampling, color_space space)
{
    return make_texture_impl(sampling, format_from_color_space(space), make_image(data));
}

texture_ptr make_texture(std::istream& stream, const tph::sampling_options& sampling, color_space space)
{
    return make_texture_impl(sampling, format_from_color_space(space), make_image(stream));
}

texture_ptr make_texture(std::uint32_t width, std::uint32_t height, const std::uint8_t* rgba, const tph::sampling_options& sampling, color_space space)
{
    return make_texture_impl(sampling, format_from_color_space(space), make_image(width, height, rgba));
}

texture_ptr make_texture(tph::image image, const tph::sampling_options& sampling, color_space space)
{
    return make_texture_impl(sampling, format_from_color_space(space), std::move(image));
}

tph::renderer& texture::get_renderer() noexcept
{
    return engine::instance().renderer();
}


cpt::texture_ptr texture_pool::default_load_callback(const std::filesystem::path& path, const tph::sampling_options& sampling, color_space space)
{
    auto output{make_texture(path, sampling, space)};

#ifdef CAPTAL_DEBUG
   output->set_name(convert_to<narrow>(path.u8string()));
#endif

    return output;
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
