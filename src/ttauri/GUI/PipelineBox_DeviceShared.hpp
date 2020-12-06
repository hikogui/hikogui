// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "../required.hpp"
#include "../vec.hpp"
#include "../rect.hpp"
#include "../vspan.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>

namespace tt {
class gui_device_vulkan;
template<typename T> struct PixelMap;
}

namespace tt::PipelineBox {
struct Image;
struct Vertex;

struct DeviceShared final {
    gui_device_vulkan const &device;

    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    DeviceShared(gui_device_vulkan const &device);
    ~DeviceShared();

    DeviceShared(DeviceShared const &) = delete;
    DeviceShared &operator=(DeviceShared const &) = delete;
    DeviceShared(DeviceShared &&) = delete;
    DeviceShared &operator=(DeviceShared &&) = delete;

    /*! Deallocate vulkan resources.
    * This is called in the destructor of gui_device_vulkan, therefor we can not use our `device`.
    */
    void destroy(gui_device_vulkan *vulkanDevice);

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer);

    static void placeVertices(
        vspan<Vertex> &vertices,
        rect box,
        f32x4 backgroundColor,
        float borderSize,
        f32x4 borderColor,
        f32x4 cornerShapes,
        aarect clippingRectangle
    );

private:
    void buildShaders();
    void teardownShaders(gui_device_vulkan *vulkanDevice);
};

}
