// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Window_base.hpp"
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>
#include <optional>

namespace TTauri {

namespace PipelineImage {
class PipelineImage;
}
namespace PipelineFlat {
class PipelineFlat;
}
namespace PipelineBox {
class PipelineBox;
}
namespace PipelineSDF {
class PipelineSDF;
}
namespace PipelineToneMapper {
class PipelineToneMapper;
}

class Window_vulkan : public Window_base {
public:
    vk::SurfaceKHR intrinsic;

    vk::SwapchainKHR swapchain;

    int nrSwapchainImages;
    vk::Extent2D swapchainImageExtent;
    vk::SurfaceFormatKHR swapchainImageFormat;
    std::vector<vk::Image> swapchainImages;
    std::vector<vk::ImageView> swapchainImageViews;
    std::vector<vk::Framebuffer> swapchainFramebuffers;

    static const vk::Format depthImageFormat = vk::Format::eD32Sfloat;
    VmaAllocation depthImageAllocation;
    vk::Image depthImage;
    vk::ImageView depthImageView;

    static const vk::Format colorImageFormat = vk::Format::eR16G16B16A16Sfloat;
    VmaAllocation colorImageAllocation;
    vk::Image colorImage;
    vk::ImageView colorImageView;
    vk::DescriptorImageInfo colorDescriptorImageInfo;

    vk::RenderPass renderPass;

    vk::CommandBuffer commandBuffer;

    vk::Semaphore imageAvailableSemaphore;
    vk::Semaphore renderFinishedSemaphore;
    vk::Fence renderFinishedFence;

    std::unique_ptr<PipelineImage::PipelineImage> imagePipeline;
    std::unique_ptr<PipelineFlat::PipelineFlat> flatPipeline;
    std::unique_ptr<PipelineBox::PipelineBox> boxPipeline;
    std::unique_ptr<PipelineSDF::PipelineSDF> SDFPipeline;
    std::unique_ptr<PipelineToneMapper::PipelineToneMapper> toneMapperPipeline;

    Window_vulkan(const std::shared_ptr<WindowDelegate> delegate, const std::string title);
    ~Window_vulkan();

    Window_vulkan(const Window_vulkan &) = delete;
    Window_vulkan &operator=(const Window_vulkan &) = delete;
    Window_vulkan(Window_vulkan &&) = delete;
    Window_vulkan &operator=(Window_vulkan &&) = delete;

    void initialize() override;
    void render(hires_utc_clock::time_point displayTimePoint) override;

protected:
    void teardown() override;
    void build() override;

    /*! Query the surface from the operating-system window.
     */
    virtual vk::SurfaceKHR getSurface() const = 0;
    
private:
    std::optional<uint32_t> acquireNextImageFromSwapchain();
    void presentImageToQueue(uint32_t frameBufferIndex, vk::Semaphore renderFinishedSemaphore);

    void fillCommandBuffer(vk::Framebuffer frameBuffer);
    void submitCommandBuffer();

    bool readSurfaceExtent();
    bool checkSurfaceExtent();

    void buildDevice();
    void buildSemaphores();
    void teardownSemaphores();
    State buildSwapchain();
    void teardownSwapchain();
    void buildCommandBuffers();
    void teardownCommandBuffers();
    void buildRenderPasses();
    void teardownRenderPasses();
    void buildFramebuffers();
    void teardownFramebuffers();
    void buildPipelines();
    void teardownPipelines();
    bool buildSurface();
    void teardownSurface();
    void teardownDevice();

    void waitIdle();
    std::tuple<uint32_t, vk::Extent2D> getImageCountAndExtent();
};

}
