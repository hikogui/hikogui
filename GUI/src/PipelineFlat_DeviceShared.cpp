// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/PipelineFlat.hpp"
#include "TTauri/GUI/PipelineFlat_DeviceShared.hpp"
#include "TTauri/GUI/Device.hpp"
#include "TTauri/Foundation/PixelMap.hpp"
#include "TTauri/Foundation/URL.hpp"
#include <glm/gtx/vec_swizzle.hpp>
#include <array>

namespace TTauri::GUI::PipelineFlat {

using namespace std;

DeviceShared::DeviceShared(Device const &device) :
    device(device)
{
    buildShaders();
}

DeviceShared::~DeviceShared()
{
}

void DeviceShared::placeVerticesBox(vspan<Vertex> &vertices, rect2 box, R16G16B16A16SFloat color, rect2 clippingRectangle, float depth) noexcept
{
    auto cr = glm::vec4{
        clippingRectangle.offset.x,
        clippingRectangle.offset.y,
        clippingRectangle.offset.x + clippingRectangle.extent.width(),
        clippingRectangle.offset.y + clippingRectangle.extent.height()
    };

    vertices.emplace_back(glm::vec3{box.corner<0>(), depth}, cr, color);
    vertices.emplace_back(glm::vec3{box.corner<1>(), depth}, cr, color);
    vertices.emplace_back(glm::vec3{box.corner<2>(), depth}, cr ,color);
    vertices.emplace_back(glm::vec3{box.corner<3>(), depth}, cr, color);
}

void DeviceShared::destroy(gsl::not_null<Device *> vulkanDevice)
{
    teardownShaders(vulkanDevice);
}

void DeviceShared::drawInCommandBuffer(vk::CommandBuffer &commandBuffer)
{
    commandBuffer.bindIndexBuffer(device.quadIndexBuffer, 0, vk::IndexType::eUint16);
}

void DeviceShared::buildShaders()
{
    vertexShaderModule = device.loadShader(URL("resource:GUI/PipelineFlat.vert.spv"));
    fragmentShaderModule = device.loadShader(URL("resource:GUI/PipelineFlat.frag.spv"));

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
