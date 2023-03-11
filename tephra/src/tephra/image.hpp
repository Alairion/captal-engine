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

#ifndef TEPHRA_IMAGE_HPP_INCLUDED
#define TEPHRA_IMAGE_HPP_INCLUDED

#include "config.hpp"

#include <filesystem>

#include "vulkan/vulkan.hpp"
#include "vulkan/memory.hpp"

#include "buffer.hpp"

namespace tph
{

class device;

enum class image_usage : std::uint32_t
{
    none = 0,
    transfer_src = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    transfer_dest = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    persistant_mapping = 0x80000000,
};

struct pixel
{
    std::uint8_t red{};
    std::uint8_t green{};
    std::uint8_t blue{};
    std::uint8_t alpha{};
};

constexpr bool operator==(const pixel& left, const pixel& right) noexcept
{
    return left.red == right.red && left.green == right.green && left.blue == right.blue && left.alpha == right.alpha;
}

constexpr bool operator!=(const pixel& left, const pixel& right) noexcept
{
    return !(left == right);
}

enum class image_format
{
    png = 0,
    bmp = 1,
    tga = 2,
    jpg = 3,
};

class TEPHRA_API image
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    using value_type = pixel;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = pixel*;
    using const_pointer = const pixel*;
    using reference = pixel&;
    using const_reference = const pixel&;
    using iterator = pixel*;
    using const_iterator = const pixel*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
    constexpr image() = default;
    explicit image(device& dev, const std::filesystem::path& file, image_usage usage);
    explicit image(device& dev, std::span<const std::uint8_t> data, image_usage usage);
    explicit image(device& dev, std::istream& stream, image_usage usage);
    explicit image(device& dev, size_type width, size_type height, const std::uint8_t* data, image_usage usage);
    explicit image(device& dev, size_type width, size_type height, image_usage usage);

    ~image() = default;
    image(const image&) = delete;
    image& operator=(const image&) = delete;
    image(image&& other) noexcept = default;
    image& operator=(image&& other) noexcept = default;

    std::vector<std::uint8_t> write(image_format format, std::int32_t quality = 85) const;
    void write(const std::filesystem::path& file, image_format format, std::int32_t quality = 85) const;

    void map();
    void unmap() noexcept;

    buffer to_buffer() noexcept;

    vulkan::device_context context() const noexcept
    {
        return m_buffer.context();
    }

    pointer data() noexcept
    {
        return reinterpret_cast<pointer>(m_map);
    }

    const_pointer data() const noexcept
    {
        return reinterpret_cast<const_pointer>(m_map);
    }

    iterator begin() noexcept
    {
        return reinterpret_cast<iterator>(m_map);
    }

    const_iterator begin() const noexcept
    {
        return reinterpret_cast<const_iterator>(m_map);
    }

    const_iterator cbegin() const noexcept
    {
        return reinterpret_cast<const_iterator>(m_map);
    }

    iterator end() noexcept
    {
        return reinterpret_cast<iterator>(m_map) + (m_width * m_height);
    }

    const_iterator end() const noexcept
    {
        return reinterpret_cast<const_iterator>(m_map) + (m_width * m_height);
    }

    const_iterator cend() const noexcept
    {
        return reinterpret_cast<const_iterator>(m_map) + (m_width * m_height);
    }

    reverse_iterator rbegin() noexcept
    {
        return reverse_iterator{reinterpret_cast<pointer>(m_map) + (m_width * m_height) - 1};
    }

    const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator{reinterpret_cast<const_pointer>(m_map) + (m_width * m_height) - 1};
    }

    const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator{reinterpret_cast<const_pointer>(m_map) + (m_width * m_height) - 1};
    }

    reverse_iterator rend() noexcept
    {
        return reverse_iterator{reinterpret_cast<pointer>(m_map) - 1};
    }

    const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator{reinterpret_cast<const_pointer>(m_map) - 1};
    }

    const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator{reinterpret_cast<const_pointer>(m_map) - 1};
    }

    size_type size() const noexcept
    {
        return m_width * m_height;
    }

    size_type max_size() const noexcept
    {
        return std::numeric_limits<size_type>::max();
    }

    size_type byte_size() const noexcept
    {
        return m_width * m_height * sizeof(value_type);
    }

    reference operator[](size_type index) noexcept
    {
        return reinterpret_cast<pixel*>(m_map)[index];
    }

    const_reference operator[](size_type index) const noexcept
    {
        return reinterpret_cast<const pixel*>(m_map)[index];
    }

    reference operator()(size_type x, size_type y) noexcept
    {
        return reinterpret_cast<pixel*>(m_map)[y * m_width + x];
    }

    const_reference operator()(size_type x, size_type y) const noexcept
    {
        return reinterpret_cast<const pixel*>(m_map)[y * m_width + x];
    }

    std::uint8_t* bytes() noexcept
    {
        return reinterpret_cast<std::uint8_t*>(m_map);
    }

    const std::uint8_t* bytes() const noexcept
    {
        return reinterpret_cast<const std::uint8_t*>(m_map);
    }

    size_type width() const noexcept
    {
        return m_width;
    }

    size_type height() const noexcept
    {
        return m_height;
    }

private:
    size_type m_width{};
    size_type m_height{};
    vulkan::buffer m_buffer{};
    vulkan::memory_heap_chunk m_memory{};
    void* m_map{};
    image_usage m_usage{};
};

TEPHRA_API void set_object_name(device& dev, const image& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const image& img) noexcept
{
    return img.m_buffer.device();
}

template<>
inline VkBuffer underlying_cast(const image& img) noexcept
{
     return img.m_buffer;
}

}

template<> struct tph::enable_enum_operations<tph::image_usage> {static constexpr bool value{true};};

#endif
