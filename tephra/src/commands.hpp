#ifndef TEPHRA_COMMANDS_HPP_INCLUDED
#define TEPHRA_COMMANDS_HPP_INCLUDED

#include "config.hpp"

#include "vulkan/vulkan.hpp"

#include "renderer.hpp"
#include "enumerations.hpp"
#include "render_target.hpp"

namespace tph
{

class render_target;
class pipeline;
class pipeline_layout;
class command_buffer;
class semaphore;
class fence;
class buffer;
class image;
class texture;
class descriptor_set;

class command_pool
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr command_pool() = default;
    command_pool(renderer& renderer);
    command_pool(renderer& renderer, queue queue);

    ~command_pool() = default;
    command_pool(const command_pool&) = delete;
    command_pool& operator=(const command_pool&) = delete;
    command_pool(command_pool&& other) noexcept = default;
    command_pool& operator=(command_pool&& other) noexcept = default;

    void reset();

private:
    VkDevice m_device{};
    vulkan::command_pool m_pool{};
};

template<>
inline VkDevice underlying_cast(const command_pool& pool) noexcept
{
    return pool.m_device;
}

template<>
inline VkCommandPool underlying_cast(const command_pool& pool) noexcept
{
    return pool.m_pool;
}

class command_buffer
{
    template<typename VulkanObject, typename... Args>
    friend VulkanObject underlying_cast(const Args&...) noexcept;

public:
    constexpr command_buffer() = default;
    ~command_buffer() = default;
    command_buffer(const command_buffer&) = delete;
    command_buffer& operator=(const command_buffer&) = delete;
    command_buffer(command_buffer&& other) noexcept = default;
    command_buffer& operator=(command_buffer&& other) noexcept = default;

    explicit command_buffer(vulkan::command_buffer buffer) noexcept
    :m_buffer{std::move(buffer)}
    {

    }

private:
    vulkan::command_buffer m_buffer{};
};

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

command_buffer begin(command_pool& pool, command_buffer_level level = command_buffer_level::primary, command_buffer_flags flags = command_buffer_flags::none);
command_buffer begin(command_pool& pool, render_target& target, std::optional<std::size_t> image_index = std::nullopt, command_buffer_flags flags = command_buffer_flags::none);

void copy(command_buffer& command_buffer, buffer& source, buffer& destination, const buffer_copy& region);
void copy(command_buffer& command_buffer, buffer& source, buffer& destination, const std::vector<buffer_copy>& regions);
void copy(command_buffer& command_buffer, buffer& source, image& destination, const buffer_image_copy& region);
void copy(command_buffer& command_buffer, buffer& source, texture& destination, const buffer_texture_copy& region);
void copy(command_buffer& command_buffer, buffer& source, texture& destination, const std::vector<buffer_texture_copy>& regions);
void copy(command_buffer& command_buffer, image& source, buffer& destination, const buffer_image_copy& region);
void copy(command_buffer& command_buffer, image& source, texture& destination, const image_texture_copy& region);
void copy(command_buffer& command_buffer, image& source, texture& destination, const std::vector<image_texture_copy>& regions);
void copy(command_buffer& command_buffer, texture& source, buffer& destination, const buffer_texture_copy& region);
void copy(command_buffer& command_buffer, texture& source, buffer& destination, const std::vector<buffer_texture_copy>& regions);
void copy(command_buffer& command_buffer, texture& source, image& destination, const image_texture_copy& region);
void copy(command_buffer& command_buffer, texture& source, image& destination, const std::vector<image_texture_copy>& regions);
void copy(command_buffer& command_buffer, texture& source, texture& destination, const texture_copy& region);
void copy(command_buffer& command_buffer, texture& source, texture& destination, const std::vector<texture_copy>& regions);

void copy(command_buffer& command_buffer, buffer& source, buffer& destination);
void copy(command_buffer& command_buffer, image& source, image& destination);
void copy(command_buffer& command_buffer, image& source, texture& destination);
void copy(command_buffer& command_buffer, texture& source, image& destination);
void copy(command_buffer& command_buffer, texture& source, texture& destination);

void blit(command_buffer& command_buffer, texture& source, texture& destination, filter filter, const texture_blit& region);
void blit(command_buffer& command_buffer, texture& source, texture& destination, filter filter, const std::vector<texture_blit>& regions);
void blit(command_buffer& command_buffer, texture& source, texture& destination, filter filter);

void pipeline_barrier(command_buffer& command_buffer, pipeline_stage source_stage, pipeline_stage destination_stage);
void prepare(command_buffer& command_buffer, texture& texture, pipeline_stage stage);

void push_constants(command_buffer& command_buffer, pipeline_layout& layout, shader_stage stages, std::uint32_t offset, std::uint32_t size, const void* data);

void begin_render_pass(command_buffer& command_buffer, render_target& target, std::uint32_t image_index, render_pass_content content = render_pass_content::inlined);
void begin_render_pass(command_buffer& command_buffer, const render_pass& render_pass, const framebuffer& framebuffer, render_pass_content content = render_pass_content::inlined);
void begin_render_pass(command_buffer& command_buffer, const render_pass& render_pass, const framebuffer& framebuffer, const scissor& area, render_pass_content content = render_pass_content::inlined);
void next_subpass(command_buffer& command_buffer, render_pass_content content = render_pass_content::inlined);
void end_render_pass(command_buffer& command_buffer);

void bind_pipeline(command_buffer& command_buffer, pipeline& pipeline);
void bind_vertex_buffer(command_buffer& command_buffer, buffer& buffer, std::uint64_t offset);
void bind_index_buffer(command_buffer& command_buffer, buffer& buffer, std::uint64_t offset, index_type type);
void bind_descriptor_set(command_buffer& command_buffer, descriptor_set& descriptor_set, pipeline_layout& layout);

void set_viewport(command_buffer& command_buffer, const viewport& viewport, std::uint32_t index = 0);
void set_scissor(command_buffer& command_buffer, const scissor& scissor, std::uint32_t index = 0);

void draw(command_buffer& command_buffer, std::uint32_t vertex_count, std::uint32_t instance_count, std::uint32_t first_vertex, std::uint32_t first_instance);
void draw_indexed(command_buffer& command_buffer, std::uint32_t index_count, std::uint32_t instance_count, std::uint32_t first_index, std::uint32_t first_vertex, std::uint32_t first_instance);
void draw_indirect(command_buffer& command_buffer, buffer& buffer, std::uint64_t offset, std::uint32_t draw_count, std::uint32_t stride);
void draw_indexed_indirect(command_buffer& command_buffer, buffer& buffer, std::uint64_t offset, std::uint32_t draw_count, std::uint32_t stride);

void dispatch(command_buffer& command_buffer, std::uint32_t group_count_x, std::uint32_t group_count_y, std::uint32_t group_count_z);
void dispatch_indirect(command_buffer& command_buffer, buffer& buffer, std::uint64_t offset);

void end(command_buffer& command_buffer);

void execute(command_buffer& buffer, command_buffer& secondary_buffer);
void execute(command_buffer& buffer, const std::vector<std::reference_wrapper<command_buffer>>& secondary_buffers);

}

void submit(renderer& renderer, const submit_info& info, optional_ref<fence> fence);
void submit(renderer& renderer, const std::vector<submit_info>& submits, optional_ref<fence> fence);
void submit(renderer& renderer, queue queue, const submit_info& info, optional_ref<fence> fence);
void submit(renderer& renderer, queue queue, const std::vector<submit_info>& submits, optional_ref<fence> fence);

}

template<> struct tph::enable_enum_operations<tph::command_buffer_flags> {static constexpr bool value{true};};

#endif
