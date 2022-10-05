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

#ifndef TEPHRA_COMMANDS_HPP_INCLUDED
#define TEPHRA_COMMANDS_HPP_INCLUDED

#include "config.hpp"

#include <span>
#include <array>
#include <vector>

#include "vulkan/vulkan.hpp"

#include "enumerations.hpp"

namespace tph
{

class device;
class render_pass;
class command_buffer;
class semaphore;
class fence;
class buffer;
class image;
class texture;
class descriptor_set;
class query_pool;
class event;
class render_pass;
class framebuffer;
class pipeline;
class pipeline_layout;

enum class command_pool_options : std::uint32_t
{
    none = 0,
    transient = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
    reset = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    protected_buffer = VK_COMMAND_POOL_CREATE_PROTECTED_BIT
};

enum class command_pool_reset_options : std::uint32_t
{
    none = 0,
    release = VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT
};

class TEPHRA_API command_pool
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr command_pool() = default;
    explicit command_pool(device& dev, command_pool_options options = command_pool_options::none);
    explicit command_pool(device& dev, queue q, command_pool_options options = command_pool_options::none);

    explicit command_pool(vulkan::command_pool cmdpool, queue q, const std::array<std::uint32_t, static_cast<std::size_t>(queue::count)>& queue_families) noexcept
    :m_pool{std::move(cmdpool)}
    ,m_queue_families{queue_families}
    ,m_queue_family{queue_families[static_cast<std::size_t>(q)]}
    {

    }

    ~command_pool() = default;
    command_pool(const command_pool&) = delete;
    command_pool& operator=(const command_pool&) = delete;
    command_pool(command_pool&& other) noexcept = default;
    command_pool& operator=(command_pool&& other) noexcept = default;

    void reset(command_pool_reset_options options = command_pool_reset_options::none);
    void trim() noexcept;

    vulkan::device_context context() const noexcept
    {
        return m_pool.context();
    }

    const std::array<std::uint32_t, static_cast<std::size_t>(queue::count)>& queue_families() const noexcept
    {
        return m_queue_families;
    }

    std::uint32_t queue_family() const noexcept
    {
        return m_queue_family;
    }

private:
    vulkan::command_pool m_pool{};
    std::array<std::uint32_t, static_cast<std::size_t>(queue::count)> m_queue_families{};
    std::uint32_t m_queue_family{};
};

TEPHRA_API void set_object_name(device& dev, const command_pool& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const command_pool& pool) noexcept
{
    return pool.m_pool.device();
}

template<>
inline VkCommandPool underlying_cast(const command_pool& pool) noexcept
{
    return pool.m_pool;
}

class TEPHRA_API command_buffer
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr command_buffer() = default;

    explicit command_buffer(vulkan::command_buffer cmdbuf, std::uint32_t queue_family, const std::array<std::uint32_t, static_cast<std::size_t>(queue::count)>& queue_families) noexcept
    :m_cmdbuf{std::move(cmdbuf)}
    ,m_queue_families{queue_families}
    ,m_queue_family{queue_family}
    {

    }

    ~command_buffer() = default;
    command_buffer(const command_buffer&) = delete;
    command_buffer& operator=(const command_buffer&) = delete;
    command_buffer(command_buffer&& other) noexcept = default;
    command_buffer& operator=(command_buffer&& other) noexcept = default;

    vulkan::device_context context() const noexcept
    {
        return m_cmdbuf.context();
    }

    const std::array<std::uint32_t, static_cast<std::size_t>(queue::count)>& queue_families() const noexcept
    {
        return m_queue_families;
    }

    std::uint32_t queue_family() const noexcept
    {
        return m_queue_family;
    }

private:
    vulkan::command_buffer m_cmdbuf{};
    std::array<std::uint32_t, static_cast<std::size_t>(queue::count)> m_queue_families{};
    std::uint32_t m_queue_family{};
};

TEPHRA_API void set_object_name(device& dev, const command_buffer& object, const std::string& name);

template<>
inline VkDevice underlying_cast(const command_buffer& cmdbuf) noexcept
{
    return cmdbuf.m_cmdbuf.device();
}

template<>
inline VkCommandPool underlying_cast(const command_buffer& cmdbuf) noexcept
{
    return cmdbuf.m_cmdbuf.command_pool();
}

template<>
inline VkCommandBuffer underlying_cast(const command_buffer& cmdbuf) noexcept
{
    return cmdbuf.m_cmdbuf;
}

enum class command_buffer_level : std::uint32_t
{
    primary = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    secondary = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
};

enum class command_buffer_options : std::uint32_t
{
    none = 0x00,
    one_time_submit = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    simultaneous_use = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
};

enum class command_buffer_reset_options : std::uint32_t
{
    none = 0x00,
    release = VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT
};

struct copy_offset
{
    std::int32_t x{};
    std::int32_t y{};
    std::int32_t z{};
};

struct copy_extent
{
    std::uint32_t width{1};
    std::uint32_t height{1};
    std::uint32_t depth{1};
};

struct texture_subresource_layer
{
    std::uint32_t mip_level{};
    std::uint32_t base_array_layer{};
    std::uint32_t array_layer_count{1};
    texture_aspect aspect{texture_aspect::undefined};
};

struct buffer_copy
{
    std::uint64_t src_offset{};
    std::uint64_t dest_offset{};
    std::uint64_t size{};
};

struct texture_copy
{
    texture_subresource_layer src_subresource{};
    copy_offset src_offset{};
    texture_subresource_layer dest_subresource{};
    copy_offset dest_offset{};
    copy_extent size{};
};

struct buffer_image_copy
{
    std::uint64_t buffer_offset{};
};

struct buffer_texture_copy
{
    std::uint64_t buffer_offset{};
    std::uint32_t buffer_image_width{};
    std::uint32_t buffer_image_height{};
    texture_subresource_layer texture_subresource{};
    copy_offset texture_offset{};
    copy_extent texture_size{};
};

struct image_texture_copy
{
    texture_subresource_layer texture_subresource{};
    copy_offset texture_offset{};
    copy_extent texture_size{};
};

struct texture_blit
{
    texture_subresource_layer src_subresource{};
    copy_offset src_offset{};
    copy_extent src_size{};
    texture_subresource_layer dest_subresource{};
    copy_offset dest_offset{};
    copy_extent dest_size{};
};

struct memory_barrier
{
    resource_access src_access{};
    resource_access dest_access{};
};

struct buffer_memory_barrier
{
    std::reference_wrapper<tph::buffer> buffer;
    std::uint64_t offset{};
    std::uint64_t size{VK_WHOLE_SIZE};
    resource_access src_access{};
    resource_access dest_access{};
    std::uint32_t src_queue_family{VK_QUEUE_FAMILY_IGNORED};
    std::uint32_t dest_queue_family{VK_QUEUE_FAMILY_IGNORED};
};

struct texture_memory_barrier
{
    std::reference_wrapper<tph::texture> texture;
    texture_subresource_range subresource{};
    resource_access src_access{};
    resource_access dest_access{};
    texture_layout old_layout{};
    texture_layout new_layout{};
    std::uint32_t src_queue_family{VK_QUEUE_FAMILY_IGNORED};
    std::uint32_t dest_queue_family{VK_QUEUE_FAMILY_IGNORED};
};

struct texture_resolve
{
    copy_offset src_offset{};
    texture_subresource_layer src_subresource{};
    copy_offset dest_offset{};
    texture_subresource_layer dest_subresource{};
    copy_extent size{};
};

struct clear_attachment
{
    uint32_t attachment{};
    clear_value_t clear_value{};
    texture_aspect aspect{texture_aspect::undefined};
};

struct clear_rect
{
    std::uint32_t x{};
    std::uint32_t y{};
    std::uint32_t width{};
    std::uint32_t height{};
    uint32_t base_array_layer{};
    uint32_t array_layer_count{1};
};

struct submit_info
{
    std::vector<std::reference_wrapper<semaphore>> wait_semaphores{};
    std::vector<pipeline_stage> wait_stages{};
    std::vector<std::reference_wrapper<command_buffer>> command_buffers{};
    std::vector<std::reference_wrapper<semaphore>> signal_semaphores{};
};

namespace cmd
{

TEPHRA_API command_buffer begin(command_pool& cmdpool, command_buffer_level level = command_buffer_level::primary, command_buffer_options options = command_buffer_options::none);
TEPHRA_API command_buffer begin(command_pool& cmdpool, render_pass& renpass, optional_ref<framebuffer> framebuf, command_buffer_options options = command_buffer_options::none);
TEPHRA_API void begin(command_buffer& cmdbuf, command_buffer_reset_options reset = command_buffer_reset_options::none, command_buffer_options options = command_buffer_options::none);
TEPHRA_API void begin(command_buffer& cmdbuf, render_pass& renpass, optional_ref<framebuffer> framebuf, command_buffer_reset_options reset = command_buffer_reset_options::none, command_buffer_options options = command_buffer_options::none);

TEPHRA_API void copy(command_buffer& cmdbuf, buffer& src, buffer& dest, const buffer_copy& region) noexcept;
TEPHRA_API void copy(command_buffer& cmdbuf, buffer& src, buffer& dest, std::span<const buffer_copy> regions);
TEPHRA_API void copy(command_buffer& cmdbuf, buffer& src, image& dest, const buffer_image_copy& region) noexcept;
TEPHRA_API void copy(command_buffer& cmdbuf, buffer& src, texture& dest, const buffer_texture_copy& region) noexcept;
TEPHRA_API void copy(command_buffer& cmdbuf, buffer& src, texture& dest, std::span<const buffer_texture_copy> regions);

TEPHRA_API void copy(command_buffer& cmdbuf, image& src, buffer& dest, const buffer_image_copy& region) noexcept;
TEPHRA_API void copy(command_buffer& cmdbuf, image& src, texture& dest, const image_texture_copy& region) noexcept;
TEPHRA_API void copy(command_buffer& cmdbuf, image& src, texture& dest, std::span<const image_texture_copy> regions);

TEPHRA_API void copy(command_buffer& cmdbuf, texture& src, buffer& dest, const buffer_texture_copy& region) noexcept;
TEPHRA_API void copy(command_buffer& cmdbuf, texture& src, buffer& dest, std::span<const buffer_texture_copy> regions);
TEPHRA_API void copy(command_buffer& cmdbuf, texture& src, image& dest, const image_texture_copy& region) noexcept;
TEPHRA_API void copy(command_buffer& cmdbuf, texture& src, image& dest, std::span<const image_texture_copy> regions);
TEPHRA_API void copy(command_buffer& cmdbuf, texture& src, texture& dest, const texture_copy& region) noexcept;
TEPHRA_API void copy(command_buffer& cmdbuf, texture& src, texture& dest, std::span<const texture_copy> regions);

TEPHRA_API void blit(command_buffer& cmdbuf, texture& src, texture& dest, filter filter, const texture_blit& region) noexcept;
TEPHRA_API void blit(command_buffer& cmdbuf, texture& src, texture& dest, filter filter, std::span<const texture_blit> regions);

TEPHRA_API void pipeline_barrier(command_buffer& cmdbuf, pipeline_stage src_stage, pipeline_stage dest_stage, dependency_flags flags) noexcept;
TEPHRA_API void pipeline_barrier(command_buffer& cmdbuf, resource_access src_access, resource_access dest_access, dependency_flags flags, pipeline_stage src_stage, pipeline_stage dest_stage) noexcept;
TEPHRA_API void pipeline_barrier(command_buffer& cmdbuf, pipeline_stage src_stage, pipeline_stage dest_stage, dependency_flags flags, std::span<const memory_barrier> memory_barriers, std::span<const buffer_memory_barrier> buffer_barriers, std::span<const texture_memory_barrier> texture_barriers);

TEPHRA_API void update_buffer(command_buffer& cmdbuf, tph::buffer& buffer, std::uint64_t offset, std::uint64_t size, const void* data) noexcept;
TEPHRA_API void fill_buffer(command_buffer& cmdbuf, tph::buffer& buffer, std::uint64_t offset, std::uint64_t size, std::uint32_t value) noexcept;
TEPHRA_API void push_constants(command_buffer& cmdbuf, pipeline_layout& layout, shader_stage stages, std::uint32_t offset, std::uint32_t size, const void* data) noexcept;

TEPHRA_API void begin_render_pass(command_buffer& cmdbuf, const render_pass& rpass, const framebuffer& framebuf, render_pass_content content = render_pass_content::inlined) noexcept;
TEPHRA_API void begin_render_pass(command_buffer& cmdbuf, const render_pass& rpass, const framebuffer& framebuf, const scissor& area, render_pass_content content = render_pass_content::inlined) noexcept;
TEPHRA_API void next_subpass(command_buffer& cmdbuf, render_pass_content content = render_pass_content::inlined) noexcept;
TEPHRA_API void end_render_pass(command_buffer& cmdbuf) noexcept;

TEPHRA_API void bind_pipeline(command_buffer& cmdbuf, pipeline& pipeline) noexcept;
TEPHRA_API void bind_vertex_buffer(command_buffer& cmdbuf, buffer& buffer, std::uint64_t offset) noexcept;
TEPHRA_API void bind_index_buffer(command_buffer& cmdbuf, buffer& buffer, std::uint64_t offset, index_type type) noexcept;
TEPHRA_API void bind_descriptor_set(command_buffer& cmdbuf, std::uint32_t index, descriptor_set& set, pipeline_layout& layout, pipeline_type bind_point = pipeline_type::graphics) noexcept;
TEPHRA_API void bind_descriptor_set(command_buffer& cmdbuf, std::uint32_t index, std::span<descriptor_set> sets, pipeline_layout& layout, pipeline_type bind_point = pipeline_type::graphics) noexcept;

TEPHRA_API void reset_event(command_buffer& cmdbuf, event& evt, pipeline_stage stage) noexcept;
TEPHRA_API void set_event(command_buffer& cmdbuf, event& evt, pipeline_stage stage) noexcept;
TEPHRA_API void wait_event(command_buffer& cmdbuf, event& evt, pipeline_stage src_stage, pipeline_stage dest_stage) noexcept;
TEPHRA_API void wait_event(command_buffer& cmdbuf, event& evt, resource_access src_access, resource_access dest_access, pipeline_stage src_stage, pipeline_stage dest_stage) noexcept;

TEPHRA_API void resolve_image(command_buffer& cmdbuf, texture& src, texture_layout src_layout, texture& dest, texture_layout dest_layout, std::span<const texture_resolve> resolves);

TEPHRA_API void clear_attachments(command_buffer& cmdbuf, std::span<const clear_attachment> attachments, std::span<const clear_rect> rects);
TEPHRA_API void clear_color_image(command_buffer& cmdbuf, texture& tex, texture_layout layout, const clear_color_value& color, std::span<const texture_subresource_range> subresources);
TEPHRA_API void clear_depth_stencil_image(command_buffer& cmdbuf, texture& tex, texture_layout layout, const clear_depth_stencil_value& value, std::span<const texture_subresource_range> subresources);

TEPHRA_API void set_viewport(command_buffer& cmdbuf, const viewport& vwport, std::uint32_t index = 0) noexcept;
TEPHRA_API void set_scissor(command_buffer& cmdbuf, const scissor& scisr, std::uint32_t index = 0) noexcept;
TEPHRA_API void set_line_width(command_buffer& cmdbuf, float width) noexcept;
TEPHRA_API void set_depth_bias(command_buffer& cmdbuf, float constant_factor, float clamp, float slope_factor) noexcept;
TEPHRA_API void set_blend_constants(command_buffer& cmdbuf, float red, float green, float blue, float alpha) noexcept;
TEPHRA_API void set_depth_bounds(command_buffer& cmdbuf, float min, float max) noexcept;
TEPHRA_API void set_stencil_compare_mask(command_buffer& cmdbuf, stencil_face face, std::uint32_t compare_mask) noexcept;
TEPHRA_API void set_stencil_reference(command_buffer& cmdbuf, stencil_face face, std::uint32_t reference) noexcept;
TEPHRA_API void set_stencil_write_mask(command_buffer& cmdbuf, stencil_face face, std::uint32_t write_mask) noexcept;

TEPHRA_API void draw(command_buffer& cmdbuf, std::uint32_t vertex_count, std::uint32_t instance_count, std::uint32_t first_vertex, std::uint32_t first_instance) noexcept;
TEPHRA_API void draw_indexed(command_buffer& cmdbuf, std::uint32_t index_count, std::uint32_t instance_count, std::uint32_t first_index, std::uint32_t first_vertex, std::uint32_t first_instance) noexcept;
TEPHRA_API void draw_indirect(command_buffer& cmdbuf, buffer& buf, std::uint64_t offset, std::uint32_t draw_count, std::uint32_t stride) noexcept;
TEPHRA_API void draw_indexed_indirect(command_buffer& cmdbuf, buffer& buf, std::uint64_t offset, std::uint32_t draw_count, std::uint32_t stride) noexcept;

TEPHRA_API void dispatch(command_buffer& cmdbuf, std::uint32_t group_count_x, std::uint32_t group_count_y, std::uint32_t group_count_z) noexcept;
TEPHRA_API void dispatch_indirect(command_buffer& cmdbuf, buffer& buffer, std::uint64_t offset) noexcept;

TEPHRA_API void reset_query_pool(command_buffer& cmdbuf, query_pool& pool, std::uint32_t first, std::uint32_t count) noexcept;
TEPHRA_API void write_timestamp(command_buffer& cmdbuf, query_pool& pool, std::uint32_t query, pipeline_stage stage) noexcept;
TEPHRA_API void begin_query(command_buffer& cmdbuf, query_pool& pool, std::uint32_t query, query_control options) noexcept;
TEPHRA_API void end_query(command_buffer& cmdbuf, query_pool& pool, std::uint32_t query) noexcept;
TEPHRA_API void copy_query_pool_results(command_buffer& cmdbuf, query_pool& pool, std::uint32_t first, std::uint32_t count, buffer& dest, std::uint64_t offset, std::uint64_t stride, query_results options) noexcept;

TEPHRA_API void begin_label(command_buffer& cmdbuf, const std::string& name, float red = 0.0f, float green = 0.0f, float blue = 0.0f, float alpha = 0.0f) noexcept;
TEPHRA_API void end_label(command_buffer& cmdbuf) noexcept;
TEPHRA_API void insert_label(command_buffer& cmdbuf, const std::string& name, float red = 0.0f, float green = 0.0f, float blue = 0.0f, float alpha = 0.0f) noexcept;

TEPHRA_API void end(command_buffer& cmdbuf);

TEPHRA_API void execute(command_buffer& cmdbuf, command_buffer& secondary_cmdbuf) noexcept;
TEPHRA_API void execute(command_buffer& cmdbuf, std::span<const command_buffer> secondary_cmdbufs);
TEPHRA_API void execute(command_buffer& cmdbuf, std::span<const std::reference_wrapper<command_buffer>> secondary_cmdbufs);

}

struct mipmap_generation_info
{
    std::reference_wrapper<tph::texture> texture;
    tph::filter filter{tph::filter::linear};
    std::uint32_t base_array_layer{};
    std::uint32_t array_layer_count{1};
    resource_access src_access{};
    resource_access dest_access{};
    texture_layout old_layout{};
    texture_layout new_layout{};
    std::uint32_t src_queue_family{VK_QUEUE_FAMILY_IGNORED};
    std::uint32_t dest_queue_family{VK_QUEUE_FAMILY_IGNORED};
};

namespace cmd
{

TEPHRA_API void generate_mipmaps(command_buffer& cmdbuf, pipeline_stage src_stage, pipeline_stage dest_stage, dependency_flags flags, std::span<const mipmap_generation_info> infos);

}

TEPHRA_API void submit(device& dev, const submit_info& info, optional_ref<fence> fen);
TEPHRA_API void submit(device& dev, std::span<const submit_info> submits, optional_ref<fence> fen);
TEPHRA_API void submit(device& dev, tph::queue queue, const submit_info& info, optional_ref<fence> fen);
TEPHRA_API void submit(device& dev, tph::queue queue, std::span<const submit_info> submits, optional_ref<fence> fen);

}

template<> struct tph::enable_enum_operations<tph::command_pool_options> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::command_pool_reset_options> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::command_buffer_options> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::command_buffer_reset_options> {static constexpr bool value{true};};

#endif
