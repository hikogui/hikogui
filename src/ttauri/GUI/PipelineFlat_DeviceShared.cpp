// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "PipelineFlat.hpp"
#include "PipelineFlat_DeviceShared.hpp"
#include "gui_device_vulkan.hpp"
#include "../pixel_map.hpp"
#include "../URL.hpp"
#include <array>

namespace tt::PipelineFlat {

using namespace std;

DeviceShared::DeviceShared(gui_device_vulkan const &device) :
    device(device)
{
    buildShaders();
}

DeviceShared::~DeviceShared()
{
}


void DeviceShared::destroy(gui_device_vulkan *vulkanDevice)
{
    tt_axiom(vulkanDevice);
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

void DeviceShared::teardownShaders(gui_device_vulkan *vulkanDevice)
{
    tt_axiom(vulkanDevice);
    vulkanDevice->destroy(vertexShaderModule);
    vulkanDevice->destroy(fragmentShaderModule);
}

}
