// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <vulkan/vulkan.hpp>

export module hikogui_GFX : gfx_pipeline_override_impl;
import : gfx_device_impl;
import : gfx_pipeline_override_intf;

export namespace hi { inline namespace v1 {

/* Do not blend, simply use just the alpha channel and overwrite the pixels in the color attachment directly.
 */
std::vector<vk::PipelineColorBlendAttachmentState> gfx_pipeline_override::getPipelineColorBlendAttachmentStates() const
{
    bool has_dual_source_blend = false;
    if (auto device_ = device()) {
        has_dual_source_blend = device_->device_features.dualSrcBlend;
    }
    
    return {
        {VK_TRUE, // blendEnable
         has_dual_source_blend ? vk::BlendFactor::eSrc1Color : vk::BlendFactor::eOne, // srcColorBlendFactor
         has_dual_source_blend ? vk::BlendFactor::eOneMinusSrc1Color : vk::BlendFactor::eZero, // dstColorBlendFactor
         vk::BlendOp::eAdd, // colorBlendOp
         has_dual_source_blend ? vk::BlendFactor::eSrc1Alpha : vk::BlendFactor::eOne, // srcAlphaBlendFactor
         has_dual_source_blend ? vk::BlendFactor::eOneMinusSrc1Alpha : vk::BlendFactor::eZero, // dstAlphaBlendFactor
         vk::BlendOp::eAdd, // alphaBlendOp
         vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
             vk::ColorComponentFlagBits::eA}};
}

void gfx_pipeline_override::draw_in_command_buffer(vk::CommandBuffer commandBuffer, draw_context const& context)
{
    gfx_pipeline::draw_in_command_buffer(commandBuffer, context);

    hi_axiom_not_null(device());
    device()->flushAllocation(vertexBufferAllocation, 0, vertexBufferData.size() * sizeof(vertex));

    std::vector<vk::Buffer> tmpvertexBuffers = {vertexBuffer};
    std::vector<vk::DeviceSize> tmpOffsets = {0};
    hi_assert(tmpvertexBuffers.size() == tmpOffsets.size());

    device()->override_pipeline->drawInCommandBuffer(commandBuffer);

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

std::vector<vk::PipelineShaderStageCreateInfo> gfx_pipeline_override::createShaderStages() const
{
    hi_axiom_not_null(device());
    return device()->override_pipeline->shaderStages;
}

std::vector<vk::DescriptorSetLayoutBinding> gfx_pipeline_override::createDescriptorSetLayoutBindings() const
{
    return {};
}

std::vector<vk::WriteDescriptorSet> gfx_pipeline_override::createWriteDescriptorSet() const
{
    return {};
}

size_t gfx_pipeline_override::getDescriptorSetVersion() const
{
    return 0;
}

std::vector<vk::PushConstantRange> gfx_pipeline_override::createPushConstantRanges() const
{
    return push_constants::pushConstantRanges();
}

vk::VertexInputBindingDescription gfx_pipeline_override::createVertexInputBindingDescription() const
{
    return vertex::inputBindingDescription();
}

std::vector<vk::VertexInputAttributeDescription> gfx_pipeline_override::createVertexInputAttributeDescriptions() const
{
    return vertex::inputAttributeDescriptions();
}

void gfx_pipeline_override::build_vertex_buffers()
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

void gfx_pipeline_override::teardown_vertex_buffers()
{
    hi_axiom_not_null(device());
    device()->unmapMemory(vertexBufferAllocation);
    device()->destroyBuffer(vertexBuffer, vertexBufferAllocation);
}

gfx_pipeline_override::device_shared::device_shared(gfx_device const& device) : device(device)
{
    buildShaders();
}

gfx_pipeline_override::device_shared::~device_shared() {}

void gfx_pipeline_override::device_shared::destroy(gfx_device const*vulkanDevice)
{
    hi_assert_not_null(vulkanDevice);
    teardownShaders(vulkanDevice);
}

void gfx_pipeline_override::device_shared::drawInCommandBuffer(vk::CommandBuffer const& commandBuffer)
{
    commandBuffer.bindIndexBuffer(device.quadIndexBuffer, 0, vk::IndexType::eUint16);
}

void gfx_pipeline_override::device_shared::place_vertices(vector_span<vertex>& vertices, aarectangle clipping_rectangle, quad box, quad_color color, quad_color blend_factor)
{
    hilet clipping_rectangle_ = sfloat_rgba32{clipping_rectangle};

    vertices.emplace_back(box.p0, clipping_rectangle_, color.p0, blend_factor.p0);
    vertices.emplace_back(box.p1, clipping_rectangle_, color.p1, blend_factor.p1);
    vertices.emplace_back(box.p2, clipping_rectangle_, color.p2, blend_factor.p2);
    vertices.emplace_back(box.p3, clipping_rectangle_, color.p3, blend_factor.p3);
}

void gfx_pipeline_override::device_shared::buildShaders()
{
    vertexShaderModule = device.loadShader(URL("resource:override_vulkan.vert.spv"));
    device.setDebugUtilsObjectNameEXT(vertexShaderModule, "alpha-pipeline vertex shader");

    fragmentShaderModule = device.loadShader(URL("resource:override_vulkan.frag.spv"));
    device.setDebugUtilsObjectNameEXT(vertexShaderModule, "alpha-pipeline fragment shader");

    shaderStages = {
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"},
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"}};
}

void gfx_pipeline_override::device_shared::teardownShaders(gfx_device const*vulkanDevice)
{
    hi_assert_not_null(vulkanDevice);
    vulkanDevice->destroy(vertexShaderModule);
    vulkanDevice->destroy(fragmentShaderModule);
}

}} // namespace hi::v1
