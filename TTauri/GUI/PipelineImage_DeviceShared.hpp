
#pragma once

#include "PipelineImage.hpp"
#include "Device_vulkan.hpp"

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace TTauri::GUI {

struct PipelineImage::DeviceShared final {
    std::weak_ptr<Device_vulkan> device;

    vk::Buffer indexBuffer;
    VmaAllocation indexBufferAllocation = {};

    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    DeviceShared(const std::shared_ptr<Device_vulkan> device);
    ~DeviceShared();

    DeviceShared(const DeviceShared &) = delete;
    DeviceShared &operator=(const DeviceShared &) = delete;
    DeviceShared(DeviceShared &&) = delete;
    DeviceShared &operator=(DeviceShared &&) = delete;

    /*! Deallocate vulkan resources.
    * This is called in the destructor of Device_vulkan, therefor we can not use our `std::weak_ptr<Device_vulkan> device`.
    */
    void destroy(gsl::not_null<Device_vulkan *> vulkanDevice);

    std::shared_ptr<PipelineImage::Image> retainImage(const std::string &name, u16vec2 extent);
    void releaseImage(std::shared_ptr<PipelineImage::Image> image);

private:
    void buildIndexBuffer();
    void teardownIndexBuffer(gsl::not_null<Device_vulkan *> vulkanDevice);
    void buildShaders();
    void teardownShaders(gsl::not_null<Device_vulkan *> vulkanDevice);
};

}