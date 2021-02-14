// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_box.hpp"
#include "pipeline_box_device_shared.hpp"
#include "gui_device_vulkan.hpp"
#include "../pixel_map.hpp"
#include "../URL.hpp"
#include <array>

namespace tt::pipeline_box {

using namespace std;

device_shared::device_shared(gui_device_vulkan const &device) :
    device(device)
{
    buildShaders();
}

device_shared::~device_shared()
{
}

void device_shared::destroy(gui_device_vulkan *vulkanDevice)
{
    tt_axiom(vulkanDevice);
    teardownShaders(vulkanDevice);
}

void device_shared::drawInCommandBuffer(vk::CommandBuffer &commandBuffer)
{
    commandBuffer.bindIndexBuffer(device.quadIndexBuffer, 0, vk::IndexType::eUint16);
}

void device_shared::placeVertices(
    vspan<vertex> &vertices,
    rect box,
    color backgroundColor,
    float borderSize,
    color borderColor,
    f32x4 cornerShapes,
    aarect clippingRectangle
)
{
    ttlet extraSpace = (borderSize * 0.5f) + 1.0f;
    ttlet outerBox = expand(box, extraSpace);

    ttlet v0 = outerBox.corner<0>();
    ttlet v1 = outerBox.corner<1>();
    ttlet v2 = outerBox.corner<2>();
    ttlet v3 = outerBox.corner<3>();

    ttlet outerExtent = outerBox.extent();

    ttlet t0 = outerExtent._00xy();
    ttlet t1 = outerExtent.x00y();
    ttlet t2 = outerExtent._0yx0();
    ttlet t3 = outerExtent.xy00();

    vertices.emplace_back(v0, t0, backgroundColor, borderSize, borderColor, cornerShapes, clippingRectangle);
    vertices.emplace_back(v1, t1, backgroundColor, borderSize, borderColor, cornerShapes, clippingRectangle);
    vertices.emplace_back(v2, t2, backgroundColor, borderSize, borderColor, cornerShapes, clippingRectangle);
    vertices.emplace_back(v3, t3, backgroundColor, borderSize, borderColor, cornerShapes, clippingRectangle);
}

void device_shared::buildShaders()
{
    vertexShaderModule = device.loadShader(URL("resource:GUI/pipeline_box.vert.spv"));
    fragmentShaderModule = device.loadShader(URL("resource:GUI/pipeline_box.frag.spv"));

    shaderStages = {
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"},
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"}
    };
}

void device_shared::teardownShaders(gui_device_vulkan *vulkanDevice)
{
    tt_axiom(vulkanDevice);
    vulkanDevice->destroy(vertexShaderModule);
    vulkanDevice->destroy(fragmentShaderModule);
}

}
