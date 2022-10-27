// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_alpha.hpp"
#include "pipeline_alpha_device_shared.hpp"
#include "gfx_device_vulkan.hpp"
#include "../file/URL.hpp"
#include "../geometry/corner_radii.hpp"
#include "../pixel_map.hpp"
#include <array>

namespace hi::inline v1::pipeline_alpha {

device_shared::device_shared(gfx_device_vulkan const& device) : device(device)
{
    buildShaders();
}

device_shared::~device_shared() {}

void device_shared::destroy(gfx_device_vulkan *vulkanDevice)
{
    hi_assert_not_null(vulkanDevice);
    teardownShaders(vulkanDevice);
}

void device_shared::drawInCommandBuffer(vk::CommandBuffer& commandBuffer)
{
    commandBuffer.bindIndexBuffer(device.quadIndexBuffer, 0, vk::IndexType::eUint16);
}

void device_shared::place_vertices(vector_span<vertex>& vertices, aarectangle clipping_rectangle, quad box, float alpha)
{
    hilet clipping_rectangle_ = sfloat_rgba32{clipping_rectangle};

    vertices.emplace_back(box.p0, clipping_rectangle_, alpha);
    vertices.emplace_back(box.p1, clipping_rectangle_, alpha);
    vertices.emplace_back(box.p2, clipping_rectangle_, alpha);
    vertices.emplace_back(box.p3, clipping_rectangle_, alpha);
}

void device_shared::buildShaders()
{
    vertexShaderModule = device.loadShader(URL("resource:shaders/pipeline_alpha.vert.spv"));
    device.setDebugUtilsObjectNameEXT(vertexShaderModule, "alpha-pipeline vertex shader");

    fragmentShaderModule = device.loadShader(URL("resource:shaders/pipeline_alpha.frag.spv"));
    device.setDebugUtilsObjectNameEXT(vertexShaderModule, "alpha-pipeline fragment shader");

    shaderStages = {
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"},
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"}};
}

void device_shared::teardownShaders(gfx_device_vulkan *vulkanDevice)
{
    hi_assert_not_null(vulkanDevice);
    vulkanDevice->destroy(vertexShaderModule);
    vulkanDevice->destroy(fragmentShaderModule);
}

} // namespace hi::inline v1::pipeline_alpha
