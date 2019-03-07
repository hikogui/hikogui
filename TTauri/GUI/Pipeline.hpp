//
//  Pipeline.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <vulkan/vulkan.hpp>
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

    std::vector<vk::CommandBuffer> commandBuffers;
    std::vector<bool> commandBuffersValid;
    std::vector<vk::Semaphore> renderFinishedSemaphores;

    boost::filesystem::path vertexShaderPath;
    boost::filesystem::path fragmentShaderPath;

    Pipeline(Window *window);
    virtual ~Pipeline();

    /*! Build the swapchain, frame buffers and pipeline.
     */
    void buildPipeline(vk::RenderPass renderPass, vk::Extent2D extent);

    /*! Teardown the swapchain, frame buffers and pipeline.
     */
    void teardownPipeline(void);

    /*! Invalidate all command buffers.
     * This is used when the command buffer needs to be recreated due to changes in views.
     */
    void invalidateCommandBuffers();

    /*! Validate/create a command buffer.
     *
     * \param imageIndex The index of the command buffer to validate.
     */
    void validateCommandBuffer(uint32_t imageIndex);

    vk::Semaphore render(uint32_t imageIndex, vk::Semaphore inputSemaphore);

protected:
    vk::RenderPass renderPass;
    
    std::vector<vk::ShaderModule> shaderModules;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    vk::PipelineLayout pipelineLayout;
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

    virtual vk::ShaderModule loadShader(boost::filesystem::path path) const;
    virtual std::vector<vk::ShaderModule> createShaderModules(void) const = 0;
    virtual std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages(const std::vector<vk::ShaderModule> &shaders) const = 0;
    virtual vk::PipelineLayout createPipelineLayout(void) const;
    virtual vk::PipelineVertexInputStateCreateInfo createPipelineVertexInputStateCreateInfo(void) const;
    virtual vk::PipelineInputAssemblyStateCreateInfo createPipelineInputAssemblyStateCreateInfo(void) const;
    virtual std::vector<vk::Viewport> createViewports(vk::Extent2D extent) const;
    virtual std::vector<vk::Rect2D> createScissors(vk::Extent2D extent) const;
    virtual vk::PipelineViewportStateCreateInfo createPipelineViewportStateCreateInfo(const std::vector<vk::Viewport> &viewports, std::vector<vk::Rect2D> &scissors) const;
    virtual vk::PipelineRasterizationStateCreateInfo createPipelineRasterizationStateCreateInfo(void) const;
    virtual vk::PipelineMultisampleStateCreateInfo createPipelineMultisampleStateCreateInfo(void) const;
    virtual std::vector<vk::PipelineColorBlendAttachmentState> createPipelineColorBlendAttachmentStates(void) const;
    virtual vk::PipelineColorBlendStateCreateInfo createPipelineColorBlendStateCreateInfo(const std::vector<vk::PipelineColorBlendAttachmentState> &attachements) const;
};

}}
