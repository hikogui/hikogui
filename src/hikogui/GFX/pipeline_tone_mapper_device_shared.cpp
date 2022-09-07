// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_tone_mapper.hpp"
#include "pipeline_tone_mapper_device_shared.hpp"
#include "gfx_device_vulkan.hpp"
#include "../pixel_map.hpp"
#include "../URL.hpp"
#include <array>

namespace hi::inline v1::pipeline_tone_mapper {

device_shared::device_shared(gfx_device_vulkan const &device) : device(device)
{
    buildShaders();
}

device_shared::~device_shared() {}

void device_shared::destroy(gfx_device_vulkan *vulkanDevice)
{
    hi_axiom(vulkanDevice);

    teardownShaders(vulkanDevice);
}

void device_shared::drawInCommandBuffer(vk::CommandBuffer &commandBuffer)
{
    commandBuffer.bindIndexBuffer(device.quadIndexBuffer, 0, vk::IndexType::eUint16);
}

void device_shared::buildShaders()
{
    vertexShaderModule = device.loadShader(URL("resource:GUI/pipeline_tone_mapper.vert.spv"));
    fragmentShaderModule = device.loadShader(URL("resource:GUI/pipeline_tone_mapper.frag.spv"));

    shaderStages = {
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"},
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"}};
}

void device_shared::teardownShaders(gfx_device_vulkan *vulkanDevice)
{
    hi_axiom(vulkanDevice);

    vulkanDevice->destroy(vertexShaderModule);
    vulkanDevice->destroy(fragmentShaderModule);
}

} // namespace hi::inline v1::pipeline_tone_mapper
