#ifndef TEPHRA_SWAPCHAIN_HPP_INCLUDED
#define TEPHRA_SWAPCHAIN_HPP_INCLUDED

#include "config.hpp"

#include "vulkan/vulkan.hpp"

#include "enumerations.hpp"
#include "texture.hpp"
#include "surface.hpp"
#include "synchronization.hpp"

namespace tph
{

class renderer;
class surface;

struct swapchain_info
{
    std::uint32_t image_count{};
    std::uint32_t width{};
    std::uint32_t height{};
    tph::texture_format format{};
    tph::texture_usage usage{tph::texture_usage::color_attachment};
    surface_transform transform{surface_transform::identity};
    surface_composite composite{surface_composite::opaque};
    present_mode present_mode{present_mode::fifo};
    bool clipping{true};
};

enum class swapchain_status : std::uint32_t
{
    valid = 0,
    suboptimal = 1,
    out_of_date = 2,
    surface_lost = 3,
};

class swapchain
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr swapchain() = default;

    swapchain(renderer& renderer, surface& surface, const swapchain_info& info, optional_ref<swapchain> old_swapchain = nullref);

    ~swapchain() = default;
    swapchain(const swapchain&) = delete;
    swapchain& operator=(const swapchain&) = delete;
    swapchain(swapchain&&) noexcept = default;
    swapchain& operator=(swapchain&&) noexcept = default;

    swapchain_status acquire(optional_ref<semaphore> semaphore, optional_ref<fence> fence);
    swapchain_status present(const std::vector<std::reference_wrapper<semaphore>>& wait_semaphores);
    swapchain_status present(semaphore& wait_semaphore);

    const swapchain_info& info() const noexcept
    {
        return m_info;
    }

    tph::texture& texture(std::size_t index) noexcept
    {
        return m_textures[index];
    }

    const tph::texture& texture(std::size_t index) const noexcept
    {
        return m_textures[index];
    }

private:
    VkDevice m_device{};
    VkQueue m_queue{};
    vulkan::swapchain m_swapchain{};
    swapchain_info m_info{};
    std::vector<tph::texture> m_textures{};
    std::uint32_t m_image_index{};
};

template<>
inline VkSwapchainKHR underlying_cast(const swapchain& swapchain) noexcept
{
    return swapchain.m_swapchain;
}

}

#endif
