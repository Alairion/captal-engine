#include "texture_pool.hpp"

namespace cpt
{

static cpt::texture_ptr default_load_callback(const std::filesystem::path& path, const tph::sampling_options& sampling)
{
    return make_texture(path, sampling);
}

texture_pool::texture_pool() noexcept
:m_load_callback{default_load_callback}
{

}

texture_pool::texture_pool(load_callback load_callback) noexcept
:m_load_callback{std::move(load_callback)}
{

}

cpt::texture_ptr texture_pool::load(const std::filesystem::path& path, const tph::sampling_options& sampling)
{
    return load(path, m_load_callback, sampling);
}

cpt::texture_ptr texture_pool::load(const std::filesystem::path& path, const load_callback& load_callback, const tph::sampling_options& sampling)
{
    const auto it{m_pool.find(path)};
    if(it != std::end(m_pool))
        return it->second;

    return m_pool.emplace(std::make_pair(path, load_callback(path, sampling))).first->second;
}

cpt::texture_weak_ptr texture_pool::weak_load(const std::filesystem::path& path)
{
    const auto it{m_pool.find(path)};
    if(it != std::end(m_pool))
        return cpt::texture_weak_ptr{it->second};

    return cpt::texture_weak_ptr{};
}

}
