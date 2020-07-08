// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "GUIDevice_forward.hpp"
#include "../required.hpp"
#include "../aarect.hpp"
#include "../vec.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>

namespace tt {
template<typename T> struct PixelMap;
}

namespace tt::PipelineToneMapper {

struct Image;

struct DeviceShared final {
    GUIDevice const &device;

    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    DeviceShared(GUIDevice const &device);
    ~DeviceShared();

    DeviceShared(DeviceShared const &) = delete;
    DeviceShared &operator=(DeviceShared const &) = delete;
    DeviceShared(DeviceShared &&) = delete;
    DeviceShared &operator=(DeviceShared &&) = delete;

    /*! Deallocate vulkan resources.
    * This is called in the destructor of GUIDevice_vulkan, therefor we can not use our `std::weak_ptr<GUIDevice_vulkan> device`.
    */
    void destroy(GUIDevice *vulkanDevice);

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer);

private:
    void buildShaders();
    void teardownShaders(GUIDevice_vulkan *vulkanDevice);
};

}
