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

class device;
class surface;

struct swapchain_info
{
    std::uint32_t image_count{};
    std::uint32_t width{};
    std::uint32_t height{};
    tph::texture_format format{};
    tph::texture_usage usage{tph::texture_usage::color_attachment};
    tph::surface_transform transform{tph::surface_transform::identity};
    tph::surface_composite composite{tph::surface_composite::opaque};
    tph::present_mode present_mode{tph::present_mode::fifo};
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
    explicit swapchain(device& dev, surface& surf, const swapchain_info& info, optional_ref<tph::swapchain> old_swapchain = nullref);

    explicit swapchain(vulkan::swapchain swpchain, VkQueue present_queue, const swapchain_info& info, std::vector<tph::texture> textures) noexcept
    :m_swapchain{std::move(swpchain)}
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

    vulkan::device_context context() const noexcept
    {
        return m_swapchain.context();
    }

    swapchain_status acquire(optional_ref<semaphore> semaphr, optional_ref<fence> fen)
    {
        return acquire_impl(std::numeric_limits<std::uint64_t>::max(), semaphr, fen);
    }

    swapchain_status try_acquire(optional_ref<semaphore> semaphr, optional_ref<fence> fen)
    {
        return acquire_impl(0, semaphr, fen);
    }

    template<typename Rep, typename Period>
    swapchain_status acquire_for(const std::chrono::duration<Rep, Period>& timeout, optional_ref<semaphore> semaphr, optional_ref<fence> fen)
    {
        return acquire_impl(std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count(), semaphr, fen);
    }

    template<typename Clock, typename Duration>
    swapchain_status acquire_until(const std::chrono::time_point<Clock, Duration>& time, optional_ref<semaphore> semaphr, optional_ref<fence> fen)
    {
        const auto current_time{Clock::now()};
        if(time < current_time)
        {
            return try_acquire(semaphr, fen);
        }

        return acquire_impl(std::chrono::duration_cast<std::chrono::nanoseconds>(time - current_time).count(), semaphr, fen);
    }

    swapchain_status present(std::span<const std::reference_wrapper<semaphore>> wait_semaphrs);
    swapchain_status present(std::span<semaphore> wait_semaphrs);
    swapchain_status present(semaphore& wait_semaphr);

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
    swapchain_status acquire_impl(std::uint64_t timeout, optional_ref<semaphore> semaphr, optional_ref<fence> fen);

private:
    vulkan::swapchain m_swapchain{};
    VkQueue m_queue{};
    swapchain_info m_info{};
    std::vector<tph::texture> m_textures{};
    std::vector<tph::texture_view> m_texture_views{};
    std::uint32_t m_image_index{};
};

TEPHRA_API void set_object_name(device& dev, const swapchain& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const swapchain& swpchain) noexcept
{
    return swpchain.m_swapchain.device();
}

template<>
inline VkSwapchainKHR underlying_cast(const swapchain& swpchain) noexcept
{
    return swpchain.m_swapchain;
}

}

#endif
