#include "pipeline.hpp"

#include <fstream>

#include <captal_foundation/stack_allocator.hpp>
#include <captal_foundation/utility.hpp>

#include "vulkan/vulkan_functions.hpp"

#include "renderer.hpp"
#include "shader.hpp"
#include "render_target.hpp"
#include "descriptor.hpp"

using namespace tph::vulkan::functions;

namespace tph
{

pipeline_layout::pipeline_layout(renderer& renderer, std::span<const std::reference_wrapper<descriptor_set_layout>> layouts, std::span<const push_constant_range> ranges)
{
    stack_memory_pool<1024> pool{};

    auto native_layouts{make_stack_vector<VkDescriptorSetLayout>(pool)};
    native_layouts.reserve(std::size(layouts));
    for(const descriptor_set_layout& layout : layouts)
    {
        native_layouts.emplace_back(underlying_cast<VkDescriptorSetLayout>(layout));
    }

    auto native_ranges{make_stack_vector<VkPushConstantRange>(pool)};
    native_ranges.reserve(std::size(ranges));
    for(auto&& range : ranges)
    {
        VkPushConstantRange native_range{};
        native_range.stageFlags = static_cast<VkShaderStageFlags>(range.stages);
        native_range.offset = range.offset;
        native_range.size = range.size;

        native_ranges.emplace_back(native_range);
    }

    m_pipeline_layout = vulkan::pipeline_layout{underlying_cast<VkDevice>(renderer), native_layouts, native_ranges};
}

pipeline_cache::pipeline_cache(renderer& renderer)
:m_device{underlying_cast<VkDevice>(renderer)}
,m_pipeline_cache{m_device}
{

}

pipeline_cache::pipeline_cache(renderer& renderer, std::span<const uint8_t> data)
:m_device{underlying_cast<VkDevice>(renderer)}
,m_pipeline_cache{m_device, std::data(data), std::size(data)}
{

}

pipeline_cache::pipeline_cache(renderer& renderer, const std::filesystem::path& file)
:pipeline_cache{renderer, read_file<std::vector<std::uint8_t>>(file)}
{

}

pipeline_cache::pipeline_cache(renderer& renderer, std::istream& stream)
:m_device{underlying_cast<VkDevice>(renderer)}
{
    assert(stream && "Invalid stream.");

    const std::string initial_data{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};
    m_pipeline_cache = vulkan::pipeline_cache{m_device, std::data(initial_data), std::size(initial_data)};
}

pipeline_cache& pipeline_cache::merge_with(pipeline_cache& other)
{
    VkPipelineCache native_pipeline_cache{other.m_pipeline_cache};

    if(auto result{vkMergePipelineCaches(m_device, m_pipeline_cache, 1, &native_pipeline_cache)}; result != VK_SUCCESS)
        throw vulkan::error{result};

    return *this;
}

pipeline_cache& pipeline_cache::merge_with(const std::vector<std::reference_wrapper<pipeline_cache>>& others)
{
    std::vector<VkPipelineCache> native_pipeline_caches{};
    native_pipeline_caches.reserve(std::size(others));
    for(const pipeline_cache& other : others)
    {
        native_pipeline_caches.emplace_back(other.m_pipeline_cache);
    }

    if(auto result{vkMergePipelineCaches(m_device, m_pipeline_cache, static_cast<std::uint32_t>(std::size(native_pipeline_caches)), std::data(native_pipeline_caches))}; result != VK_SUCCESS)
        throw vulkan::error{result};

    return *this;
}

std::string pipeline_cache::data() const
{
    std::size_t size{};
    if(auto result{vkGetPipelineCacheData(m_device, m_pipeline_cache, &size, nullptr)}; result != VK_SUCCESS)
        throw vulkan::error{result};

    std::string output{};
    output.resize(size);

    if(auto result{vkGetPipelineCacheData(m_device, m_pipeline_cache, &size, std::data(output))}; result != VK_SUCCESS)
        throw vulkan::error{result};

    return output;
}

pipeline::pipeline(renderer& renderer, render_pass& render_pass, const graphics_pipeline_info& info, const pipeline_layout& layout, optional_ref<pipeline_cache> cache, optional_ref<pipeline> parent)
:pipeline{renderer, render_pass, 0, info, layout, cache, parent}
{

}

pipeline::pipeline(renderer& renderer, render_pass& render_pass, std::uint32_t subpass, const graphics_pipeline_info& info, const pipeline_layout& layout, optional_ref<pipeline_cache> cache, optional_ref<pipeline> parent)
:m_type{pipeline_type::graphics}
{
    VkGraphicsPipelineCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.flags = static_cast<VkPipelineCreateFlags>(info.options);

    stack_memory_pool<1024 / 2> pool{};

    auto stages_specialization{make_stack_vector<VkSpecializationInfo>(pool)};
    stages_specialization.reserve(std::size(info.stages));

    auto stages{make_stack_vector<VkPipelineShaderStageCreateInfo>(pool)};
    stages.reserve(std::size(info.stages));

    for(auto&& stage : info.stages)
    {
        auto& native_stage{stages.emplace_back()};
        native_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        native_stage.stage = static_cast<VkShaderStageFlagBits>(stage.shader.stage());
        native_stage.module = underlying_cast<VkShaderModule>(stage.shader);
        native_stage.pName = reinterpret_cast<const char*>(std::data(stage.name));

        if(!std::empty(stage.specialisation_info.entries))
        {
            auto& specialisation_info{stages_specialization.emplace_back()};
            specialisation_info.mapEntryCount = static_cast<std::uint32_t>(std::size(stage.specialisation_info.entries));
            specialisation_info.pMapEntries = reinterpret_cast<const VkSpecializationMapEntry*>(std::data(stage.specialisation_info.entries));
            specialisation_info.dataSize = stage.specialisation_info.size;
            specialisation_info.pData = stage.specialisation_info.data;

            native_stage.pSpecializationInfo = &specialisation_info;
        }
    }

    create_info.stageCount = static_cast<std::uint32_t>(std::size(stages));
    create_info.pStages = std::data(stages);

    VkPipelineVertexInputStateCreateInfo vertex_input{};
    vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input.vertexBindingDescriptionCount = static_cast<std::uint32_t>(std::size(info.vertex_input.bindings));
    vertex_input.pVertexBindingDescriptions = reinterpret_cast<const VkVertexInputBindingDescription*>(std::data(info.vertex_input.bindings));
    vertex_input.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(std::size(info.vertex_input.attributes));
    vertex_input.pVertexAttributeDescriptions = reinterpret_cast<const VkVertexInputAttributeDescription*>(std::data(info.vertex_input.attributes));

    create_info.pVertexInputState = &vertex_input;

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = static_cast<VkPrimitiveTopology>(info.input_assembly.topology);
    input_assembly.primitiveRestartEnable = static_cast<VkBool32>(info.input_assembly.primitive_restart);

    create_info.pInputAssemblyState = &input_assembly;

    VkPipelineTessellationStateCreateInfo tesselation{};
    tesselation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tesselation.patchControlPoints = info.tesselation.patch_control_points;

    create_info.pTessellationState = &tesselation;

    VkPipelineViewportStateCreateInfo viewport{};
    viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport.viewportCount = static_cast<std::uint32_t>(info.viewport.viewport_count);
    viewport.pViewports = reinterpret_cast<const VkViewport*>(std::data(info.viewport.viewports));
    viewport.scissorCount = static_cast<std::uint32_t>(info.viewport.viewport_count);
    viewport.pScissors = reinterpret_cast<const VkRect2D*>(std::data(info.viewport.scissors));

    create_info.pViewportState = &viewport;

    VkPipelineRasterizationStateCreateInfo rasterization{};
    rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization.depthClampEnable = static_cast<VkBool32>(info.rasterization.depth_clamp);
    rasterization.rasterizerDiscardEnable = static_cast<VkBool32>(info.rasterization.rasterizer_discard);
    rasterization.polygonMode = static_cast<VkPolygonMode>(info.rasterization.polygon_mode);
    rasterization.cullMode = static_cast<VkCullModeFlags>(info.rasterization.cull_mode);
    rasterization.frontFace = static_cast<VkFrontFace>(info.rasterization.front_face);
    rasterization.depthBiasEnable = static_cast<VkBool32>(info.rasterization.depth_bias);
    rasterization.depthBiasConstantFactor = info.rasterization.depth_bias_constant_factor;
    rasterization.depthBiasClamp = info.rasterization.depth_bias_clamp;
    rasterization.depthBiasSlopeFactor = info.rasterization.depth_bias_slope_factor;
    rasterization.lineWidth = info.rasterization.line_width;

    create_info.pRasterizationState = &rasterization;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = static_cast<VkSampleCountFlagBits>(info.multisample.sample_count);
    multisampling.sampleShadingEnable = static_cast<VkBool32>(info.multisample.sample_shading != 0.0f);
    multisampling.minSampleShading = info.multisample.sample_shading;
    multisampling.pSampleMask = info.multisample.sample_mask;
    multisampling.alphaToCoverageEnable = static_cast<VkBool32>(info.multisample.alpha_to_coverage);
    multisampling.alphaToOneEnable = static_cast<VkBool32>(info.multisample.alpha_to_one);

    create_info.pMultisampleState = &multisampling;

    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = static_cast<VkBool32>(info.depth_stencil.depth_test);
    depth_stencil.depthWriteEnable = static_cast<VkBool32>(info.depth_stencil.depth_write);
    depth_stencil.depthCompareOp = static_cast<VkCompareOp>(info.depth_stencil.depth_compare_op);
    depth_stencil.depthBoundsTestEnable = static_cast<VkBool32>(info.depth_stencil.depth_bounds_test);
    depth_stencil.stencilTestEnable = static_cast<VkBool32>(info.depth_stencil.stencil_test);
    depth_stencil.front.failOp = static_cast<VkStencilOp>(info.depth_stencil.front.fail_op);
    depth_stencil.front.passOp = static_cast<VkStencilOp>(info.depth_stencil.front.pass_op);
    depth_stencil.front.depthFailOp = static_cast<VkStencilOp>(info.depth_stencil.front.depth_fail_op);
    depth_stencil.front.compareOp = static_cast<VkCompareOp>(info.depth_stencil.front.compare_op);
    depth_stencil.front.compareMask = info.depth_stencil.front.compare_mask;
    depth_stencil.front.writeMask = info.depth_stencil.front.write_mask;
    depth_stencil.front.reference = info.depth_stencil.front.reference;
    depth_stencil.back.failOp = static_cast<VkStencilOp>(info.depth_stencil.back.fail_op);
    depth_stencil.back.passOp = static_cast<VkStencilOp>(info.depth_stencil.back.pass_op);
    depth_stencil.back.depthFailOp = static_cast<VkStencilOp>(info.depth_stencil.back.depth_fail_op);
    depth_stencil.back.compareOp = static_cast<VkCompareOp>(info.depth_stencil.back.compare_op);
    depth_stencil.back.compareMask = info.depth_stencil.back.compare_mask;
    depth_stencil.back.writeMask = info.depth_stencil.back.write_mask;
    depth_stencil.back.reference = info.depth_stencil.back.reference;
    depth_stencil.minDepthBounds = info.depth_stencil.min_depth_bounds;
    depth_stencil.maxDepthBounds = info.depth_stencil.max_depth_bounds;

    create_info.pDepthStencilState = &depth_stencil;

    std::vector<VkPipelineColorBlendAttachmentState> attachements{};
    attachements.reserve(std::size(info.color_blend.attachments));

    for(auto&& attachment : info.color_blend.attachments)
    {
        VkPipelineColorBlendAttachmentState& native_attachment{attachements.emplace_back()};
        native_attachment.blendEnable = static_cast<VkBool32>(attachment.blend);
        native_attachment.srcColorBlendFactor = static_cast<VkBlendFactor>(attachment.source_color_blend_factor);
        native_attachment.dstColorBlendFactor = static_cast<VkBlendFactor>(attachment.destination_color_blend_factor);
        native_attachment.colorBlendOp = static_cast<VkBlendOp>(attachment.color_blend_op);
        native_attachment.srcAlphaBlendFactor = static_cast<VkBlendFactor>(attachment.source_alpha_blend_factor);
        native_attachment.dstAlphaBlendFactor = static_cast<VkBlendFactor>(attachment.destination_alpha_blend_factor);
        native_attachment.alphaBlendOp = static_cast<VkBlendOp>(attachment.alpha_blend_op);
        native_attachment.colorWriteMask = static_cast<VkColorComponentFlags>(attachment.color_write_mask);
    }

    VkPipelineColorBlendStateCreateInfo color_blend{};
    color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend.logicOpEnable = static_cast<VkBool32>(info.color_blend.logic_op_enable);
    color_blend.logicOp = static_cast<VkLogicOp>(info.color_blend.logic_op);
    color_blend.attachmentCount = static_cast<std::uint32_t>(std::size(attachements));
    color_blend.pAttachments = std::data(attachements);
    std::copy(std::begin(info.color_blend.blend_constants), std::end(info.color_blend.blend_constants), color_blend.blendConstants);

    create_info.pColorBlendState = &color_blend;

    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = static_cast<std::uint32_t>(std::size(info.dynamic_states));
    dynamic_state.pDynamicStates = reinterpret_cast<const VkDynamicState*>(std::data(info.dynamic_states));

    create_info.pDynamicState = &dynamic_state;

    create_info.layout = underlying_cast<VkPipelineLayout>(layout);
    create_info.renderPass = underlying_cast<VkRenderPass>(render_pass);
    create_info.subpass = subpass;

    if(parent.has_value())
    {
        create_info.flags |= VK_PIPELINE_CREATE_DERIVATIVE_BIT;
        create_info.basePipelineHandle = parent.value().m_pipeline;
    }

    VkPipelineCache native_cache{cache.has_value() ? underlying_cast<VkPipelineCache>(cache.value()) : VkPipelineCache{}};

    m_pipeline = vulkan::pipeline{underlying_cast<VkDevice>(renderer), create_info, native_cache};
}

pipeline::pipeline(renderer& renderer, const compute_pipeline_info& info, const pipeline_layout& layout, optional_ref<pipeline_cache> cache, optional_ref<pipeline> parent)
:m_type{pipeline_type::compute}
{
    VkComputePipelineCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    create_info.flags = static_cast<VkPipelineCreateFlags>(info.options);

    create_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    create_info.stage.stage = static_cast<VkShaderStageFlagBits>(info.stage.shader.stage());
    create_info.stage.module = underlying_cast<VkShaderModule>(info.stage.shader);
    create_info.stage.pName = reinterpret_cast<const char*>(std::data(info.stage.name));

    VkSpecializationInfo specialisation_info{};
    if(!std::empty(info.stage.specialisation_info.entries))
    {
        specialisation_info.mapEntryCount = static_cast<std::uint32_t>(std::size(info.stage.specialisation_info.entries));
        specialisation_info.pMapEntries = reinterpret_cast<const VkSpecializationMapEntry*>(std::data(info.stage.specialisation_info.entries));
        specialisation_info.dataSize = info.stage.specialisation_info.size;
        specialisation_info.pData = info.stage.specialisation_info.data;

        create_info.stage.pSpecializationInfo = &specialisation_info;
    }

    create_info.layout = underlying_cast<VkPipelineLayout>(layout);

    if(parent.has_value())
    {
        create_info.flags |= VK_PIPELINE_CREATE_DERIVATIVE_BIT;
        create_info.basePipelineHandle = parent.value().m_pipeline;
    }

    VkPipelineCache native_cache{cache.has_value() ? underlying_cast<VkPipelineCache>(cache.value()) : VkPipelineCache{}};

    m_pipeline = vulkan::pipeline{underlying_cast<VkDevice>(renderer), create_info, native_cache};
}

}
