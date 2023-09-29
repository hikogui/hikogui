// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_pipeline_alpha_vulkan.hpp"
#include "gfx_device_vulkan_impl.hpp"
#include "../macros.hpp"

namespace hi { inline namespace v1 {

/* Do not blend, simply use just the alpha channel and overwrite the pixels in the color attachment directly.
 */
inline std::vector<vk::PipelineColorBlendAttachmentState> gfx_pipeline_alpha::getPipelineColorBlendAttachmentStates() const
{
    return {
        {VK_FALSE, // blendEnable
         vk::BlendFactor::eOne, // srcColorBlendFactor
         vk::BlendFactor::eZero, // dstColorBlendFactor
         vk::BlendOp::eAdd, // colorBlendOp
         vk::BlendFactor::eOne, // srcAlphaBlendFactor
         vk::BlendFactor::eZero, // dstAlphaBlendFactor
         vk::BlendOp::eAdd, // aphaBlendOp
         vk::ColorComponentFlagBits::eA}};
}

inline void gfx_pipeline_alpha::draw_in_command_buffer(vk::CommandBuffer commandBuffer, draw_context const& context)
{
    gfx_pipeline::draw_in_command_buffer(commandBuffer, context);

    hi_axiom_not_null(device());
    device()->flushAllocation(vertexBufferAllocation, 0, vertexBufferData.size() * sizeof(vertex));

    std::vector<vk::Buffer> tmpvertexBuffers = {vertexBuffer};
    std::vector<vk::DeviceSize> tmpOffsets = {0};
    hi_assert(tmpvertexBuffers.size() == tmpOffsets.size());

    device()->alpha_pipeline->drawInCommandBuffer(commandBuffer);

    commandBuffer.bindVertexBuffers(0, tmpvertexBuffers, tmpOffsets);

    pushConstants.windowExtent = extent2{narrow_cast<float>(extent.width), narrow_cast<float>(extent.height)};
    pushConstants.viewportScale = scale2{2.0f / extent.width, 2.0f / extent.height};
    commandBuffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0,
        sizeof(push_constants),
        &pushConstants);

    hilet numberOfRectangles = vertexBufferData.size() / 4;
    hilet numberOfTriangles = numberOfRectangles * 2;

    device()->cmdBeginDebugUtilsLabelEXT(commandBuffer, "draw alpha overlays");
    commandBuffer.drawIndexed(narrow_cast<uint32_t>(numberOfTriangles * 3), 1, 0, 0, 0);
    device()->cmdEndDebugUtilsLabelEXT(commandBuffer);
}

inline std::vector<vk::PipelineShaderStageCreateInfo> gfx_pipeline_alpha::createShaderStages() const
{
    hi_axiom_not_null(device());
    return device()->alpha_pipeline->shaderStages;
}

inline std::vector<vk::DescriptorSetLayoutBinding> gfx_pipeline_alpha::createDescriptorSetLayoutBindings() const
{
    return {};
}

inline std::vector<vk::WriteDescriptorSet> gfx_pipeline_alpha::createWriteDescriptorSet() const
{
    return {};
}

inline size_t gfx_pipeline_alpha::getDescriptorSetVersion() const
{
    return 0;
}

inline std::vector<vk::PushConstantRange> gfx_pipeline_alpha::createPushConstantRanges() const
{
    return push_constants::pushConstantRanges();
}

inline vk::VertexInputBindingDescription gfx_pipeline_alpha::createVertexInputBindingDescription() const
{
    return vertex::inputBindingDescription();
}

inline std::vector<vk::VertexInputAttributeDescription> gfx_pipeline_alpha::createVertexInputAttributeDescriptions() const
{
    return vertex::inputAttributeDescriptions();
}

inline void gfx_pipeline_alpha::build_vertex_buffers()
{
    using vertexIndexType = uint16_t;
    constexpr ssize_t numberOfVertices = 1 << (sizeof(vertexIndexType) * CHAR_BIT);

    vk::BufferCreateInfo const bufferCreateInfo = {
        vk::BufferCreateFlags(),
        sizeof(vertex) * numberOfVertices,
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::SharingMode::eExclusive};
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    allocationCreateInfo.pUserData = const_cast<char *>("alpha-pipeline vertex buffer");
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    hi_axiom_not_null(device());
    std::tie(vertexBuffer, vertexBufferAllocation) = device()->createBuffer(bufferCreateInfo, allocationCreateInfo);
    device()->setDebugUtilsObjectNameEXT(vertexBuffer, "alpha-pipeline vertex buffer");
    vertexBufferData = device()->mapMemory<vertex>(vertexBufferAllocation);
}

inline void gfx_pipeline_alpha::teardown_vertex_buffers()
{
    hi_axiom_not_null(device());
    device()->unmapMemory(vertexBufferAllocation);
    device()->destroyBuffer(vertexBuffer, vertexBufferAllocation);
}

inline gfx_pipeline_alpha::device_shared::device_shared(gfx_device const& device) : device(device)
{
    buildShaders();
}

inline gfx_pipeline_alpha::device_shared::~device_shared() {}

inline void gfx_pipeline_alpha::device_shared::destroy(gfx_device const*vulkanDevice)
{
    hi_assert_not_null(vulkanDevice);
    teardownShaders(vulkanDevice);
}

inline void gfx_pipeline_alpha::device_shared::drawInCommandBuffer(vk::CommandBuffer const& commandBuffer)
{
    commandBuffer.bindIndexBuffer(device.quadIndexBuffer, 0, vk::IndexType::eUint16);
}

inline void gfx_pipeline_alpha::device_shared::place_vertices(vector_span<vertex>& vertices, aarectangle clipping_rectangle, quad box, float alpha)
{
    hilet clipping_rectangle_ = sfloat_rgba32{clipping_rectangle};

    vertices.emplace_back(box.p0, clipping_rectangle_, alpha);
    vertices.emplace_back(box.p1, clipping_rectangle_, alpha);
    vertices.emplace_back(box.p2, clipping_rectangle_, alpha);
    vertices.emplace_back(box.p3, clipping_rectangle_, alpha);
}

inline void gfx_pipeline_alpha::device_shared::buildShaders()
{
    vertexShaderModule = device.loadShader(URL("resource:alpha_vulkan.vert.spv"));
    device.setDebugUtilsObjectNameEXT(vertexShaderModule, "alpha-pipeline vertex shader");

    fragmentShaderModule = device.loadShader(URL("resource:alpha_vulkan.frag.spv"));
    device.setDebugUtilsObjectNameEXT(vertexShaderModule, "alpha-pipeline fragment shader");

    shaderStages = {
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"},
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"}};
}

inline void gfx_pipeline_alpha::device_shared::teardownShaders(gfx_device const*vulkanDevice)
{
    hi_assert_not_null(vulkanDevice);
    vulkanDevice->destroy(vertexShaderModule);
    vulkanDevice->destroy(fragmentShaderModule);
}

}} // namespace hi::v1
