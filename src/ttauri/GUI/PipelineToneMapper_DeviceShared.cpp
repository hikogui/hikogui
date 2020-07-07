// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/GUI/PipelineToneMapper.hpp"
#include "ttauri/GUI/PipelineToneMapper_DeviceShared.hpp"
#include "ttauri/GUI/GUIDevice.hpp"
#include "ttauri/PixelMap.hpp"
#include "ttauri/URL.hpp"
#include <array>

namespace tt::PipelineToneMapper {

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
    tt_assume(vulkanDevice);

    teardownShaders(vulkanDevice);
}

void DeviceShared::drawInCommandBuffer(vk::CommandBuffer &commandBuffer)
{
    commandBuffer.bindIndexBuffer(device.quadIndexBuffer, 0, vk::IndexType::eUint16);
}

void DeviceShared::buildShaders()
{
    vertexShaderModule = device.loadShader(URL("resource:GUI/PipelineToneMapper.vert.spv"));
    fragmentShaderModule = device.loadShader(URL("resource:GUI/PipelineToneMapper.frag.spv"));

    shaderStages = {
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"},
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"}
    };
}

void DeviceShared::teardownShaders(GUIDevice_vulkan *vulkanDevice)
{
    tt_assume(vulkanDevice);

    vulkanDevice->destroy(vertexShaderModule);
    vulkanDevice->destroy(fragmentShaderModule);
}

}
