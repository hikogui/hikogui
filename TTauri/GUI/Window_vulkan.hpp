#pragma once

#include "Window.hpp"

#include <vulkan/vulkan.hpp>

namespace TTauri {
namespace GUI {
class Window_vulkan : public Window {
public:
    vk::SurfaceKHR intrinsic;

    vk::SwapchainCreateInfoKHR swapchainCreateInfo;

    vk::SwapchainKHR swapchain;

    std::optional<uint32_t> acquiredImageIndex;
    std::vector<vk::Image> swapchainImages;
    std::vector<vk::ImageView> swapchainImageViews;
    std::vector<vk::Framebuffer> swapchainFramebuffers;

    vk::RenderPass firstRenderPass;
    vk::RenderPass followUpRenderPass;

    vk::Semaphore imageAvailableSemaphore;
    vk::Fence renderFinishedFence;

    std::shared_ptr<PipelineRectanglesFromAtlas> pipelineRectanglesFromAtlas;

    Window_vulkan(std::shared_ptr<Delegate> delegate, const std::string &title, vk::SurfaceKHR surface);
    ~Window_vulkan();

    Window_vulkan(const Window_vulkan &) = delete;
    Window_vulkan &operator=(const Window_vulkan &) = delete;
    Window_vulkan(Window_vulkan &&) = delete;
    Window_vulkan &operator=(Window_vulkan &&) = delete;

    void initialize() override;

    State buildForDeviceChange() override;
    void teardownForDeviceChange() override;
    State rebuildForSwapchainChange() override;

protected:
    bool render(bool blockOnVSync) override;

private:
    void buildSemaphores();
    void teardownSemaphores();
    std::pair<vk::SwapchainKHR, Window::State> buildSwapchain(vk::SwapchainKHR oldSwapchain = {});
    void teardownSwapchain();
    void buildRenderPasses();
    void teardownRenderPasses();
    void buildFramebuffers();
    void teardownFramebuffers();
    void buildPipelines();
    void teardownPipelines();

    void waitIdle();
    std::tuple<uint32_t, vk::Extent2D, State> getImageCountExtentAndState();
};

}}