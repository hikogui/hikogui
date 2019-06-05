// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Window_base.hpp"
#include "PipelineImage.hpp"
#include <vulkan/vulkan.hpp>

namespace TTauri::GUI {

class Window_vulkan : public Window_base {
public:
    vk::SurfaceKHR intrinsic;

    vk::SwapchainKHR swapchain;

    uint32_t nrSwapchainImages;
    vk::Extent2D swapchainImageExtent;
    vk::SurfaceFormatKHR swapchainImageFormat;
    std::vector<vk::Image> swapchainImages;
    std::vector<vk::ImageView> swapchainImageViews;
    std::vector<vk::Framebuffer> swapchainFramebuffers;

    vk::RenderPass firstRenderPass;
    vk::RenderPass followUpRenderPass;

    vk::Semaphore imageAvailableSemaphore;
    vk::Fence renderFinishedFence;

    std::shared_ptr<PipelineImage::PipelineImage> imagePipeline;

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
    virtual vk::SurfaceKHR getSurface() = 0;
    
private:
    std::optional<uint32_t> acquireNextImageFromSwapchain();
    void presentImageToQueue(uint32_t imageIndex, vk::Semaphore renderFinishedSemaphore);

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
