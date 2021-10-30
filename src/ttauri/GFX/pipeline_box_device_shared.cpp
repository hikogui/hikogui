// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_box.hpp"
#include "pipeline_box_device_shared.hpp"
#include "gfx_device_vulkan.hpp"
#include "../pixel_map.hpp"
#include "../URL.hpp"
#include "../geometry/corner_shapes.hpp"
#include <array>

namespace tt::pipeline_box {

using namespace std;

device_shared::device_shared(gfx_device_vulkan const &device) :
    device(device)
{
    buildShaders();
}

device_shared::~device_shared()
{
}

void device_shared::destroy(gfx_device_vulkan *vulkanDevice)
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
    quad box,
    quad_color fill_colors,
    quad_color line_colors,
    float line_width,
    tt::corner_shapes corner_shapes
)
{
    // Include the half line_width, so that the border is drawn centered
    // around the box outline. Then add 1 pixel for anti-aliasing.
    // The shader will compensate for the pixel and half the border.
    ttlet extra_space = (line_width * 0.5f) + 1.0f;
    ttlet [box_, lengths] = expand_and_edge_hypots(box, extent2{extra_space, extra_space});

    // t0-t3 are used inside the shader to determine how far from the corner
    // a certain fragment is.
    //
    // x = Number of pixels from the right edge.
    // y = Number of pixels above the bottom edge.
    // z = Number of pixels from the left edge.
    // w = Number of pixels below the top edge.
    ttlet t0 = sfloat_rgba32{lengths._00xy()};
    ttlet t1 = sfloat_rgba32{lengths.x00w()};
    ttlet t2 = sfloat_rgba32{lengths._0yz0()};
    ttlet t3 = sfloat_rgba32{lengths.zw00()};

    ttlet clipping_rectangle_ = sfloat_rgba32{clipping_rectangle};
    ttlet corner_shapes_ = uint_abgr8_pack{corner_shapes};

    vertices.emplace_back(box_.p0, clipping_rectangle_, t0, fill_colors.p0, line_colors.p0, corner_shapes_, line_width);
    vertices.emplace_back(box_.p1, clipping_rectangle_, t1, fill_colors.p1, line_colors.p1, corner_shapes_, line_width);
    vertices.emplace_back(box_.p2, clipping_rectangle_, t2, fill_colors.p2, line_colors.p2, corner_shapes_, line_width);
    vertices.emplace_back(box_.p3, clipping_rectangle_, t3, fill_colors.p3, line_colors.p3, corner_shapes_, line_width);
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

void device_shared::teardownShaders(gfx_device_vulkan *vulkanDevice)
{
    tt_axiom(vulkanDevice);
    vulkanDevice->destroy(vertexShaderModule);
    vulkanDevice->destroy(fragmentShaderModule);
}

}
