// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Window_base.hpp"
#include <vulkan/vulkan.hpp>
#include <optional>

namespace TTauri::GUI {

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

    vk::RenderPass firstRenderPass;
    vk::RenderPass followUpRenderPass;
    vk::RenderPass lastRenderPass;

    vk::Semaphore imageAvailableSemaphore;
    vk::Fence renderFinishedFence;

    std::unique_ptr<PipelineImage::PipelineImage> imagePipeline;
    std::unique_ptr<PipelineFlat::PipelineFlat> flatPipeline;
    std::unique_ptr<PipelineBox::PipelineBox> boxPipeline;
    std::unique_ptr<PipelineSDF::PipelineSDF> SDFPipeline;

    Window_vulkan(const std::shared_ptr<WindowDelegate> delegate, const std::string title);
    ~Window_vulkan();

    Window_vulkan(const Window_vulkan &) = delete;
    Window_vulkan &operator=(const Window_vulkan &) = delete;
    Window_vulkan(Window_vulkan &&) = delete;
    Window_vulkan &operator=(Window_vulkan &&) = delete;

    void initialize() override;
    void render() override;

protected:
    void teardown() override;
    void build() override;

    /*! Query the surface from the operating-system window.
     */
    virtual vk::SurfaceKHR getSurface() const = 0;
    
private:
    std::optional<uint32_t> acquireNextImageFromSwapchain();
    void presentImageToQueue(uint32_t frameBufferIndex, vk::Semaphore renderFinishedSemaphore);

    bool readSurfaceExtent();
    bool checkSurfaceExtent();

    void buildDevice();
    void buildSemaphores();
    void teardownSemaphores();
    State buildSwapchain();
    void teardownSwapchain();
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
