// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Device_forward.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/rect.hpp"
#include "TTauri/Foundation/vspan.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>

namespace TTauri {
template<typename T> struct PixelMap;
}

namespace TTauri::GUI::PipelineBox {

struct Image;
struct Vertex;

struct DeviceShared final {
    Device const &device;

    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    DeviceShared(Device const &device);
    ~DeviceShared();

    DeviceShared(DeviceShared const &) = delete;
    DeviceShared &operator=(DeviceShared const &) = delete;
    DeviceShared(DeviceShared &&) = delete;
    DeviceShared &operator=(DeviceShared &&) = delete;

    /*! Deallocate vulkan resources.
    * This is called in the destructor of Device_vulkan, therefor we can not use our `std::weak_ptr<Device_vulkan> device`.
    */
    void destroy(Device *vulkanDevice);

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer);

    static void placeVertices(
        vspan<Vertex> &vertices,
        rect box,
        vec backgroundColor,
        float borderSize,
        vec borderColor,
        vec cornerShapes,
        aarect clippingRectangle
    );

private:
    void buildShaders();
    void teardownShaders(Device_vulkan *vulkanDevice);
};

}
