#ifndef CAPTAL_TEXTURE_HPP_INCLUDED
#define CAPTAL_TEXTURE_HPP_INCLUDED

#include "config.hpp"

#include <optional>
#include <filesystem>
#include <istream>
#include <memory>

#include <tephra/image.hpp>
#include <tephra/texture.hpp>

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
    texture(std::uint32_t width, std::uint32_t height, tph::texture_usage usage, color_space space = color_space::srgb);
    texture(std::uint32_t width, std::uint32_t height, tph::texture_usage usage, const tph::sampling_options& options, color_space space = color_space::srgb);

    texture(const std::filesystem::path& file, const tph::sampling_options& sampling = tph::sampling_options{}, color_space space = color_space::srgb);
    texture(const std::string_view& data, const tph::sampling_options& sampling = tph::sampling_options{}, color_space space = color_space::srgb);
    texture(std::istream& stream, const tph::sampling_options& sampling = tph::sampling_options{}, color_space space = color_space::srgb);

    texture(std::uint32_t width, std::uint32_t height, const std::uint8_t* rgba, const tph::sampling_options& sampling = tph::sampling_options{}, color_space space = color_space::srgb);
    texture(tph::image image, const tph::sampling_options& sampling = tph::sampling_options{}, color_space space = color_space::srgb);

    template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<tph::texture, tph::renderer&, Args...>>>
    texture(Args&&... args) noexcept(std::is_nothrow_constructible_v<tph::texture, tph::renderer&, Args...>)
    :m_texture{get_renderer(), std::forward<Args>(args)...}
    {

    }

    virtual ~texture() = default;
    texture(const texture&) = delete;
    texture& operator=(const texture&) = delete;
    texture(texture&&) noexcept = default;
    texture& operator=(texture&&) noexcept = default;

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

}

#endif
