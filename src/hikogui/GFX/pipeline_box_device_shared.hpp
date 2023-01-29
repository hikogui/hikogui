// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../geometry/module.hpp"
#include "../vector_span.hpp"
#include "../color/module.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>

namespace hi::inline v1 {
class gfx_device_vulkan;

namespace pipeline_box {
struct Image;
struct vertex;

struct device_shared final {
    gfx_device_vulkan const &device;

    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    device_shared(gfx_device_vulkan const &device);
    ~device_shared();

    device_shared(device_shared const &) = delete;
    device_shared &operator=(device_shared const &) = delete;
    device_shared(device_shared &&) = delete;
    device_shared &operator=(device_shared &&) = delete;

    /*! Deallocate vulkan resources.
     * This is called in the destructor of gfx_device_vulkan, therefor we can not use our `device`.
     */
    void destroy(gfx_device_vulkan const*vulkanDevice);

    void drawInCommandBuffer(vk::CommandBuffer const &commandBuffer);

    static void place_vertices(
        vector_span<vertex> &vertices,
        aarectangle clipping_rectangle,
        quad box,
        quad_color fill_colors,
        quad_color line_colors,
        float line_width,
        corner_radii corner_radii);

private:
    void buildShaders();
    void teardownShaders(gfx_device_vulkan const*vulkanDevice);
};

} // namespace pipeline_box
} // namespace hi::inline v1
