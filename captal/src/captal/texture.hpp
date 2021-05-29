#ifndef CAPTAL_TEXTURE_HPP_INCLUDED
#define CAPTAL_TEXTURE_HPP_INCLUDED

#include "config.hpp"

#include <optional>
#include <concepts>
#include <filesystem>
#include <istream>
#include <memory>

#include <tephra/image.hpp>
#include <tephra/texture.hpp>

#include <captal_foundation/math.hpp>

#include "asynchronous_resource.hpp"

namespace cpt
{

enum class color_space : std::uint32_t
{
    srgb = 0,
    linear = 1
};

class CAPTAL_API texture : public asynchronous_resource
{
public:
    texture() = default;

    template<typename... Args> requires std::constructible_from<tph::texture, tph::renderer&, Args...>
    explicit texture(Args&&... args)
    :m_texture{get_renderer(), std::forward<Args>(args)...}
    ,m_texture_view{get_renderer(), m_texture}
    ,m_sampler{get_renderer(), tph::sampler_info{}}
    {

    }

    template<typename... Args> requires std::constructible_from<tph::texture, tph::renderer&, Args...>
    explicit texture(const tph::sampler_info& sampler, Args&&... args)
    :m_texture{get_renderer(), std::forward<Args>(args)...}
    ,m_texture_view{get_renderer(), m_texture}
    ,m_sampler{get_renderer(), sampler}
    {

    }

    template<typename... Args> requires std::constructible_from<tph::texture, tph::renderer&, Args...>
    explicit texture(const tph::component_mapping& mapping, Args&&... args)
    :m_texture{get_renderer(), std::forward<Args>(args)...}
    ,m_texture_view{get_renderer(), m_texture, mapping}
    ,m_sampler{get_renderer(), tph::sampler_info{}}
    {

    }

    template<typename... Args> requires std::constructible_from<tph::texture, tph::renderer&, Args...>
    explicit texture(const tph::sampler_info& sampler, const tph::component_mapping& mapping, Args&&... args)
    :m_texture{get_renderer(), std::forward<Args>(args)...}
    ,m_texture_view{get_renderer(), m_texture, mapping}
    ,m_sampler{get_renderer(), sampler}
    {

    }

    explicit texture(tph::texture&& texture, tph::texture_view&& texture_view, tph::sampler&& sampler) noexcept
    :m_texture{std::move(texture)}
    ,m_texture_view{std::move(texture_view)}
    ,m_sampler{std::move(sampler)}
    {

    }

    virtual ~texture() = default;
    texture(const texture&) = delete;
    texture& operator=(const texture&) = delete;
    texture(texture&&) noexcept = default;
    texture& operator=(texture&&) noexcept = default;

    std::uint32_t width() const noexcept
    {
        return m_texture.width();
    }

    std::uint32_t height() const noexcept
    {
        return m_texture.height();
    }

    std::uint32_t depth() const noexcept
    {
        return m_texture.depth();
    }

    bool is_cubemap() const noexcept
    {
        return m_texture.is_cubemap();
    }

    tph::texture_format format() const noexcept
    {
        return m_texture.format();
    }

    tph::texture_aspect aspect() const noexcept
    {
        return m_texture.aspect();
    }

    std::uint32_t mip_levels() const noexcept
    {
        return m_texture.mip_levels();
    }

    std::uint32_t array_layers() const noexcept
    {
        return m_texture.array_layers();
    }

    tph::sample_count sample_count() const noexcept
    {
        return m_texture.sample_count();
    }

    tph::texture& get_texture() noexcept
    {
        return m_texture;
    }

    const tph::texture& get_texture() const noexcept
    {
        return m_texture;
    }

    tph::texture_view& get_texture_view() noexcept
    {
        return m_texture_view;
    }

    const tph::texture_view& get_texture_view() const noexcept
    {
        return m_texture_view;
    }

    tph::sampler& get_sampler() noexcept
    {
        return m_sampler;
    }

    const tph::sampler& get_sampler() const noexcept
    {
        return m_sampler;
    }

#ifdef CAPTAL_DEBUG
    void set_name(std::string_view name);
#else
    void set_name(std::string_view name [[maybe_unused]]) const noexcept
    {

    }
#endif

private:
    static tph::renderer& get_renderer() noexcept;

private:
    tph::texture m_texture{};
    tph::texture_view m_texture_view{};
    tph::sampler m_sampler{};
};

using texture_ptr = std::shared_ptr<texture>;
using texture_weak_ptr = std::weak_ptr<texture>;

template<typename... Args> requires std::constructible_from<texture, Args...>
texture_ptr make_texture(Args&&... args)
{
    return std::make_shared<texture>(std::forward<Args>(args)...);
}

CAPTAL_API texture_ptr make_texture(const std::filesystem::path& file, const tph::sampler_info& sampling = tph::sampler_info{}, color_space space = color_space::srgb);
CAPTAL_API texture_ptr make_texture(std::span<const std::uint8_t> data, const tph::sampler_info& sampling = tph::sampler_info{}, color_space space = color_space::srgb);
CAPTAL_API texture_ptr make_texture(std::istream& stream, const tph::sampler_info& sampling = tph::sampler_info{}, color_space space = color_space::srgb);
CAPTAL_API texture_ptr make_texture(std::uint32_t width, std::uint32_t height, const std::uint8_t* rgba, const tph::sampler_info& sampling = tph::sampler_info{}, color_space space = color_space::srgb);
CAPTAL_API texture_ptr make_texture(tph::image&& image, const tph::sampler_info& sampling = tph::sampler_info{}, color_space space = color_space::srgb);

class CAPTAL_API texture_pool
{
    struct path_hash
    {
        std::size_t operator()(const std::filesystem::path& path) const noexcept
        {
            return std::filesystem::hash_value(path);
        }
    };

public:
    static cpt::texture_ptr default_load_callback(const std::filesystem::path& path, const tph::sampler_info& sampling, color_space space);

public:
    using load_callback_t = std::function<cpt::texture_ptr(const std::filesystem::path& path, const tph::sampler_info& sampling, color_space space)>;

public:
    texture_pool();
    explicit texture_pool(load_callback_t load_callback);

    ~texture_pool() = default;
    texture_pool(const texture_pool&) = delete;
    texture_pool& operator=(const texture_pool&) = default;
    texture_pool(texture_pool&&) noexcept = default;
    texture_pool& operator=(texture_pool&&) noexcept = default;

    cpt::texture_ptr load(const std::filesystem::path& path, const tph::sampler_info& sampling = tph::sampler_info{}, color_space space = color_space::srgb);
    cpt::texture_ptr load(const std::filesystem::path& path, const load_callback_t& load_callback, const tph::sampler_info& sampling = tph::sampler_info{}, color_space space = color_space::srgb);
    cpt::texture_weak_ptr weak_load(const std::filesystem::path& path) const;
    std::pair<cpt::texture_ptr, bool> emplace(std::filesystem::path path, texture_ptr texture);

    void clear(std::size_t threshold = 1);

    template<typename Predicate>
    void clear_if(Predicate predicate)
    {
        auto it = std::begin(m_pool);
        while(it != std::end(m_pool))
        {
            if(predicate(it->first, it->second))
            {
                it = m_pool.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void remove(const std::filesystem::path& path);
    void remove(const texture_ptr& texture);

    void set_load_callback(load_callback_t new_callback)
    {
        m_load_callback = std::move(new_callback);
    }

    const load_callback_t& load_callback() const noexcept
    {
        return m_load_callback;
    }

private:
    std::unordered_map<std::filesystem::path, texture_ptr, path_hash> m_pool{};
    load_callback_t m_load_callback{};
};

class CAPTAL_API tileset
{
public:
    struct texture_rect
    {
        vec2f top_left{};
        vec2f bottom_right{};
    };

public:
    tileset() = default;

    explicit tileset(texture_ptr texture, std::uint32_t tile_width, std::uint32_t tile_height)
    :m_texture{std::move(texture)}
    ,m_tile_width{tile_width}
    ,m_tile_height{tile_height}
    {

    }

    ~tileset() = default;
    tileset(const tileset&) = delete;
    tileset& operator=(const tileset&) = delete;
    tileset(tileset&&) noexcept = default;
    tileset& operator=(tileset&&) noexcept = default;

    texture_rect compute_rect(std::uint32_t index) const noexcept
    {
        return compute_rect(index % col_count(), index / row_count());
    }

    texture_rect compute_rect(std::uint32_t col, std::uint32_t row) const noexcept
    {
        texture_rect output{};

        const float width {static_cast<float>(m_texture->width())};
        const float height{static_cast<float>(m_texture->height())};

        output.top_left     = vec2f{static_cast<float>(( col      * m_tile_width)) / width, static_cast<float>(( row      * m_tile_height)) / height};
        output.bottom_right = vec2f{static_cast<float>(((col + 1) * m_tile_width)) / width, static_cast<float>(((row + 1) * m_tile_height)) / height};

        return output;
    }

    std::uint32_t tile_width() const noexcept
    {
        return m_tile_width;
    }

    std::uint32_t tile_height() const noexcept
    {
        return m_tile_height;
    }

    std::uint32_t col_count() const noexcept
    {
        return m_texture->width() / m_tile_width;
    }

    std::uint32_t row_count() const noexcept
    {
        return m_texture->height() / m_tile_height;
    }

    const texture_ptr& texture() const noexcept
    {
        return m_texture;
    }

private:
    texture_ptr m_texture{};
    std::uint32_t m_tile_width{};
    std::uint32_t m_tile_height{};
};

}

#endif
