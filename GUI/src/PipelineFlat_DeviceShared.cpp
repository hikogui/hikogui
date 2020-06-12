// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/PipelineFlat.hpp"
#include "TTauri/GUI/PipelineFlat_DeviceShared.hpp"
#include "TTauri/GUI/GUIDevice.hpp"
#include "TTauri/Foundation/PixelMap.hpp"
#include "TTauri/Foundation/URL.hpp"
#include <array>

namespace tt::PipelineFlat {

using namespace std;

DeviceShared::DeviceShared(GUIDevice const &device) :
    device(device)
{
    buildShaders();
}

DeviceShared::~DeviceShared()
{
}


void DeviceShared::destroy(GUIDevice *vulkanDevice)
{
    ttauri_assume(vulkanDevice);
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

void DeviceShared::teardownShaders(GUIDevice_vulkan *vulkanDevice)
{
    ttauri_assume(vulkanDevice);
    vulkanDevice->destroy(vertexShaderModule);
    vulkanDevice->destroy(fragmentShaderModule);
}

}
