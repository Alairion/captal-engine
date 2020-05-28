#include "texture_pool.hpp"

namespace cpt
{

cpt::texture_ptr texture_pool::default_load_callback(const std::filesystem::path& path, const tph::sampling_options& sampling, color_space space)
{
    return make_texture(path, sampling, space);
}

texture_pool::texture_pool() noexcept
:m_load_callback{default_load_callback}
{

}

texture_pool::texture_pool(load_callback_t load_callback) noexcept
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
