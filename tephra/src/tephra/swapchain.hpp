#ifndef TEPHRA_SWAPCHAIN_HPP_INCLUDED
#define TEPHRA_SWAPCHAIN_HPP_INCLUDED

#include "config.hpp"

#include <span>

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
    not_ready = 4
};

class TEPHRA_API swapchain
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr swapchain() = default;
    explicit swapchain(renderer& renderer, surface& surface, const swapchain_info& info, optional_ref<swapchain> old_swapchain = nullref);

    explicit swapchain(vulkan::swapchain swapchain, VkQueue present_queue, const swapchain_info& info, std::vector<tph::texture> textures) noexcept
    :m_swapchain{std::move(swapchain)}
    ,m_queue{present_queue}
    ,m_info{info}
    ,m_textures{std::move(textures)}
    {

    }

    ~swapchain() = default;
    swapchain(const swapchain&) = delete;
    swapchain& operator=(const swapchain&) = delete;
    swapchain(swapchain&&) noexcept = default;
    swapchain& operator=(swapchain&&) noexcept = default;

    swapchain_status acquire(optional_ref<semaphore> semaphore, optional_ref<fence> fence)
    {
        return acquire_impl(std::numeric_limits<std::uint64_t>::max(), semaphore, fence);
    }

    swapchain_status try_acquire(optional_ref<semaphore> semaphore, optional_ref<fence> fence)
    {
        return acquire_impl(0, semaphore, fence);
    }

    template<typename Rep, typename Period>
    swapchain_status acquire_for(const std::chrono::duration<Rep, Period>& timeout, optional_ref<semaphore> semaphore, optional_ref<fence> fence)
    {
        return acquire_impl(std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count(), semaphore, fence);
    }

    template<typename Clock, typename Duration>
    swapchain_status acquire_until(const std::chrono::time_point<Clock, Duration>& time, optional_ref<semaphore> semaphore, optional_ref<fence> fence)
    {
        const auto current_time{Clock::now()};
        if(time < current_time)
        {
            return try_acquire(semaphore, fence);
        }

        return acquire_impl(std::chrono::duration_cast<std::chrono::nanoseconds>(time - current_time).count(), semaphore, fence);
    }

    swapchain_status present(std::span<const std::reference_wrapper<semaphore>> wait_semaphores);
    swapchain_status present(std::span<semaphore> wait_semaphores);
    swapchain_status present(semaphore& wait_semaphore);

    const swapchain_info& info() const noexcept
    {
        return m_info;
    }

    std::span<tph::texture> textures() noexcept
    {
        return m_textures;
    }

    std::span<const tph::texture> textures() const noexcept
    {
        return m_textures;
    }

    std::span<tph::texture_view> texture_views() noexcept
    {
        return m_texture_views;
    }

    std::span<const tph::texture_view> texture_views() const noexcept
    {
        return m_texture_views;
    }

    std::uint32_t image_index() const noexcept
    {
        return m_image_index;
    }

private:
    swapchain_status acquire_impl(std::uint64_t timeout, optional_ref<semaphore> semaphore, optional_ref<fence> fence);

private:
    vulkan::swapchain m_swapchain{};
    VkQueue m_queue{};
    swapchain_info m_info{};
    std::vector<tph::texture> m_textures{};
    std::vector<tph::texture_view> m_texture_views{};
    std::uint32_t m_image_index{};
};

TEPHRA_API void set_object_name(renderer& renderer, const swapchain& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const swapchain& swapchain) noexcept
{
    return swapchain.m_swapchain.device();
}

template<>
inline VkSwapchainKHR underlying_cast(const swapchain& swapchain) noexcept
{
    return swapchain.m_swapchain;
}

}

#endif
