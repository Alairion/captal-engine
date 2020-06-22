#include "image.hpp"

#include <cassert>
#include <memory>
#include <fstream>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <captal_foundation/encoding.hpp>
#include <captal_foundation/utility.hpp>

#include "renderer.hpp"

namespace tph
{

using stbi_ptr = std::unique_ptr<stbi_uc, void(*)(void*)>;

static constexpr image_usage not_extension{~(image_usage::host_access | image_usage::persistant_mapping)};

static VkMemoryPropertyFlags optimal_memory_types(image_usage usage)
{
    VkMemoryPropertyFlags optimal{VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

    if(static_cast<bool>(usage & image_usage::host_access))
    {
        return optimal | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    }

    if(static_cast<bool>(usage & image_usage::transfer_source) && !static_cast<bool>(usage & image_usage::transfer_destination))
    {
        return optimal | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }

    return optimal;
}

image::image(renderer& renderer, const std::filesystem::path& file, image_usage usage)
:image{renderer, read_file<std::vector<std::uint8_t>>(file), usage}
{

}

image::image(renderer& renderer, std::span<const std::uint8_t> data, image_usage usage)
:m_usage{usage}
{
    int width{};
    int height{};
    int channels{};

    stbi_ptr pixels{stbi_load_from_memory(std::data(data), static_cast<int>(std::size(data)), &width, &height, &channels, STBI_rgb_alpha), &stbi_image_free};
    if(!pixels)
        throw std::runtime_error{"Can not load image. " + std::string{stbi_failure_reason()}};

    m_buffer = vulkan::buffer{underlying_cast<VkDevice>(renderer), static_cast<std::uint64_t>(width * height * 4), static_cast<VkBufferUsageFlags>(usage & not_extension)};
    m_memory = renderer.allocator().allocate_bound(m_buffer, vulkan::memory_resource_type::linear, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, optimal_memory_types(usage));

    m_map = m_memory.map();
    std::memcpy(m_map, pixels.get(), static_cast<std::size_t>(width * height * 4));

    if(!static_cast<bool>(usage & image_usage::persistant_mapping))
    {
        m_memory.unmap();
    }

    m_width = static_cast<size_type>(width);
    m_height = static_cast<size_type>(height);
}

image::image(renderer& renderer, std::istream& stream, image_usage usage)
:m_usage{usage}
{
    assert(stream && "Invalid stream.");

    const std::string data{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};

    int width{};
    int height{};
    int channels{};
    std::unique_ptr<stbi_uc, void(*)(void*)> pixels{stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(std::data(data)), static_cast<int>(std::size(data)), &width, &height, &channels, STBI_rgb_alpha), &stbi_image_free};
    if(!pixels)
        throw std::runtime_error{"Can not load image. " + std::string{stbi_failure_reason()}};

    m_buffer = vulkan::buffer{underlying_cast<VkDevice>(renderer), static_cast<std::uint64_t>(width * height * 4), static_cast<VkBufferUsageFlags>(usage & not_extension)};
    m_memory = renderer.allocator().allocate_bound(m_buffer, vulkan::memory_resource_type::linear, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, optimal_memory_types(usage));

    m_map = m_memory.map();
    std::memcpy(m_map, pixels.get(), static_cast<std::size_t>(width * height * 4));

    if(!static_cast<bool>(usage & image_usage::persistant_mapping))
    {
        m_memory.unmap();
    }

    m_width = static_cast<size_type>(width);
    m_height = static_cast<size_type>(height);
}

image::image(renderer& renderer, size_type width, size_type height, const std::uint8_t* data, image_usage usage)
:m_width{width}
,m_height{height}
,m_usage{usage}
{
    assert(width > 0 && "Image width must be greater than 0");
    assert(height > 0 && "Image width must be greater than 0");

    m_buffer = vulkan::buffer{underlying_cast<VkDevice>(renderer), static_cast<std::uint64_t>(width * height * 4), static_cast<VkBufferUsageFlags>(usage & not_extension)};
    m_memory = renderer.allocator().allocate_bound(m_buffer, vulkan::memory_resource_type::linear, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, optimal_memory_types(usage));

    m_map = m_memory.map();
    std::memcpy(m_map, data, static_cast<std::size_t>(width * height * 4));

    if(!static_cast<bool>(usage & image_usage::persistant_mapping))
    {
        m_memory.unmap();
    }
}

image::image(renderer& renderer, size_type width, size_type height, image_usage usage)
:m_width{width}
,m_height{height}
,m_usage{usage}
{
    assert(width > 0 && "Image width must be greater than 0");
    assert(height > 0 && "Image width must be greater than 0");

    m_buffer = vulkan::buffer{underlying_cast<VkDevice>(renderer), static_cast<std::uint64_t>(width * height * 4), static_cast<VkBufferUsageFlags>(usage & not_extension)};
    m_memory = renderer.allocator().allocate_bound(m_buffer, vulkan::memory_resource_type::linear, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, optimal_memory_types(usage));

    if(static_cast<bool>(usage & image_usage::persistant_mapping))
    {
        m_map = m_memory.map();
    }
}

static void write_func(void* context, void* data, int size)
{
    auto& buffer{*reinterpret_cast<std::vector<std::uint8_t>*>(context)};

    buffer.insert(std::end(buffer), reinterpret_cast<const std::uint8_t*>(data), reinterpret_cast<const std::uint8_t*>(data) + size);
}

struct unmapper
{
    ~unmapper()
    {
        if(unmap)
            memory.unmap();
    }

    const vulkan::memory_heap_chunk& memory;
    bool unmap;
};

std::vector<std::uint8_t> image::write(image_format format, std::int32_t quality) const
{
    const bool was_mapped{static_cast<bool>(m_map)};
    const unmapper unmapper{m_memory, !was_mapped};

    const void* input{};
    std::vector<pixel> local_buffer{};

    if(static_cast<bool>(m_usage & image_usage::host_access))
    {
        if(!was_mapped)
        {
            input = m_memory.map();
        }
        else
        {
            input = m_map;
        }
    }
    else
    {
        local_buffer.resize(m_width * m_height);

        if(!was_mapped)
        {
            std::memcpy(std::data(local_buffer), m_memory.map(), m_width * m_height * 4);
        }
        else
        {
            std::memcpy(std::data(local_buffer), m_map, m_width * m_height * 4);
        }

        input = std::data(local_buffer);
    }

    std::vector<std::uint8_t> output{};

    if(format == image_format::png)
    {
        if(!stbi_write_png_to_func(&write_func, &output, static_cast<int>(m_width), static_cast<int>(m_height), 4, input, static_cast<int>(m_width * 4)))
            throw std::runtime_error{"Can not format image file."};
    }
    else if(format == image_format::bmp)
    {
        if(!stbi_write_bmp_to_func(&write_func, &output, static_cast<int>(m_width), static_cast<int>(m_height), 4, input))
            throw std::runtime_error{"Can not format image file."};
    }
    else if(format == image_format::tga)
    {
        if(!stbi_write_tga_to_func(&write_func, &output, static_cast<int>(m_width), static_cast<int>(m_height), 4, input))
            throw std::runtime_error{"Can not format image file."};
    }
    else if(format == image_format::jpg)
    {
        if(!stbi_write_jpg_to_func(&write_func, &output, static_cast<int>(m_width), static_cast<int>(m_height), 4, input, quality))
            throw std::runtime_error{"Can not format image file."};
    }

    return output;
}

void image::write(const std::filesystem::path& file, image_format format, std::int32_t quality) const
{
    const std::vector<std::uint8_t> data{write(format, quality)};

    std::ofstream ofs{file, std::ios_base::binary};
    if(!ofs)
        throw std::runtime_error{"Can not open file \"" + convert_to<narrow>(file.string()) + "\"."};

    ofs.write(reinterpret_cast<const char*>(std::data(data)), std::size(data));
}

void image::map()
{
    if(!static_cast<bool>(m_usage & image_usage::persistant_mapping))
        m_map = m_memory.map();
}

void image::unmap() noexcept
{
    if(!static_cast<bool>(m_usage & image_usage::persistant_mapping))
    {
        m_memory.unmap();
        m_map = nullptr;
    }
}

buffer image::to_buffer() noexcept
{
    if(m_map)
    {
        m_memory.unmap();
        m_map = nullptr;
    }

    buffer output{};
    output.m_buffer = std::move(m_buffer);
    output.m_memory = std::move(m_memory);
    output.m_size = m_width * m_height * 4;

    return output;
}

}
