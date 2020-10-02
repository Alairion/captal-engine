#ifndef TEPHRA_COMMANDS_HPP_INCLUDED
#define TEPHRA_COMMANDS_HPP_INCLUDED

#include "config.hpp"

#include <span>

#include "vulkan/vulkan.hpp"

#include "renderer.hpp"
#include "render_target.hpp"
#include "enumerations.hpp"
#include "pipeline.hpp"

namespace tph
{

class render_pass;
class command_buffer;
class semaphore;
class fence;
class buffer;
class image;
class texture;
class descriptor_set;

class TEPHRA_API command_pool
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr command_pool() = default;
    explicit command_pool(renderer& renderer);
    explicit command_pool(renderer& renderer, queue queue);

    explicit command_pool(vulkan::command_pool pool) noexcept
    :m_pool{std::move(pool)}
    {

    }

    ~command_pool() = default;
    command_pool(const command_pool&) = delete;
    command_pool& operator=(const command_pool&) = delete;
    command_pool(command_pool&& other) noexcept = default;
    command_pool& operator=(command_pool&& other) noexcept = default;

    void reset();

private:
    vulkan::command_pool m_pool{};
};

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

    explicit command_buffer(vulkan::command_buffer buffer) noexcept
    :m_buffer{std::move(buffer)}
    {

    }

    ~command_buffer() = default;
    command_buffer(const command_buffer&) = delete;
    command_buffer& operator=(const command_buffer&) = delete;
    command_buffer(command_buffer&& other) noexcept = default;
    command_buffer& operator=(command_buffer&& other) noexcept = default;

private:
    vulkan::command_buffer m_buffer{};
};

template<>
inline VkDevice underlying_cast(const command_buffer& buffer) noexcept
{
    return buffer.m_buffer.device();
}

template<>
inline VkCommandPool underlying_cast(const command_buffer& buffer) noexcept
{
    return buffer.m_buffer.command_pool();
}

template<>
inline VkCommandBuffer underlying_cast(const command_buffer& buffer) noexcept
{
    return buffer.m_buffer;
}

enum class command_buffer_level : std::uint32_t
{
    primary = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    secondary = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
};

enum class command_buffer_flags : std::uint32_t
{
    none = 0x00,
    one_time_submit = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    simultaneous_use = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
};

struct copy_offset
{
    std::int32_t x{};
    std::int32_t y{};
    std::int32_t z{};
};

struct copy_extent
{
    std::uint32_t width{};
    std::uint32_t height{1};
    std::uint32_t depth{1};
};

struct buffer_copy
{
    std::uint64_t source_offset{};
    std::uint64_t destination_offset{};
    std::uint64_t size{};
};

struct texture_copy
{
    copy_offset source_offset{};
    copy_offset destination_offset{};
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
    copy_offset texture_offset{};
    copy_extent texture_size{};
};

struct image_texture_copy
{
    copy_offset texture_offset{};
    copy_extent texture_size{};
};

struct texture_blit
{
    copy_offset source_offset{};
    copy_extent source_size{};
    copy_offset destination_offset{};
    copy_extent destination_size{};
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

TEPHRA_API command_buffer begin(command_pool& pool, command_buffer_level level = command_buffer_level::primary, command_buffer_flags flags = command_buffer_flags::none);
TEPHRA_API command_buffer begin(command_pool& pool, render_pass& render_pass, optional_ref<framebuffer> framebuffer, command_buffer_flags flags = command_buffer_flags::none);

TEPHRA_API void copy(command_buffer& command_buffer, buffer& source, buffer& destination, const buffer_copy& region);
TEPHRA_API void copy(command_buffer& command_buffer, buffer& source, buffer& destination, std::span<const buffer_copy> regions);
TEPHRA_API void copy(command_buffer& command_buffer, buffer& source, image& destination, const buffer_image_copy& region);
TEPHRA_API void copy(command_buffer& command_buffer, buffer& source, texture& destination, const buffer_texture_copy& region);
TEPHRA_API void copy(command_buffer& command_buffer, buffer& source, texture& destination, std::span<const buffer_texture_copy> regions);
TEPHRA_API void copy(command_buffer& command_buffer, image& source, buffer& destination, const buffer_image_copy& region);
TEPHRA_API void copy(command_buffer& command_buffer, image& source, texture& destination, const image_texture_copy& region);
TEPHRA_API void copy(command_buffer& command_buffer, image& source, texture& destination, std::span<const image_texture_copy> regions);
TEPHRA_API void copy(command_buffer& command_buffer, texture& source, buffer& destination, const buffer_texture_copy& region);
TEPHRA_API void copy(command_buffer& command_buffer, texture& source, buffer& destination, std::span<const buffer_texture_copy> regions);
TEPHRA_API void copy(command_buffer& command_buffer, texture& source, image& destination, const image_texture_copy& region);
TEPHRA_API void copy(command_buffer& command_buffer, texture& source, image& destination, std::span<const image_texture_copy> regions);
TEPHRA_API void copy(command_buffer& command_buffer, texture& source, texture& destination, const texture_copy& region);
TEPHRA_API void copy(command_buffer& command_buffer, texture& source, texture& destination, std::span<const texture_copy> regions);

TEPHRA_API void copy(command_buffer& command_buffer, buffer& source, buffer& destination);
TEPHRA_API void copy(command_buffer& command_buffer, image& source, image& destination);
TEPHRA_API void copy(command_buffer& command_buffer, image& source, texture& destination);
TEPHRA_API void copy(command_buffer& command_buffer, texture& source, image& destination);
TEPHRA_API void copy(command_buffer& command_buffer, texture& source, texture& destination);

TEPHRA_API void blit(command_buffer& command_buffer, texture& source, texture& destination, filter filter, const texture_blit& region);
TEPHRA_API void blit(command_buffer& command_buffer, texture& source, texture& destination, filter filter, std::span<const texture_blit> regions);
TEPHRA_API void blit(command_buffer& command_buffer, texture& source, texture& destination, filter filter);

TEPHRA_API void transition(command_buffer& command_buffer, texture& texture, resource_access source_access, resource_access destination_access, pipeline_stage source_stage, pipeline_stage destination_stage, texture_layout current_layout, texture_layout next_layout);
TEPHRA_API void pipeline_barrier(command_buffer& command_buffer, pipeline_stage source_stage, pipeline_stage destination_stage);
TEPHRA_API void pipeline_barrier(command_buffer& command_buffer, resource_access source_access, resource_access destination_access, pipeline_stage source_stage, pipeline_stage destination_stage);

TEPHRA_API void update_buffer(command_buffer& command_buffer, tph::buffer& buffer, std::uint64_t offset, std::uint64_t size, const void* data);
TEPHRA_API void fill_buffer(command_buffer& command_buffer, tph::buffer& buffer, std::uint64_t offset, std::uint64_t size, std::uint32_t value);
TEPHRA_API void push_constants(command_buffer& command_buffer, pipeline_layout& layout, shader_stage stages, std::uint32_t offset, std::uint32_t size, const void* data);

TEPHRA_API void begin_render_pass(command_buffer& command_buffer, const render_pass& render_pass, const framebuffer& framebuffer, render_pass_content content = render_pass_content::inlined);
TEPHRA_API void begin_render_pass(command_buffer& command_buffer, const render_pass& render_pass, const framebuffer& framebuffer, const scissor& area, render_pass_content content = render_pass_content::inlined);
TEPHRA_API void next_subpass(command_buffer& command_buffer, render_pass_content content = render_pass_content::inlined);
TEPHRA_API void end_render_pass(command_buffer& command_buffer);

TEPHRA_API void bind_pipeline(command_buffer& command_buffer, pipeline& pipeline);
TEPHRA_API void bind_vertex_buffer(command_buffer& command_buffer, buffer& buffer, std::uint64_t offset);
TEPHRA_API void bind_index_buffer(command_buffer& command_buffer, buffer& buffer, std::uint64_t offset, index_type type);
TEPHRA_API void bind_descriptor_set(command_buffer& command_buffer, descriptor_set& descriptor_set, pipeline_layout& layout, pipeline_type bind_point = pipeline_type::graphics);

TEPHRA_API void set_viewport(command_buffer& command_buffer, const viewport& viewport, std::uint32_t index = 0);
TEPHRA_API void set_scissor(command_buffer& command_buffer, const scissor& scissor, std::uint32_t index = 0);

TEPHRA_API void draw(command_buffer& command_buffer, std::uint32_t vertex_count, std::uint32_t instance_count, std::uint32_t first_vertex, std::uint32_t first_instance);
TEPHRA_API void draw_indexed(command_buffer& command_buffer, std::uint32_t index_count, std::uint32_t instance_count, std::uint32_t first_index, std::uint32_t first_vertex, std::uint32_t first_instance);
TEPHRA_API void draw_indirect(command_buffer& command_buffer, buffer& buffer, std::uint64_t offset, std::uint32_t draw_count, std::uint32_t stride);
TEPHRA_API void draw_indexed_indirect(command_buffer& command_buffer, buffer& buffer, std::uint64_t offset, std::uint32_t draw_count, std::uint32_t stride);

TEPHRA_API void dispatch(command_buffer& command_buffer, std::uint32_t group_count_x, std::uint32_t group_count_y, std::uint32_t group_count_z);
TEPHRA_API void dispatch_indirect(command_buffer& command_buffer, buffer& buffer, std::uint64_t offset);

TEPHRA_API void end(command_buffer& command_buffer);

TEPHRA_API void execute(command_buffer& buffer, command_buffer& secondary_buffer);
TEPHRA_API void execute(command_buffer& buffer, std::span<const command_buffer> secondary_buffers);
TEPHRA_API void execute(command_buffer& buffer, std::span<const std::reference_wrapper<command_buffer>> secondary_buffers);

}

TEPHRA_API void submit(renderer& renderer, const submit_info& info, optional_ref<fence> fence);
TEPHRA_API void submit(renderer& renderer, std::span<const submit_info> submits, optional_ref<fence> fence);
TEPHRA_API void submit(renderer& renderer, queue queue, const submit_info& info, optional_ref<fence> fence);
TEPHRA_API void submit(renderer& renderer, queue queue, std::span<const submit_info> submits, optional_ref<fence> fence);

}

template<> struct tph::enable_enum_operations<tph::command_buffer_flags> {static constexpr bool value{true};};

#endif
