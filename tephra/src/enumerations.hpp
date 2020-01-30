#ifndef TEPHRA_ENUMERATION_HPP_INCLUDED
#define TEPHRA_ENUMERATION_HPP_INCLUDED

#include "config.hpp"

#include "vulkan/vulkan.hpp"

namespace tph
{

enum class present_mode : std::uint32_t
{
    immediate = VK_PRESENT_MODE_IMMEDIATE_KHR,
    mailbox = VK_PRESENT_MODE_MAILBOX_KHR,
    fifo = VK_PRESENT_MODE_FIFO_KHR,
    fifo_relaxed = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
};

enum class sample_count : std::uint32_t
{
    msaa_x1 = VK_SAMPLE_COUNT_1_BIT,
    msaa_x2 = VK_SAMPLE_COUNT_2_BIT,
    msaa_x4 = VK_SAMPLE_COUNT_4_BIT,
    msaa_x8 = VK_SAMPLE_COUNT_8_BIT,
    msaa_x16 = VK_SAMPLE_COUNT_16_BIT,
    msaa_x32 = VK_SAMPLE_COUNT_32_BIT,
    msaa_x64 = VK_SAMPLE_COUNT_64_BIT,
};

enum class primitive_topology : std::uint32_t
{
    point = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
    line = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
    line_strip = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
    triangle = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    triangle_strip = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
    triangle_fan = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
    line_with_adjacency = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
    line_strip_with_adjacency = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
    triangle_with_adjacency = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
    triangle_strip_with_adjacency = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
    topology_patch_list = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
};

enum class polygon_mode : std::uint32_t
{
    fill = VK_POLYGON_MODE_FILL,
    line = VK_POLYGON_MODE_LINE,
    point = VK_POLYGON_MODE_POINT,
};

enum class cull_mode : std::uint32_t
{
    none = VK_CULL_MODE_NONE,
    front = VK_CULL_MODE_FRONT_BIT,
    back = VK_CULL_MODE_BACK_BIT,
    front_and_back = VK_CULL_MODE_FRONT_AND_BACK,
};

enum class front_face : std::uint32_t
{
    counter_clockwise = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    clockwise = VK_FRONT_FACE_CLOCKWISE,
};

enum class compare_op : std::uint32_t
{
    never = VK_COMPARE_OP_NEVER,
    less = VK_COMPARE_OP_LESS,
    equal = VK_COMPARE_OP_EQUAL,
    less_or_equal = VK_COMPARE_OP_LESS_OR_EQUAL,
    greater = VK_COMPARE_OP_GREATER,
    not_equal = VK_COMPARE_OP_NOT_EQUAL,
    greater_or_equal = VK_COMPARE_OP_GREATER_OR_EQUAL,
    always = VK_COMPARE_OP_ALWAYS,
};

enum class stencil_op : std::uint32_t
{
    keep = VK_STENCIL_OP_KEEP,
    zero = VK_STENCIL_OP_ZERO,
    replace = VK_STENCIL_OP_REPLACE,
    increment_and_clamp = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    decrement_and_clamp = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    invert = VK_STENCIL_OP_INVERT,
    increment_and_wrap = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    decrement_and_wrap = VK_STENCIL_OP_DECREMENT_AND_WRAP,
};

enum class logic_op : std::uint32_t
{
    clear = VK_LOGIC_OP_CLEAR,
    bit_and = VK_LOGIC_OP_AND,
    bit_and_reverse = VK_LOGIC_OP_AND_REVERSE,
    copy = VK_LOGIC_OP_COPY,
    bit_and_inverted = VK_LOGIC_OP_AND_INVERTED,
    no_op = VK_LOGIC_OP_NO_OP,
    bit_xor = VK_LOGIC_OP_XOR,
    bit_or = VK_LOGIC_OP_OR,
    bit_nor = VK_LOGIC_OP_NOR,
    equivalent = VK_LOGIC_OP_EQUIVALENT,
    invert = VK_LOGIC_OP_INVERT,
    reverse = VK_LOGIC_OP_OR_REVERSE,
    copy_inverted = VK_LOGIC_OP_COPY_INVERTED,
    inverted = VK_LOGIC_OP_OR_INVERTED,
    bit_nand = VK_LOGIC_OP_NAND,
    set = VK_LOGIC_OP_SET,
};

enum class blend_factor : std::uint32_t
{
    zero = VK_BLEND_FACTOR_ZERO,
    one = VK_BLEND_FACTOR_ONE,
    source_color = VK_BLEND_FACTOR_SRC_COLOR,
    one_minus_source_color = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
    destination_color = VK_BLEND_FACTOR_DST_COLOR,
    one_minus_destination_color = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
    source_alpha = VK_BLEND_FACTOR_SRC_ALPHA,
    one_minus_source_alpha = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    destination_alpha = VK_BLEND_FACTOR_DST_ALPHA,
    one_minus_destination_alpha = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
    constant_color = VK_BLEND_FACTOR_CONSTANT_COLOR,
    one_minus_constant_color = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
    constant_alpha = VK_BLEND_FACTOR_CONSTANT_ALPHA,
    one_minus_constant_alpha = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
    source_alpha_saturate = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
};

enum class blend_op : std::uint32_t
{
    add = VK_BLEND_OP_ADD,
    substrat = VK_BLEND_OP_SUBTRACT,
    reverse_substract = VK_BLEND_OP_REVERSE_SUBTRACT,
    min = VK_BLEND_OP_MIN,
    max = VK_BLEND_OP_MAX,
};

enum class color_component : std::uint32_t
{
    r = VK_COLOR_COMPONENT_R_BIT,
    g = VK_COLOR_COMPONENT_G_BIT,
    b = VK_COLOR_COMPONENT_B_BIT,
    a = VK_COLOR_COMPONENT_A_BIT,
};

template<> struct enable_enum_operations<color_component> {static constexpr bool value{true};};

enum class dynamic_state : std::uint32_t
{
    viewport = VK_DYNAMIC_STATE_VIEWPORT,
    scissor = VK_DYNAMIC_STATE_SCISSOR,
    line_width = VK_DYNAMIC_STATE_LINE_WIDTH,
    depth_bias = VK_DYNAMIC_STATE_DEPTH_BIAS,
    blend_constants = VK_DYNAMIC_STATE_BLEND_CONSTANTS,
    depth_bounds = VK_DYNAMIC_STATE_DEPTH_BOUNDS,
    compare_mask = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
    stencil_write_mask = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
    stencil_reference = VK_DYNAMIC_STATE_STENCIL_REFERENCE,
};

enum class shader_stage : std::uint32_t
{
    vertex = VK_SHADER_STAGE_VERTEX_BIT,
    tessellation_control = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    tessellation_evaluation = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
    geometry = VK_SHADER_STAGE_GEOMETRY_BIT,
    fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
    compute = VK_SHADER_STAGE_COMPUTE_BIT,
};

template<> struct enable_enum_operations<shader_stage> {static constexpr bool value{true};};

enum class pipeline_stage : std::uint32_t
{
    top_of_pipe = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    draw_indirect = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
    vertex_input = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
    vertex_shader = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
    tessellation_control_shader = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
    tessellation_evaluation_shader = VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,
    geometry_shader = VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
    fragment_shader = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
    early_fragment_tests = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
    late_fragment_tests = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    color_attachment_output = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    compute_shader = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    transfer = VK_PIPELINE_STAGE_TRANSFER_BIT,
    bottom_of_pipe = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    host = VK_PIPELINE_STAGE_HOST_BIT,
    all_graphics = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
    all_commands = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
};

template<> struct enable_enum_operations<pipeline_stage> {static constexpr bool value{true};};

enum class resource_access
{
    none = 0,
    indirect_command_read = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
    index_read = VK_ACCESS_INDEX_READ_BIT,
    vertex_attribute = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
    uniform_read = VK_ACCESS_UNIFORM_READ_BIT,
    input_attachment = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
    shader_read = VK_ACCESS_SHADER_READ_BIT,
    shared_write = VK_ACCESS_SHADER_WRITE_BIT,
    color_attachment_read = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
    color_attachment_write = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    depth_stencil_attachment_read = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
    depth_stencil_attachment_write = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    transfer_read = VK_ACCESS_TRANSFER_READ_BIT,
    transfer_write = VK_ACCESS_TRANSFER_WRITE_BIT,
    host_read = VK_ACCESS_HOST_READ_BIT,
    host_write = VK_ACCESS_HOST_WRITE_BIT,
    memory_read = VK_ACCESS_MEMORY_READ_BIT,
    memory_write = VK_ACCESS_MEMORY_WRITE_BIT,
};

template<> struct enable_enum_operations<resource_access> {static constexpr bool value{true};};

enum class image_layout : std::uint32_t
{
    undefined = VK_IMAGE_LAYOUT_UNDEFINED,
    general = VK_IMAGE_LAYOUT_GENERAL,
    color_attchment_optimal = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    depth_stencil_attachment_optimal = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    depth_stencil_read_only_optimal = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
    shader_read_only_optimal = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    transfer_source_optimal = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    transfer_destination_optimal = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    preinitialized = VK_IMAGE_LAYOUT_PREINITIALIZED,
};

enum class descriptor_type : std::uint32_t
{
    sampler = VK_DESCRIPTOR_TYPE_SAMPLER,
    image_sampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    sampled_image = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    storage_image = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    uniform_texel_buffer = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
    storage_texel_buffer = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
    uniform_buffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    storage_buffer = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    uniform_buffer_dynamic = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
    storage_buffer_dynamic = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
    input_attachment = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
};

enum class vertex_input_rate : std::uint32_t
{
    vertex = VK_VERTEX_INPUT_RATE_VERTEX,
    instance = VK_VERTEX_INPUT_RATE_INSTANCE,
};

enum class vertex_format : std::uint32_t
{
    uint32 = VK_FORMAT_R32_UINT,
    int32 = VK_FORMAT_R32_SINT,
    float32 = VK_FORMAT_R32_SFLOAT,
    float64 = VK_FORMAT_R64_SFLOAT,
    vec2i = VK_FORMAT_R32G32_UINT,
    vec2u = VK_FORMAT_R32G32_SINT,
    vec2f = VK_FORMAT_R32G32_SFLOAT,
    vec2d = VK_FORMAT_R64G64_SFLOAT,
    vec3u = VK_FORMAT_R32G32B32_UINT,
    vec3i = VK_FORMAT_R32G32B32_SINT,
    vec3f = VK_FORMAT_R32G32B32_SFLOAT,
    vec3d = VK_FORMAT_R64G64B64_SFLOAT,
    vec4u = VK_FORMAT_R32G32B32A32_UINT,
    vec4i = VK_FORMAT_R32G32B32A32_SINT,
    vec4f = VK_FORMAT_R32G32B32A32_SFLOAT,
    vec4d = VK_FORMAT_R64G64B64A64_SFLOAT,
};

enum class filter : std::uint32_t
{
    nearest = VK_FILTER_NEAREST,
    linear = VK_FILTER_LINEAR,
};

enum class index_type : std::uint32_t
{
    uint16 = VK_INDEX_TYPE_UINT16,
    uint32 = VK_INDEX_TYPE_UINT32,
};

}

#endif
