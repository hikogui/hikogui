// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_box.hpp"
#include "pipeline_box_device_shared.hpp"
#include "gui_device_vulkan.hpp"
#include "../pixel_map.hpp"
#include "../URL.hpp"
#include "../geometry/corner_shapes.hpp"
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

void device_shared::place_vertices(
    vspan<vertex> &vertices,
    aarectangle clipping_rectangle,
    rectangle box,
    color fill_color,
    color line_color,
    float line_width,
    tt::corner_shapes corner_shapes
)
{
    ttlet extra_space = (line_width * 0.5f) + 1.0f;
    ttlet outer_box = expand(box, extra_space);

    ttlet v0 = get<0>(outer_box);
    ttlet v1 = get<1>(outer_box);
    ttlet v2 = get<2>(outer_box);
    ttlet v3 = get<3>(outer_box);

    ttlet outer_extent = outer_box.extent();

    // x = Number of pixels to the right from the left edge of the quad.
    // y = Number of pixels above the bottom edge.
    // z = Number of pixels to the left from the right edge of the quad.
    // w = Number of pixels below the top edge.
    ttlet t0 = static_cast<f32x4>(outer_extent)._00xy();
    ttlet t1 = static_cast<f32x4>(outer_extent).x00y();
    ttlet t2 = static_cast<f32x4>(outer_extent)._0yx0();
    ttlet t3 = static_cast<f32x4>(outer_extent).xy00();

    vertices.emplace_back(clipping_rectangle, v0, t0, fill_color, line_color, line_width, corner_shapes);
    vertices.emplace_back(clipping_rectangle, v1, t1, fill_color, line_color, line_width, corner_shapes);
    vertices.emplace_back(clipping_rectangle, v2, t2, fill_color, line_color, line_width, corner_shapes);
    vertices.emplace_back(clipping_rectangle, v3, t3, fill_color, line_color, line_width, corner_shapes);
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
