#ifndef TEPHRA_ENUMERATION_HPP_INCLUDED
#define TEPHRA_ENUMERATION_HPP_INCLUDED

#include "config.hpp"

#include "vulkan/vulkan.hpp"

namespace tph
{

enum class object_type : std::uint32_t
{
    unknown = VK_OBJECT_TYPE_UNKNOWN,
    instance = VK_OBJECT_TYPE_INSTANCE,
    physical_device = VK_OBJECT_TYPE_PHYSICAL_DEVICE,
    device = VK_OBJECT_TYPE_DEVICE,
    queue = VK_OBJECT_TYPE_QUEUE,
    semaphore = VK_OBJECT_TYPE_SEMAPHORE,
    command_buffer = VK_OBJECT_TYPE_COMMAND_BUFFER,
    fence = VK_OBJECT_TYPE_FENCE,
    device_memory = VK_OBJECT_TYPE_DEVICE_MEMORY,
    buffer = VK_OBJECT_TYPE_BUFFER,
    image = VK_OBJECT_TYPE_IMAGE,
    event = VK_OBJECT_TYPE_EVENT,
    query_pool = VK_OBJECT_TYPE_QUERY_POOL,
    buffer_view = VK_OBJECT_TYPE_BUFFER_VIEW,
    image_view = VK_OBJECT_TYPE_IMAGE_VIEW,
    shader_module = VK_OBJECT_TYPE_SHADER_MODULE,
    pipeline_cache = VK_OBJECT_TYPE_PIPELINE_CACHE,
    pipeline_layout = VK_OBJECT_TYPE_PIPELINE_LAYOUT,
    render_pass = VK_OBJECT_TYPE_RENDER_PASS,
    pipeline = VK_OBJECT_TYPE_PIPELINE,
    descriptor_set_layout = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
    sampler = VK_OBJECT_TYPE_SAMPLER,
    descriptor_pool = VK_OBJECT_TYPE_DESCRIPTOR_POOL,
    descriptor_set = VK_OBJECT_TYPE_DESCRIPTOR_SET,
    framebuffer = VK_OBJECT_TYPE_FRAMEBUFFER,
    command_pool = VK_OBJECT_TYPE_COMMAND_POOL,
    sampler_ycbcr_conversion = VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION,
    surface = VK_OBJECT_TYPE_SURFACE_KHR,
    swapchain = VK_OBJECT_TYPE_SWAPCHAIN_KHR,
    debug_report_callback = VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT,
    debug_messenger = VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT,
};

enum class queue : std::size_t
{
    graphics = 0,
    present = 1,
    transfer = 2,
    compute = 3,
    count = 4
};

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

enum class dynamic_state : std::uint32_t
{
    viewport = VK_DYNAMIC_STATE_VIEWPORT,
    scissor = VK_DYNAMIC_STATE_SCISSOR,
    line_width = VK_DYNAMIC_STATE_LINE_WIDTH,
    depth_bias = VK_DYNAMIC_STATE_DEPTH_BIAS,
    blend_constants = VK_DYNAMIC_STATE_BLEND_CONSTANTS,
    depth_bounds = VK_DYNAMIC_STATE_DEPTH_BOUNDS,
    stencil_compare_mask = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
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

enum class pipeline_stage : std::uint32_t
{
    none = 0,
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

enum class pipeline_type : std::uint32_t
{
    graphics = VK_PIPELINE_BIND_POINT_GRAPHICS,
    compute = VK_PIPELINE_BIND_POINT_COMPUTE,
};

enum class resource_access
{
    none = 0,
    indirect_command_read = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
    index_read = VK_ACCESS_INDEX_READ_BIT,
    vertex_attribute = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
    uniform_read = VK_ACCESS_UNIFORM_READ_BIT,
    input_attachment = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
    shader_read = VK_ACCESS_SHADER_READ_BIT,
    shader_write = VK_ACCESS_SHADER_WRITE_BIT,
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

enum class texture_layout : std::uint32_t
{
    undefined = VK_IMAGE_LAYOUT_UNDEFINED,
    general = VK_IMAGE_LAYOUT_GENERAL,
    color_attachment_optimal = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    depth_stencil_attachment_optimal = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    depth_stencil_read_only_optimal = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
    shader_read_only_optimal = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    transfer_source_optimal = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    transfer_destination_optimal = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    preinitialized = VK_IMAGE_LAYOUT_PREINITIALIZED,
    present_source = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
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

enum class texture_format : std::uint32_t
{
    undefined = VK_FORMAT_UNDEFINED,
    r4g4_unorm_pack = VK_FORMAT_R4G4_UNORM_PACK8,
    r4g4b4a4_unorm_pack = VK_FORMAT_R4G4B4A4_UNORM_PACK16,
    b4g4r4a4_unorm_pack = VK_FORMAT_B4G4R4A4_UNORM_PACK16,
    r5g6b5_unorm_pack = VK_FORMAT_R5G6B5_UNORM_PACK16,
    b5g6r5_unorm_pack = VK_FORMAT_B5G6R5_UNORM_PACK16,
    r5g5b5a1_unorm_pack = VK_FORMAT_R5G5B5A1_UNORM_PACK16,
    b5g5r5a1_unorm_pack = VK_FORMAT_B5G5R5A1_UNORM_PACK16,
    a1r5g5b5_unorm_pack = VK_FORMAT_A1R5G5B5_UNORM_PACK16,
    r8_unorm = VK_FORMAT_R8_UNORM,
    r8_snorm = VK_FORMAT_R8_SNORM,
    r8_uscaled = VK_FORMAT_R8_USCALED,
    r8_sscaled = VK_FORMAT_R8_SSCALED,
    r8_uint = VK_FORMAT_R8_UINT,
    r8_sint = VK_FORMAT_R8_SINT,
    r8_srgb = VK_FORMAT_R8_SRGB,
    r8g8_unorm = VK_FORMAT_R8G8_UNORM,
    r8g8_snorm = VK_FORMAT_R8G8_SNORM,
    r8g8_uscaled = VK_FORMAT_R8G8_USCALED,
    r8g8_sscaled = VK_FORMAT_R8G8_SSCALED,
    r8g8_uint = VK_FORMAT_R8G8_UINT,
    r8g8_sint = VK_FORMAT_R8G8_SINT,
    r8g8_srgb = VK_FORMAT_R8G8_SRGB,
    r8g8b8_unorm = VK_FORMAT_R8G8B8_UNORM,
    r8g8b8_snorm = VK_FORMAT_R8G8B8_SNORM,
    r8g8b8_uscaled = VK_FORMAT_R8G8B8_USCALED,
    r8g8b8_sscaled = VK_FORMAT_R8G8B8_SSCALED,
    r8g8b8_uint = VK_FORMAT_R8G8B8_UINT,
    r8g8b8_sint = VK_FORMAT_R8G8B8_SINT,
    r8g8b8_srgb = VK_FORMAT_R8G8B8_SRGB,
    b8g8r8_unorm = VK_FORMAT_B8G8R8_UNORM,
    b8g8r8_snorm = VK_FORMAT_B8G8R8_SNORM,
    b8g8r8_uscaled = VK_FORMAT_B8G8R8_USCALED,
    b8g8r8_sscaled = VK_FORMAT_B8G8R8_SSCALED,
    b8g8r8_uint = VK_FORMAT_B8G8R8_UINT,
    b8g8r8_sint = VK_FORMAT_B8G8R8_SINT,
    b8g8r8_srgb = VK_FORMAT_B8G8R8_SRGB,
    r8g8b8a8_unorm = VK_FORMAT_R8G8B8A8_UNORM,
    r8g8b8a8_snorm = VK_FORMAT_R8G8B8A8_SNORM,
    r8g8b8a8_uscaled = VK_FORMAT_R8G8B8A8_USCALED,
    r8g8b8a8_sscaled = VK_FORMAT_R8G8B8A8_SSCALED,
    r8g8b8a8_uint = VK_FORMAT_R8G8B8A8_UINT,
    r8g8b8a8_sint = VK_FORMAT_R8G8B8A8_SINT,
    r8g8b8a8_srgb = VK_FORMAT_R8G8B8A8_SRGB,
    b8g8r8a8_unorm = VK_FORMAT_B8G8R8A8_UNORM,
    b8g8r8a8_snorm = VK_FORMAT_B8G8R8A8_SNORM,
    b8g8r8a8_uscaled = VK_FORMAT_B8G8R8A8_USCALED,
    b8g8r8a8_sscaled = VK_FORMAT_B8G8R8A8_SSCALED,
    b8g8r8a8_uint = VK_FORMAT_B8G8R8A8_UINT,
    b8g8r8a8_sint = VK_FORMAT_B8G8R8A8_SINT,
    b8g8r8a8_srgb = VK_FORMAT_B8G8R8A8_SRGB,
    a8b8g8r8_unorm_pack = VK_FORMAT_A8B8G8R8_UNORM_PACK32,
    a8b8g8r8_snorm_pack = VK_FORMAT_A8B8G8R8_SNORM_PACK32,
    a8b8g8r8_uscaled_pack = VK_FORMAT_A8B8G8R8_USCALED_PACK32,
    a8b8g8r8_sscaled_pack = VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
    a8b8g8r8_uint_pack = VK_FORMAT_A8B8G8R8_UINT_PACK32,
    a8b8g8r8_sint_pack = VK_FORMAT_A8B8G8R8_SINT_PACK32,
    a8b8g8r8_srgb_pack = VK_FORMAT_A8B8G8R8_SRGB_PACK32,
    a2r10g10b10_unorm_pack = VK_FORMAT_A2R10G10B10_UNORM_PACK32,
    a2r10g10b10_snorm_pack = VK_FORMAT_A2R10G10B10_SNORM_PACK32,
    a2r10g10b10_uscaled_pack = VK_FORMAT_A2R10G10B10_USCALED_PACK32,
    a2r10g10b10_sscaled_pack = VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
    a2r10g10b10_uint_pack = VK_FORMAT_A2R10G10B10_UINT_PACK32,
    a2r10g10b10_sint_pack = VK_FORMAT_A2R10G10B10_SINT_PACK32,
    a2b10g10r10_unorm_pack = VK_FORMAT_A2B10G10R10_UNORM_PACK32,
    a2b10g10r10_snorm_pack = VK_FORMAT_A2B10G10R10_SNORM_PACK32,
    a2b10g10r10_uscaled_pack = VK_FORMAT_A2B10G10R10_USCALED_PACK32,
    a2b10g10r10_sscaled_pack = VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
    a2b10g10r10_uint_pack = VK_FORMAT_A2B10G10R10_UINT_PACK32,
    a2b10g10r10_sint_pack = VK_FORMAT_A2B10G10R10_SINT_PACK32,
    r16_unorm = VK_FORMAT_R16_UNORM,
    r16_snorm = VK_FORMAT_R16_SNORM,
    r16_uscaled = VK_FORMAT_R16_USCALED,
    r16_sscaled = VK_FORMAT_R16_SSCALED,
    r16_uint = VK_FORMAT_R16_UINT,
    r16_sint = VK_FORMAT_R16_SINT,
    r16_sfloat = VK_FORMAT_R16_SFLOAT,
    r16g16_unorm = VK_FORMAT_R16G16_UNORM,
    r16g16_snorm = VK_FORMAT_R16G16_SNORM,
    r16g16_uscaled = VK_FORMAT_R16G16_USCALED,
    r16g16_sscaled = VK_FORMAT_R16G16_SSCALED,
    r16g16_uint = VK_FORMAT_R16G16_UINT,
    r16g16_sint = VK_FORMAT_R16G16_SINT,
    r16g16_sfloat = VK_FORMAT_R16G16_SFLOAT,
    r16g16b16_unorm = VK_FORMAT_R16G16B16_UNORM,
    r16g16b16_snorm = VK_FORMAT_R16G16B16_SNORM,
    r16g16b16_uscaled = VK_FORMAT_R16G16B16_USCALED,
    r16g16b16_sscaled = VK_FORMAT_R16G16B16_SSCALED,
    r16g16b16_uint = VK_FORMAT_R16G16B16_UINT,
    r16g16b16_sint = VK_FORMAT_R16G16B16_SINT,
    r16g16b16_sfloat = VK_FORMAT_R16G16B16_SFLOAT,
    r16g16b16a16_unorm = VK_FORMAT_R16G16B16A16_UNORM,
    r16g16b16a16_snorm = VK_FORMAT_R16G16B16A16_SNORM,
    r16g16b16a16_uscaled = VK_FORMAT_R16G16B16A16_USCALED,
    r16g16b16a16_sscaled = VK_FORMAT_R16G16B16A16_SSCALED,
    r16g16b16a16_uint = VK_FORMAT_R16G16B16A16_UINT,
    r16g16b16a16_sint = VK_FORMAT_R16G16B16A16_SINT,
    r16g16b16a16_sfloat = VK_FORMAT_R16G16B16A16_SFLOAT,
    r32_uint = VK_FORMAT_R32_UINT,
    r32_sint = VK_FORMAT_R32_SINT,
    r32_sfloat = VK_FORMAT_R32_SFLOAT,
    r32g32_uint = VK_FORMAT_R32G32_UINT,
    r32g32_sint = VK_FORMAT_R32G32_SINT,
    r32g32_sfloat = VK_FORMAT_R32G32_SFLOAT,
    r32g32b32_uint = VK_FORMAT_R32G32B32_UINT,
    r32g32b32_sint = VK_FORMAT_R32G32B32_SINT,
    r32g32b32_sfloat = VK_FORMAT_R32G32B32_SFLOAT,
    r32g32b32a32_uint = VK_FORMAT_R32G32B32A32_UINT,
    r32g32b32a32_sint = VK_FORMAT_R32G32B32A32_SINT,
    r32g32b32a32_sfloat = VK_FORMAT_R32G32B32A32_SFLOAT,
    r64_uint = VK_FORMAT_R64_UINT,
    r64_sint = VK_FORMAT_R64_SINT,
    r64_sfloat = VK_FORMAT_R64_SFLOAT,
    r64g64_uint = VK_FORMAT_R64G64_UINT,
    r64g64_sint = VK_FORMAT_R64G64_SINT,
    r64g64_sfloat = VK_FORMAT_R64G64_SFLOAT,
    r64g64b64_uint = VK_FORMAT_R64G64B64_UINT,
    r64g64b64_sint = VK_FORMAT_R64G64B64_SINT,
    r64g64b64_sfloat = VK_FORMAT_R64G64B64_SFLOAT,
    r64g64b64a64_uint = VK_FORMAT_R64G64B64A64_UINT,
    r64g64b64a64_sint = VK_FORMAT_R64G64B64A64_SINT,
    r64g64b64a64_sfloat = VK_FORMAT_R64G64B64A64_SFLOAT,
    b10g11r11_ufloat_pack = VK_FORMAT_B10G11R11_UFLOAT_PACK32,
    e5b9g9r9_ufloat_pack = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
    d16_unorm = VK_FORMAT_D16_UNORM,
    x8_d24_unorm_pack = VK_FORMAT_X8_D24_UNORM_PACK32,
    d32_sfloat = VK_FORMAT_D32_SFLOAT,
    s8_uint = VK_FORMAT_S8_UINT,
    d16_unorm_s8_uint = VK_FORMAT_D16_UNORM_S8_UINT,
    d24_unorm_s8_uint = VK_FORMAT_D24_UNORM_S8_UINT,
    d32_sfloat_s8_uint = VK_FORMAT_D32_SFLOAT_S8_UINT,
};

enum class texture_aspect : std::uint32_t
{
    undefined = 0,
    color = VK_IMAGE_ASPECT_COLOR_BIT,
    depth = VK_IMAGE_ASPECT_DEPTH_BIT,
    stencil = VK_IMAGE_ASPECT_STENCIL_BIT,
};

enum class format_feature : std::uint32_t
{
    none = 0,
    sampled_image = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT,
    storage_image = VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT,
    storage_image_atomic = VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT,
    unifom_texel_buffer = VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT,
    storage_texel_buffer = VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT,
    storage_texel_buffer_atomic = VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT,
    vertex_buffer = VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT,
    color_attachment = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT,
    color_attachment_blend = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT,
    depth_stencil_attachment = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
    blit_source = VK_FORMAT_FEATURE_BLIT_SRC_BIT,
    blit_destination = VK_FORMAT_FEATURE_BLIT_DST_BIT,
    sampled_image_filter_linear = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
    transfer_source = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT,
    transfer_destination = VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
};

enum class component_swizzle : std::uint32_t
{
    identity = VK_COMPONENT_SWIZZLE_IDENTITY,
    zero = VK_COMPONENT_SWIZZLE_ZERO,
    one = VK_COMPONENT_SWIZZLE_ONE,
    r = VK_COMPONENT_SWIZZLE_R,
    g = VK_COMPONENT_SWIZZLE_G,
    b = VK_COMPONENT_SWIZZLE_B,
    a = VK_COMPONENT_SWIZZLE_A
};

enum class dependency_flags : std::uint32_t
{
    none = 0,
    by_region = VK_DEPENDENCY_BY_REGION_BIT,
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

enum class render_pass_content : std::uint32_t
{
    inlined = VK_SUBPASS_CONTENTS_INLINE,
    recorded = VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS,
};

enum class attachment_load_op : std::uint32_t
{
    load = VK_ATTACHMENT_LOAD_OP_LOAD,
    clear = VK_ATTACHMENT_LOAD_OP_CLEAR,
    dont_care = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
};

enum class attachment_store_op : std::uint32_t
{
    store = VK_ATTACHMENT_STORE_OP_STORE,
    dont_care = VK_ATTACHMENT_STORE_OP_DONT_CARE,
};

enum class query_type : std::uint32_t
{
    occlusion = VK_QUERY_TYPE_OCCLUSION,
    pipeline_statistics = VK_QUERY_TYPE_PIPELINE_STATISTICS,
    timestamp = VK_QUERY_TYPE_TIMESTAMP,
};

enum class query_pipeline_statistic : std::uint32_t
{
    none = 0,
    input_assembly_vertices = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT,
    input_assembly_primitives = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT,
    vertex_shader_invocations = VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT,
    geometry_shader_invocations = VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT,
    geometry_shader_primitives = VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT,
    clipping_invocations = VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT,
    clipping_primitives = VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT,
    fragment_shader_invocations = VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT,
    tessellation_control_shader_patches = VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT,
    tessellation_evaluation_shader_invocations = VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT,
    compute_shader_invocation = VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT,
};

enum class query_control : std::uint32_t
{
    none = 0,
    precise = VK_QUERY_CONTROL_PRECISE_BIT
};

enum class query_results : std::uint32_t
{
    none = 0,
    uint64 = VK_QUERY_RESULT_64_BIT,
    wait = VK_QUERY_RESULT_WAIT_BIT,
    with_availability = VK_QUERY_RESULT_WITH_AVAILABILITY_BIT,
    partial = VK_QUERY_RESULT_PARTIAL_BIT,
};

enum class stencil_face : std::uint32_t
{
    front = VK_STENCIL_FACE_FRONT_BIT,
    back = VK_STENCIL_FACE_BACK_BIT,
    front_and_back = VK_STENCIL_FACE_FRONT_AND_BACK,
};

struct viewport
{
    float x{};
    float y{};
    float width{};
    float height{};
    float min_depth{};
    float max_depth{};
};

struct scissor
{
    std::int32_t x{};
    std::int32_t y{};
    std::uint32_t width{};
    std::uint32_t height{};
};

struct texture_subresource_range
{
    std::uint32_t base_mip_level{};
    std::uint32_t mip_level_count{1};
    std::uint32_t base_array_layer{};
    std::uint32_t array_layer_count{1};
    texture_aspect aspect{};
};

struct clear_color_float_value
{
    float red{};
    float green{};
    float blue{};
    float alpha{};
};

struct clear_color_int_value
{
    std::int32_t red{};
    std::int32_t green{};
    std::int32_t blue{};
    std::int32_t alpha{};
};

struct clear_color_uint_value
{
    std::uint32_t red{};
    std::uint32_t green{};
    std::uint32_t blue{};
    std::uint32_t alpha{};
};

using clear_color_value = std::variant<clear_color_float_value, clear_color_int_value, clear_color_uint_value>;

struct clear_depth_stencil_value
{
    float depth{};
    std::uint32_t stencil{};
};

using clear_value_t = std::variant<clear_color_value, clear_depth_stencil_value>;

}

template<> struct tph::enable_enum_operations<tph::color_component> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::shader_stage> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::pipeline_stage> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::resource_access> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::texture_aspect> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::format_feature> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::dependency_flags> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::query_pipeline_statistic> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::query_control> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::query_results> {static constexpr bool value{true};};
template<> struct tph::enable_enum_operations<tph::stencil_face> {static constexpr bool value{true};};

#endif
