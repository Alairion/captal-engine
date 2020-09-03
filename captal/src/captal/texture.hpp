#ifndef CAPTAL_TEXTURE_HPP_INCLUDED
#define CAPTAL_TEXTURE_HPP_INCLUDED

#include "config.hpp"

#include <optional>
#include <filesystem>
#include <istream>
#include <memory>

#include <tephra/image.hpp>
#include <tephra/texture.hpp>

#include <glm/vec2.hpp>

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

    texture(const std::filesystem::path& file, const tph::sampling_options& sampling = tph::sampling_options{}, color_space space = color_space::srgb);
    texture(std::span<const std::uint8_t> data, const tph::sampling_options& sampling = tph::sampling_options{}, color_space space = color_space::srgb);
    texture(std::istream& stream, const tph::sampling_options& sampling = tph::sampling_options{}, color_space space = color_space::srgb);
    texture(std::uint32_t width, std::uint32_t height, const std::uint8_t* rgba, const tph::sampling_options& sampling = tph::sampling_options{}, color_space space = color_space::srgb);
    texture(tph::image image, const tph::sampling_options& sampling = tph::sampling_options{}, color_space space = color_space::srgb);

    template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<tph::texture, tph::renderer&, Args...>>>
    texture(Args&&... args)
    :m_texture{get_renderer(), std::forward<Args>(args)...}
    {

    }

    virtual ~texture() = default;
    texture(const texture&) = delete;
    texture& operator=(const texture&) = delete;
    texture(texture&&) noexcept = delete;
    texture& operator=(texture&&) noexcept = delete;

    tph::texture::size_type width() const noexcept
    {
        return m_texture.width();
    }

    tph::texture::size_type height() const noexcept
    {
        return m_texture.height();
    }

    tph::texture::size_type depth() const noexcept
    {
        return m_texture.depth();
    }

    tph::texture_format format() const noexcept
    {
        return m_texture.format();
    }

    tph::texture_aspect aspect() const noexcept
    {
        return m_texture.aspect();
    }

    tph::texture& get_texture() noexcept
    {
        return m_texture;
    }

    const tph::texture& get_texture() const noexcept
    {
        return m_texture;
    }

private:
    static tph::renderer& get_renderer() noexcept;

private:
    tph::texture m_texture{};
};

using texture_ptr = std::shared_ptr<texture>;
using texture_weak_ptr = std::weak_ptr<texture>;

template<typename... Args>
texture_ptr make_texture(Args&&... args)
{
    return std::make_shared<texture>(std::forward<Args>(args)...);
}

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
    static cpt::texture_ptr default_load_callback(const std::filesystem::path& path, const tph::sampling_options& sampling, color_space space);

public:
    using load_callback_t = std::function<cpt::texture_ptr(const std::filesystem::path& path, const tph::sampling_options& sampling, color_space space)>;

public:
    texture_pool();
    explicit texture_pool(load_callback_t load_callback);
    ~texture_pool() = default;
    texture_pool(const texture_pool&) = delete;
    texture_pool& operator=(const texture_pool&) = default;
    texture_pool(texture_pool&&) noexcept = default;
    texture_pool& operator=(texture_pool&&) noexcept = default;

    cpt::texture_ptr load(const std::filesystem::path& path, const tph::sampling_options& sampling = tph::sampling_options{}, color_space space = color_space::srgb);
    cpt::texture_ptr load(const std::filesystem::path& path, const load_callback_t& load_callback, const tph::sampling_options& sampling = tph::sampling_options{}, color_space space = color_space::srgb);
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
        glm::vec2 top_left{};
        glm::vec2 bottom_right{};
    };

public:
    tileset() = default;

    tileset(texture_ptr texture, std::uint32_t tile_width, std::uint32_t tile_height)
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

        const float width{static_cast<float>(m_texture->width())};
        const float height{static_cast<float>(m_texture->height())};

        output.top_left     = glm::vec2{static_cast<float>(( col      * m_tile_width)) / width, static_cast<float>(( row      * m_tile_height)) / height};
        output.bottom_right = glm::vec2{static_cast<float>(((col + 1) * m_tile_width)) / width, static_cast<float>(((row + 1) * m_tile_height)) / height};

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
