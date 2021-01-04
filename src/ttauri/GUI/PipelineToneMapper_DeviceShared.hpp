// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "../required.hpp"
#include "../aarect.hpp"
#include "../numeric_array.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>

namespace tt {
class gui_device_vulkan;
}

namespace tt::PipelineToneMapper {

struct Image;

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
    * This is called in the destructor of gui_device_vulkan, therefor we can not use our `std::weak_ptr<gui_device_vulkan> device`.
    */
    void destroy(gui_device_vulkan *vulkanDevice);

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer);

private:
    void buildShaders();
    void teardownShaders(gui_device_vulkan *vulkanDevice);
};

}
