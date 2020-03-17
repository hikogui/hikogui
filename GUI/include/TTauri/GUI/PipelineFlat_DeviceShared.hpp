// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Device_forward.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/rect.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>

namespace TTauri {
template<typename T> struct PixelMap;
}

namespace TTauri::GUI::PipelineFlat {

struct Image;

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
    void destroy(gsl::not_null<Device *> vulkanDevice);

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer);

    static void placeVerticesBox(vspan<Vertex> &vertices, rect box, R16G16B16A16SFloat color, rect clippingRectangle, float depth) noexcept;

private:
    void buildShaders();
    void teardownShaders(gsl::not_null<Device_vulkan *> vulkanDevice);
};

}
