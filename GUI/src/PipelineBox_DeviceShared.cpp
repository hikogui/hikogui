// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/PipelineBox.hpp"
#include "TTauri/GUI/PipelineBox_DeviceShared.hpp"
#include "TTauri/GUI/Device.hpp"
#include "TTauri/Foundation/PixelMap.hpp"
#include "TTauri/Foundation/URL.hpp"
#include <array>

namespace TTauri::GUI::PipelineBox {

using namespace std;

DeviceShared::DeviceShared(Device const &device) :
    device(device)
{
    buildShaders();
}

DeviceShared::~DeviceShared()
{
}

void DeviceShared::destroy(Device *vulkanDevice)
{
    ttauri_assume(vulkanDevice);
    teardownShaders(vulkanDevice);
}

void DeviceShared::drawInCommandBuffer(vk::CommandBuffer &commandBuffer)
{
    commandBuffer.bindIndexBuffer(device.quadIndexBuffer, 0, vk::IndexType::eUint16);
}

void DeviceShared::placeVertices(
    vspan<Vertex> &vertices,
    rect box,
    vec backgroundColor,
    float borderSize,
    vec borderColor,
    vec cornerShapes,
    aarect clippingRectangle
)
{
    let extraSpace = (borderSize * 0.5f) + 1.0f;
    let outerBox = expand(box, extraSpace);

    let v0 = outerBox.corner<0>();
    let v1 = outerBox.corner<1>();
    let v2 = outerBox.corner<2>();
    let v3 = outerBox.corner<3>();

    let outerExtent = outerBox.extent();

    let t0 = outerExtent._00xy();
    let t1 = outerExtent.x00y();
    let t2 = outerExtent._0yx0();
    let t3 = outerExtent.xy00();

    vertices.emplace_back(v0, t0, backgroundColor, borderSize, borderColor, cornerShapes, clippingRectangle);
    vertices.emplace_back(v1, t1, backgroundColor, borderSize, borderColor, cornerShapes, clippingRectangle);
    vertices.emplace_back(v2, t2, backgroundColor, borderSize, borderColor, cornerShapes, clippingRectangle);
    vertices.emplace_back(v3, t3, backgroundColor, borderSize, borderColor, cornerShapes, clippingRectangle);
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

void DeviceShared::teardownShaders(Device_vulkan *vulkanDevice)
{
    ttauri_assume(vulkanDevice);
    vulkanDevice->destroy(vertexShaderModule);
    vulkanDevice->destroy(fragmentShaderModule);
}

}
