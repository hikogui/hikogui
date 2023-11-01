// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <vulkan/vulkan.hpp>

export module hikogui_GFX : gfx_pipeline_box_impl;
import : draw_context_intf;
import : gfx_device_impl;
import : gfx_pipeline_box_intf;

export namespace hi { inline namespace v1 {

void gfx_pipeline_box::draw_in_command_buffer(vk::CommandBuffer commandBuffer, draw_context const& context)
{
    gfx_pipeline::draw_in_command_buffer(commandBuffer, context);

    hi_axiom_not_null(device());
    device()->flushAllocation(vertexBufferAllocation, 0, vertexBufferData.size() * sizeof(vertex));

    std::vector<vk::Buffer> tmpvertexBuffers = {vertexBuffer};
    std::vector<vk::DeviceSize> tmpOffsets = {0};
    hi_assert(tmpvertexBuffers.size() == tmpOffsets.size());

    device()->box_pipeline->drawInCommandBuffer(commandBuffer);

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

    device()->cmdBeginDebugUtilsLabelEXT(commandBuffer, "draw boxes");
    commandBuffer.drawIndexed(narrow_cast<uint32_t>(numberOfTriangles * 3), 1, 0, 0, 0);
    device()->cmdEndDebugUtilsLabelEXT(commandBuffer);
}

std::vector<vk::PipelineShaderStageCreateInfo> gfx_pipeline_box::createShaderStages() const
{
    hi_axiom_not_null(device());
    return device()->box_pipeline->shaderStages;
}

std::vector<vk::DescriptorSetLayoutBinding> gfx_pipeline_box::createDescriptorSetLayoutBindings() const
{
    return {};
}

std::vector<vk::WriteDescriptorSet> gfx_pipeline_box::createWriteDescriptorSet() const
{
    return {};
}

size_t gfx_pipeline_box::getDescriptorSetVersion() const
{
    return 0;
}

std::vector<vk::PushConstantRange> gfx_pipeline_box::createPushConstantRanges() const
{
    return push_constants::pushConstantRanges();
}

vk::VertexInputBindingDescription gfx_pipeline_box::createVertexInputBindingDescription() const
{
    return vertex::inputBindingDescription();
}

std::vector<vk::VertexInputAttributeDescription> gfx_pipeline_box::createVertexInputAttributeDescriptions() const
{
    return vertex::inputAttributeDescriptions();
}

void gfx_pipeline_box::build_vertex_buffers()
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
    allocationCreateInfo.pUserData = const_cast<char *>("box-pipeline vertex buffer");
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    hi_axiom_not_null(device());
    std::tie(vertexBuffer, vertexBufferAllocation) = device()->createBuffer(bufferCreateInfo, allocationCreateInfo);
    device()->setDebugUtilsObjectNameEXT(vertexBuffer, "box-pipeline vertex buffer");
    vertexBufferData = device()->mapMemory<vertex>(vertexBufferAllocation);
}

void gfx_pipeline_box::teardown_vertex_buffers()
{
    hi_axiom_not_null(device());
    device()->unmapMemory(vertexBufferAllocation);
    device()->destroyBuffer(vertexBuffer, vertexBufferAllocation);
}

gfx_pipeline_box::device_shared::device_shared(gfx_device const &device) : device(device)
{
    buildShaders();
}

gfx_pipeline_box::device_shared::~device_shared() {}

void gfx_pipeline_box::device_shared::destroy(gfx_device const *vulkanDevice)
{
    hi_assert_not_null(vulkanDevice);
    teardownShaders(vulkanDevice);
}

void gfx_pipeline_box::device_shared::drawInCommandBuffer(vk::CommandBuffer const &commandBuffer)
{
    commandBuffer.bindIndexBuffer(device.quadIndexBuffer, 0, vk::IndexType::eUint16);
}

void gfx_pipeline_box::device_shared::place_vertices(
    vector_span<vertex> &vertices,
    aarectangle clipping_rectangle,
    quad box,
    quad_color fill_colors,
    quad_color line_colors,
    float line_width,
    hi::corner_radii corner_radii)
{
    // Include the half line_width, so that the border is drawn centered
    // around the box outline. Then add 1 pixel for anti-aliasing.
    // The shader will compensate for the pixel and half the border.
    hilet extra_space = (line_width * 0.5f) + 1.0f;
    hilet[box_, lengths] = expand_and_edge_hypots(box, extent2{extra_space, extra_space});

    // t0-t3 are used inside the shader to determine how far from the corner
    // a certain fragment is.
    //
    // x = Number of pixels from the right edge.
    // y = Number of pixels above the bottom edge.
    // z = Number of pixels from the left edge.
    // w = Number of pixels below the top edge.
    hilet t0 = sfloat_rgba32{lengths._00xy()};
    hilet t1 = sfloat_rgba32{lengths.x00w()};
    hilet t2 = sfloat_rgba32{lengths._0yz0()};
    hilet t3 = sfloat_rgba32{lengths.zw00()};

    hilet clipping_rectangle_ = sfloat_rgba32{clipping_rectangle};
    hilet corner_radii_ = sfloat_rgba32{corner_radii};

    vertices.emplace_back(box_.p0, clipping_rectangle_, t0, corner_radii_, fill_colors.p0, line_colors.p0, line_width);
    vertices.emplace_back(box_.p1, clipping_rectangle_, t1, corner_radii_, fill_colors.p1, line_colors.p1, line_width);
    vertices.emplace_back(box_.p2, clipping_rectangle_, t2, corner_radii_, fill_colors.p2, line_colors.p2, line_width);
    vertices.emplace_back(box_.p3, clipping_rectangle_, t3, corner_radii_, fill_colors.p3, line_colors.p3, line_width);
}

void gfx_pipeline_box::device_shared::buildShaders()
{
    vertexShaderModule = device.loadShader(URL("resource:box_vulkan.vert.spv"));
    device.setDebugUtilsObjectNameEXT(vertexShaderModule, "box-pipeline vertex shader");

    fragmentShaderModule = device.loadShader(URL("resource:box_vulkan.frag.spv"));
    device.setDebugUtilsObjectNameEXT(vertexShaderModule, "box-pipeline fragment shader");

    shaderStages = {
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"},
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"}};
}

void gfx_pipeline_box::device_shared::teardownShaders(gfx_device const*vulkanDevice)
{
    hi_assert_not_null(vulkanDevice);
    vulkanDevice->destroy(vertexShaderModule);
    vulkanDevice->destroy(fragmentShaderModule);
}

}} // namespace hi::inline v1::gfx_pipeline_box
