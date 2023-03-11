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

#include "texture.hpp"

#include "engine.hpp"

namespace cpt
{

#ifdef CAPTAL_DEBUG
void texture::set_name(std::string_view name)
{
    const std::string string_name{name};

    tph::set_object_name(engine::instance().device(), m_texture, string_name);
    tph::set_object_name(engine::instance().device(), m_texture_view, string_name + " view");
    tph::set_object_name(engine::instance().device(), m_sampler, string_name + " sampler");
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
    tph::image output{engine::instance().device(), std::forward<Args>(args)..., tph::image_usage::transfer_src};

#ifdef CAPTAL_DEBUG
    tph::set_object_name(engine::instance().device(), output, "transfer image");
#endif

    return output;
}

static texture_ptr make_texture_impl(const tph::sampler_info& sampling, tph::texture_format format, tph::image image)
{
    const tph::texture_info info{format, tph::texture_usage::sampled | tph::texture_usage::transfer_dest};
    texture_ptr texture{make_texture(sampling, static_cast<std::uint32_t>(image.width()), static_cast<std::uint32_t>(image.height()), info)};

    auto&& [buffer, signal, keeper] = cpt::engine::instance().begin_transfer();

    tph::texture_memory_barrier barrier{texture->get_texture()};
    barrier.src_access  = tph::resource_access::none;
    barrier.dest_access = tph::resource_access::transfer_write;
    barrier.old_layout  = tph::texture_layout::undefined;
    barrier.new_layout  = tph::texture_layout::transfer_dest_optimal;

    tph::cmd::pipeline_barrier(buffer, tph::pipeline_stage::top_of_pipe, tph::pipeline_stage::transfer, tph::dependency_flags::none, {}, {}, std::span{&barrier, 1});

    tph::image_texture_copy region{};
    region.texture_size.width  = static_cast<std::uint32_t>(image.width());
    region.texture_size.height = static_cast<std::uint32_t>(image.height());

    tph::cmd::copy(buffer, image, texture->get_texture(), region);

    barrier.src_access  = tph::resource_access::transfer_write;
    barrier.dest_access = tph::resource_access::shader_read;
    barrier.old_layout  = tph::texture_layout::transfer_dest_optimal;
    barrier.new_layout  = tph::texture_layout::shader_read_only_optimal;

    tph::cmd::pipeline_barrier(buffer, tph::pipeline_stage::transfer, tph::pipeline_stage::fragment_shader, tph::dependency_flags::none, {}, {}, std::span{&barrier, 1});

    signal.connect([image = std::move(image)](){});
    keeper.keep(texture);

    return texture;
}

texture_ptr make_texture(const std::filesystem::path& file, const tph::sampler_info& sampling, color_space space)
{
    return make_texture_impl(sampling, format_from_color_space(space), make_image(file));
}

texture_ptr make_texture(std::span<const std::uint8_t> data, const tph::sampler_info& sampling, color_space space)
{
    return make_texture_impl(sampling, format_from_color_space(space), make_image(data));
}

texture_ptr make_texture(std::istream& stream, const tph::sampler_info& sampling, color_space space)
{
    return make_texture_impl(sampling, format_from_color_space(space), make_image(stream));
}

texture_ptr make_texture(std::uint32_t width, std::uint32_t height, const std::uint8_t* rgba, const tph::sampler_info& sampling, color_space space)
{
    return make_texture_impl(sampling, format_from_color_space(space), make_image(width, height, rgba));
}

texture_ptr make_texture(tph::image&& image, const tph::sampler_info& sampling, color_space space)
{
    return make_texture_impl(sampling, format_from_color_space(space), std::move(image));
}

tph::device& texture::get_device() noexcept
{
    return engine::instance().device();
}


cpt::texture_ptr texture_pool::default_load_callback(const std::filesystem::path& path, const tph::sampler_info& sampling, color_space space)
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

cpt::texture_ptr texture_pool::load(const std::filesystem::path& path, const tph::sampler_info& sampling, color_space space)
{
    return load(path, m_load_callback, sampling, space);
}

cpt::texture_ptr texture_pool::load(const std::filesystem::path& path, const load_callback_t& load_callback, const tph::sampler_info& sampling, color_space space)
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
