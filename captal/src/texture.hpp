#ifndef CAPTAL_TEXTURE_HPP_INCLUDED
#define CAPTAL_TEXTURE_HPP_INCLUDED

#include "config.hpp"

#include <optional>
#include <memory>

#include <tephra/image.hpp>
#include <tephra/texture.hpp>

#include "asynchronous_resource.hpp"

namespace cpt
{

class CAPTAL_API texture : public asynchronous_resource
{
public:
    texture() = default;
    texture(std::uint32_t width, std::uint32_t height, tph::texture_usage usage);
    texture(std::uint32_t width, std::uint32_t height, const tph::sampling_options& options, tph::texture_usage usage);
    texture(std::uint32_t width, std::uint32_t height, std::uint32_t depth, tph::texture_usage usage);
    texture(std::uint32_t width, std::uint32_t height, std::uint32_t depth, const tph::sampling_options& options, tph::texture_usage usage);
    texture(tph::texture other);
    texture(std::string_view file, cpt::load_from_file_t, const tph::sampling_options& sampling = tph::sampling_options{});
    texture(std::string_view data, cpt::load_from_memory_t, const tph::sampling_options& sampling = tph::sampling_options{});
    texture(std::uint32_t width, std::uint32_t height, const std::uint8_t* rgba, const tph::sampling_options& sampling = tph::sampling_options{});
    texture(tph::image image, const tph::sampling_options& sampling = tph::sampling_options{});

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
