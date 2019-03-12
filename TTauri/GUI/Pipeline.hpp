//
//  Pipeline.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include <vector>

namespace TTauri {
namespace GUI {

class Device;
class Window;

class Pipeline {
public:
    vk::Pipeline intrinsic;

    Window *window;

 
    Pipeline(Window *window);
    virtual ~Pipeline();

    Device *device() const;

    /*! Render
     */
    virtual vk::Semaphore render(uint32_t imageIndex, vk::Semaphore inputSemaphore);

    /*! Build the swapchain, frame buffers and pipeline.
     */
    void buildPipeline(vk::RenderPass renderPass, vk::Extent2D extent, size_t maximumNumberOfTriangles);

    /*! Teardown the swapchain, frame buffers and pipeline.
     */
    void teardownPipeline();

    /*! Invalidate all command buffers.
     * This is used when the command buffer needs to be recreated due to changes in views.
     */
    void invalidateCommandBuffers();

    /*! Validate/create a command buffer.
     *
     * \param imageIndex The index of the command buffer to validate.
     */
    void validateCommandBuffer(uint32_t imageIndex);

protected:
    std::vector<vk::CommandBuffer> commandBuffers;
    std::vector<bool> commandBuffersValid;
    std::vector<vk::Semaphore> renderFinishedSemaphores;

    std::vector<vk::Buffer> vertexBuffers;
    vk::DeviceMemory vertexBufferMemory;
    std::vector<size_t> vertexBufferOffsets;
    std::vector<size_t> vertexBufferSizes;
    size_t vertexBufferDataSize;
    void *vertexBufferData;

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
    size_t maximumNumberOfTriangles;
    size_t maximumNumberOfVertices;
    

    virtual void drawInCommandBuffer(vk::CommandBuffer &commandBuffer) = 0;
    virtual vk::ShaderModule loadShader(boost::filesystem::path path) const;
    virtual std::vector<vk::ShaderModule> createShaderModules() const = 0;
    virtual std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages(const std::vector<vk::ShaderModule> &shaders) const = 0;
    virtual std::vector<vk::PushConstantRange> createPushConstantRanges() const = 0;
    virtual vk::PipelineLayout createPipelineLayout() const;
    virtual vk::PipelineVertexInputStateCreateInfo createPipelineVertexInputStateCreateInfo(const vk::VertexInputBindingDescription &vertexBindingDescriptions, const std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) const;
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
    virtual std::vector<vk::Buffer> createVertexBuffers(size_t nrBuffers, size_t bufferSize) const;

};

}}
