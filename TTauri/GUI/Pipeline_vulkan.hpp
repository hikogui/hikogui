
#pragma once

#include "Pipeline.hpp"

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include <boost/filesystem.hpp>

#include <string>
#include <vector>

namespace TTauri::GUI {

class Pipeline_vulkan : public Pipeline {
public:
    struct NonVulkanWindowError : virtual Error {};

    vk::Pipeline intrinsic;

    Pipeline_vulkan(const std::shared_ptr<Window> window);
    ~Pipeline_vulkan();

    Pipeline_vulkan(const Pipeline_vulkan &) = delete;
    Pipeline_vulkan &operator=(const Pipeline_vulkan &) = delete;
    Pipeline_vulkan(Pipeline_vulkan &&) = delete;
    Pipeline_vulkan &operator=(Pipeline_vulkan &&) = delete;

    /*! Render
     */
    virtual vk::Semaphore render(uint32_t imageIndex, vk::Semaphore inputSemaphore);

    /*! Invalidate all command buffers.
     * This is used when the command buffer needs to be recreated due to changes in views.
     *
     * \param reset Also reset the command buffer to release resources associated with them.
     */
    void invalidateCommandBuffers(bool reset);

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

    boost::filesystem::path vertexShaderPath;
    boost::filesystem::path fragmentShaderPath;

    vk::RenderPass renderPass;
    vk::Extent2D extent;
    vk::Rect2D scissor;
    std::vector<vk::ShaderModule> shaderModules;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    vk::PipelineLayout pipelineLayout;

    virtual void drawInCommandBuffer(vk::CommandBuffer &commandBuffer, uint32_t imageIndex) = 0;
    virtual vk::ShaderModule loadShader(boost::filesystem::path path) const;
    virtual std::vector<vk::ShaderModule> createShaderModules() const = 0;
    virtual std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages(const std::vector<vk::ShaderModule> &shaders) const = 0;
    virtual std::vector<vk::PushConstantRange> createPushConstantRanges() const = 0;
    virtual vk::VertexInputBindingDescription createVertexInputBindingDescription() const = 0;
    virtual std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const = 0;


    virtual void buildShaders();
    virtual void teardownShaders();
    virtual void buildVertexBuffers(size_t nrFrameBuffers) = 0;
    virtual void teardownVertexBuffers() = 0;
    virtual void buildCommandBuffers(size_t nrFrameBuffers);
    virtual void teardownCommandBuffers();
    virtual void buildSemaphores(size_t nrFrameBuffers);
    virtual void teardownSemaphores();
    virtual void buildPipeline(vk::RenderPass renderPass, vk::Extent2D extent);
    virtual void teardownPipeline();
};

}