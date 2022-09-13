// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility.hpp"
#include "../geometry/rectangle.hpp"
#include "../vector_span.hpp"
#include "../color/color.hpp"
#include "../color/quad_color.hpp"
#include "../geometry/corner_radii.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>

namespace hi::inline v1 {
class gfx_device_vulkan;

namespace pipeline_alpha {
struct Image;
struct vertex;

struct device_shared final {
    gfx_device_vulkan const& device;

    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    device_shared(gfx_device_vulkan const& device);
    ~device_shared();

    device_shared(device_shared const&) = delete;
    device_shared& operator=(device_shared const&) = delete;
    device_shared(device_shared&&) = delete;
    device_shared& operator=(device_shared&&) = delete;

    /*! Deallocate vulkan resources.
     * This is called in the destructor of gfx_device_vulkan, therefor we can not use our `device`.
     */
    void destroy(gfx_device_vulkan *vulkanDevice);

    void drawInCommandBuffer(vk::CommandBuffer& commandBuffer);

    static void place_vertices(vector_span<vertex>& vertices, aarectangle clipping_rectangle, quad box, float alpha);

private:
    void buildShaders();
    void teardownShaders(gfx_device_vulkan *vulkanDevice);
};

} // namespace pipeline_alpha
} // namespace hi::inline v1
