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

#include "image.hpp"

#include <cassert>
#include <memory>
#include <fstream>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <captal_foundation/encoding.hpp>
#include <captal_foundation/utility.hpp>

#include "vulkan/vulkan_functions.hpp"

#include "device.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

struct stbi_deleter
{
    void operator()(void* ptr) const noexcept
    {
        stbi_image_free(ptr);
    }
};

using stbi_ptr = std::unique_ptr<stbi_uc, stbi_deleter>;

static constexpr image_usage not_extension{~image_usage::persistant_mapping};

static VkMemoryPropertyFlags optimal_memory_types(image_usage usage)
{
    if(usage == image_usage::transfer_source)
    {
        return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }

    return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
}

image::image(device& device, const std::filesystem::path& file, image_usage usage)
:image{device, read_file<std::vector<std::uint8_t>>(file), usage}
{

}

image::image(device& device, std::span<const std::uint8_t> data, image_usage usage)
:m_usage{usage}
{
    int width{};
    int height{};
    int channels{};

    stbi_ptr pixels{stbi_load_from_memory(std::data(data), static_cast<int>(std::size(data)), &width, &height, &channels, STBI_rgb_alpha)};
    if(!pixels)
        throw std::runtime_error{"Can not load image. " + std::string{stbi_failure_reason()}};

    m_buffer = vulkan::buffer{device.context(), static_cast<std::uint64_t>(width * height * 4), static_cast<VkBufferUsageFlags>(usage & not_extension)};
    m_memory = device.allocator().allocate_bound(m_buffer, vulkan::memory_resource_type::linear, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, optimal_memory_types(usage));

    m_map = m_memory.map();
    std::memcpy(m_map, pixels.get(), static_cast<std::size_t>(width * height * 4));

    if(!static_cast<bool>(usage & image_usage::persistant_mapping))
    {
        m_memory.unmap();
    }

    m_width = static_cast<size_type>(width);
    m_height = static_cast<size_type>(height);
}

image::image(device& device, std::istream& stream, image_usage usage)
:m_usage{usage}
{
    assert(stream && "Invalid stream.");

    const std::string data{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};

    int width{};
    int height{};
    int channels{};
    stbi_ptr pixels{stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(std::data(data)), static_cast<int>(std::size(data)), &width, &height, &channels, STBI_rgb_alpha)};
    if(!pixels)
        throw std::runtime_error{"Can not load image. " + std::string{stbi_failure_reason()}};

    m_buffer = vulkan::buffer{device.context(), static_cast<std::uint64_t>(width * height * 4), static_cast<VkBufferUsageFlags>(usage & not_extension)};
    m_memory = device.allocator().allocate_bound(m_buffer, vulkan::memory_resource_type::linear, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, optimal_memory_types(usage));

    m_map = m_memory.map();
    std::memcpy(m_map, pixels.get(), static_cast<std::size_t>(width * height * 4));

    if(!static_cast<bool>(usage & image_usage::persistant_mapping))
    {
        m_memory.unmap();
    }

    m_width = static_cast<size_type>(width);
    m_height = static_cast<size_type>(height);
}

image::image(device& device, size_type width, size_type height, const std::uint8_t* data, image_usage usage)
:m_width{width}
,m_height{height}
,m_usage{usage}
{
    assert(width > 0 && "Image width must be greater than 0");
    assert(height > 0 && "Image width must be greater than 0");

    m_buffer = vulkan::buffer{device.context(), static_cast<std::uint64_t>(width * height * 4), static_cast<VkBufferUsageFlags>(usage & not_extension)};
    m_memory = device.allocator().allocate_bound(m_buffer, vulkan::memory_resource_type::linear, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, optimal_memory_types(usage));

    m_map = m_memory.map();
    std::memcpy(m_map, data, static_cast<std::size_t>(width * height * 4));

    if(!static_cast<bool>(usage & image_usage::persistant_mapping))
    {
        m_memory.unmap();
    }
}

image::image(device& device, size_type width, size_type height, image_usage usage)
:m_width{width}
,m_height{height}
,m_usage{usage}
{
    assert(width > 0 && "Image width must be greater than 0");
    assert(height > 0 && "Image width must be greater than 0");

    m_buffer = vulkan::buffer{device.context(), static_cast<std::uint64_t>(width * height * 4), static_cast<VkBufferUsageFlags>(usage & not_extension)};
    m_memory = device.allocator().allocate_bound(m_buffer, vulkan::memory_resource_type::linear, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, optimal_memory_types(usage));

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

    if(m_usage != image_usage::transfer_source)
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
    {
        m_map = m_memory.map();
    }
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

    return buffer{std::move(m_buffer), std::move(m_memory), m_width * m_height * 4};
}

void set_object_name(device& device, const image& object, const std::string& name)
{
    VkDebugUtilsObjectNameInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = VK_OBJECT_TYPE_BUFFER;
    info.objectHandle = reinterpret_cast<std::uint64_t>(underlying_cast<VkBuffer>(object));
    info.pObjectName = std::data(name);

    vulkan::check(device->vkSetDebugUtilsObjectNameEXT(underlying_cast<VkDevice>(device), &info));
}

}
