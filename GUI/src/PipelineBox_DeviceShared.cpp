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
    buildShaders();
}

DeviceShared::~DeviceShared()
{
}

void DeviceShared::destroy(gsl::not_null<Device *> vulkanDevice)
{
    teardownShaders(vulkanDevice);
}

void DeviceShared::drawInCommandBuffer(vk::CommandBuffer &commandBuffer)
{
    commandBuffer.bindIndexBuffer(device.quadIndexBuffer, 0, vk::IndexType::eUint16);
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
