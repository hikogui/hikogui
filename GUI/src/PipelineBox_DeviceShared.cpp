// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/PipelineBox.hpp"
#include "TTauri/GUI/PipelineBox_DeviceShared.hpp"
#include "TTauri/GUI/Device.hpp"
#include "TTauri/Foundation/PixelMap.hpp"
#include "TTauri/Foundation/URL.hpp"
#include <glm/gtx/vec_swizzle.hpp>
#include <array>

namespace TTauri::GUI::PipelineBox {

using namespace std;

DeviceShared::DeviceShared(Device const &device) :
    device(device)
{
    buildIndexBuffer();
    buildShaders();
}

DeviceShared::~DeviceShared()
{
}

void DeviceShared::destroy(gsl::not_null<Device *> vulkanDevice)
{
    teardownIndexBuffer(vulkanDevice);
    teardownShaders(vulkanDevice);
}

void DeviceShared::drawInCommandBuffer(vk::CommandBuffer &commandBuffer)
{
    commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint16);
}

void DeviceShared::placeVertices(
    vspan<Vertex> &vertices,
    float depth,
    rect2 box,
    R16G16B16A16SFloat backgroundColor,
    float borderSize,
    R16G16B16A16SFloat borderColor,
    float shadowSize,
    R16G16B16A16SFloat cornerShapes,
    rect2 clippingRectangle
)
{
    let extraSpace = (borderSize * 0.5f) + shadowSize + 1.0f;
    let outerBox = box.expand(extraSpace);

    let v0 = glm::vec3{outerBox.corner<0>(), depth};
    let v1 = glm::vec3{outerBox.corner<1>(), depth};
    let v2 = glm::vec3{outerBox.corner<2>(), depth};
    let v3 = glm::vec3{outerBox.corner<3>(), depth};

    let t0 = glm::vec4{0.0f, 0.0f, outerBox.width(), outerBox.height()};
    let t1 = glm::vec4{outerBox.width(), 0.0f, 0.0f, outerBox.height()};
    let t2 = glm::vec4{0.0f, outerBox.height(), outerBox.width(), 0.0f};
    let t3 = glm::vec4{outerBox.width(), outerBox.height(), 0.0f, 0.0f};

    vertices.emplace_back(v0, t0, backgroundColor, borderSize, borderColor, shadowSize, cornerShapes, clippingRectangle);
    vertices.emplace_back(v1, t1, backgroundColor, borderSize, borderColor, shadowSize, cornerShapes, clippingRectangle);
    vertices.emplace_back(v2, t2, backgroundColor, borderSize, borderColor, shadowSize, cornerShapes, clippingRectangle);
    vertices.emplace_back(v3, t3, backgroundColor, borderSize, borderColor, shadowSize, cornerShapes, clippingRectangle);
}

void DeviceShared::buildIndexBuffer()
{
    // Create vertex index buffer
    {
        vk::BufferCreateInfo const bufferCreateInfo = {
            vk::BufferCreateFlags(),
            sizeof (uint16_t) * PipelineBox::PipelineBox::maximumNumberOfIndices,
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            vk::SharingMode::eExclusive
        };
        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        tie(indexBuffer, indexBufferAllocation) = device.createBuffer(bufferCreateInfo, allocationCreateInfo);
    }

    // Fill in the vertex index buffer, using a staging buffer, then copying.
    {
        // Create staging vertex index buffer.
        vk::BufferCreateInfo const bufferCreateInfo = {
            vk::BufferCreateFlags(),
            sizeof (uint16_t) * PipelineBox::PipelineBox::maximumNumberOfIndices,
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferSrc,
            vk::SharingMode::eExclusive
        };
        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        let [stagingVertexIndexBuffer, stagingVertexIndexBufferAllocation] = device.createBuffer(bufferCreateInfo, allocationCreateInfo);

        // Initialize indices.
        let stagingVertexIndexBufferData = device.mapMemory<uint16_t>(stagingVertexIndexBufferAllocation);
        for (size_t i = 0; i < PipelineBox::PipelineBox::maximumNumberOfIndices; i++) {
            let vertexInRectangle = i % 6;
            let rectangleNr = i / 6;
            let rectangleBase = rectangleNr * 4;

            //   2---3
            //   | \ |
            //   0---1
            switch (vertexInRectangle) {
            case 0: gsl::at(stagingVertexIndexBufferData, i) = numeric_cast<uint16_t>(rectangleBase + 0); break;
            case 1: gsl::at(stagingVertexIndexBufferData, i) = numeric_cast<uint16_t>(rectangleBase + 1); break;
            case 2: gsl::at(stagingVertexIndexBufferData, i) = numeric_cast<uint16_t>(rectangleBase + 2); break;
            case 3: gsl::at(stagingVertexIndexBufferData, i) = numeric_cast<uint16_t>(rectangleBase + 2); break;
            case 4: gsl::at(stagingVertexIndexBufferData, i) = numeric_cast<uint16_t>(rectangleBase + 1); break;
            case 5: gsl::at(stagingVertexIndexBufferData, i) = numeric_cast<uint16_t>(rectangleBase + 3); break;
            default: no_default;
            }
        }
        device.flushAllocation(stagingVertexIndexBufferAllocation, 0, VK_WHOLE_SIZE);
        device.unmapMemory(stagingVertexIndexBufferAllocation);

        // Copy indices to vertex index buffer.
        auto commands = device.allocateCommandBuffers({
            device.graphicsCommandPool, 
            vk::CommandBufferLevel::ePrimary, 
            1
            }).at(0);
        commands.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        commands.copyBuffer(stagingVertexIndexBuffer, indexBuffer, {{0, 0, sizeof (uint16_t) * PipelineBox::PipelineBox::maximumNumberOfIndices}});
        commands.end();

        vector<vk::CommandBuffer> const commandBuffersToSubmit = { commands };
        vector<vk::SubmitInfo> const submitInfo = { { 0, nullptr, nullptr, numeric_cast<uint32_t>(commandBuffersToSubmit.size()), commandBuffersToSubmit.data(), 0, nullptr } };
        device.graphicsQueue.submit(submitInfo, vk::Fence());
        device.graphicsQueue.waitIdle();

        device.freeCommandBuffers(device.graphicsCommandPool, {commands});
        device.destroyBuffer(stagingVertexIndexBuffer, stagingVertexIndexBufferAllocation);
    }
}

void DeviceShared::teardownIndexBuffer(gsl::not_null<Device_vulkan *> vulkanDevice)
{
    vulkanDevice->destroyBuffer(indexBuffer, indexBufferAllocation);
}

void DeviceShared::buildShaders()
{
    vertexShaderModule = device.loadShader(URL("resource:GUI/PipelineBox.vert.spv"));
    fragmentShaderModule = device.loadShader(URL("resource:GUI/PipelineBox.frag.spv"));

    shaderStages = {
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"},
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"}
    };
}

void DeviceShared::teardownShaders(gsl::not_null<Device_vulkan *> vulkanDevice)
{
    vulkanDevice->destroy(vertexShaderModule);
    vulkanDevice->destroy(fragmentShaderModule);
}

}
