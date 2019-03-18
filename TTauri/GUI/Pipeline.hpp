//
//  Pipeline.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include <boost/filesystem.hpp>

#include <string>
#include <vector>

namespace TTauri { namespace GUI {

class Device;
class Window;

class Pipeline {
public:
    struct Error : virtual boost::exception, virtual std::exception {};
    struct NonVulkanWindowError : virtual Error {};

    vk::Pipeline intrinsic;

    std::weak_ptr<Window> window;

    Pipeline(const std::shared_ptr<Window> &window);
    virtual ~Pipeline();

    template <typename T>
    std::shared_ptr<T> device() const {
        return lock_dynamic_cast<T>(window.lock()->device);
    }

    /*! Render
     */
    virtual vk::Semaphore render(uint32_t imageIndex, vk::Semaphore inputSemaphore);

    /*! Invalidate all command buffers.
     * This is used when the command buffer needs to be recreated due to changes in views.
     */
    void invalidateCommandBuffers();

    /*! Validate/create a command buffer.
     *
     * \param imageIndex The index of the command buffer to validate.
     */
    void validateCommandBuffer(uint32_t imageIndex);

    void buildForDeviceChange(vk::RenderPass renderPass, vk::Extent2D extent, size_t nrFrameBuffers);

    void teardownForDeviceChange();

    void buildForSwapchainChange(vk::RenderPass renderPass, vk::Extent2D extent, size_t nrFrameBuffers);

    void teardownForSwapchainChange();

protected:
    std::vector<vk::CommandBuffer> commandBuffers;
    std::vector<bool> commandBuffersValid;
    std::vector<vk::Semaphore> renderFinishedSemaphores;

    std::vector<vk::Buffer> vertexBuffers;
    vk::DeviceMemory vertexBufferMemory;
    std::vector<size_t> vertexBufferOffsets;
    std::vector<size_t> vertexBufferSizes;
    size_t vertexBufferDataSize = 0;
    void *vertexBufferData = nullptr;

    boost::filesystem::path vertexShaderPath;
    boost::filesystem::path fragmentShaderPath;

    vk::RenderPass renderPass;
    std::vector<vk::ShaderModule> shaderModules;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    vk::PipelineLayout pipelineLayout;
    vk::VertexInputBindingDescription vertexInputBindingDescription;
    std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
    vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo;
    vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo;
    std::vector<vk::Viewport> viewports;
    std::vector<vk::Rect2D> scissors;
    vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo;
    vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo;
    vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo;
    std::vector<vk::PipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStates;
    vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo;
    vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo;

    virtual void drawInCommandBuffer(vk::CommandBuffer &commandBuffer) = 0;
    virtual vk::ShaderModule loadShader(boost::filesystem::path path) const;
    virtual std::vector<vk::ShaderModule> createShaderModules() const = 0;
    virtual std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages(const std::vector<vk::ShaderModule> &shaders) const = 0;
    virtual std::vector<vk::PushConstantRange> createPushConstantRanges() const = 0;
    virtual vk::PipelineLayout createPipelineLayout() const;
    virtual vk::PipelineVertexInputStateCreateInfo createPipelineVertexInputStateCreateInfo(
        const vk::VertexInputBindingDescription &vertexBindingDescriptions, const std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) const;
    virtual vk::VertexInputBindingDescription createVertexInputBindingDescription() const = 0;
    virtual std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const = 0;
    virtual vk::PipelineInputAssemblyStateCreateInfo createPipelineInputAssemblyStateCreateInfo() const;
    virtual std::vector<vk::Viewport> createViewports(vk::Extent2D extent) const;
    virtual std::vector<vk::Rect2D> createScissors(vk::Extent2D extent) const;
    virtual vk::PipelineViewportStateCreateInfo createPipelineViewportStateCreateInfo(const std::vector<vk::Viewport> &viewports, std::vector<vk::Rect2D> &scissors) const;
    virtual vk::PipelineRasterizationStateCreateInfo createPipelineRasterizationStateCreateInfo() const;
    virtual vk::PipelineMultisampleStateCreateInfo createPipelineMultisampleStateCreateInfo() const;
    virtual std::vector<vk::PipelineColorBlendAttachmentState> createPipelineColorBlendAttachmentStates() const;
    virtual vk::PipelineColorBlendStateCreateInfo createPipelineColorBlendStateCreateInfo(const std::vector<vk::PipelineColorBlendAttachmentState> &attachements) const;
    virtual size_t maximumNumberOfVertices() const = 0;
    virtual std::vector<vk::Buffer> createVertexBuffers(size_t nrBuffers, size_t bufferSize) const;

    virtual void buildShaders();
    virtual void teardownShaders();
    virtual void buildVertexBuffers(size_t nrFrameBuffers);
    virtual void teardownVertexBuffers();
    virtual void buildCommandBuffers(size_t nrFrameBuffers);
    virtual void teardownCommandBuffers();
    virtual void buildSemaphores(size_t nrFrameBuffers);
    virtual void teardownSemaphores();
    virtual void buildPipeline(vk::RenderPass renderPass, vk::Extent2D extent);
    virtual void teardownPipeline();
};

}}
