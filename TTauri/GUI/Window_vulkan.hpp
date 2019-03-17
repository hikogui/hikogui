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

    std::vector<vk::Image> swapchainImages;
    std::vector<vk::ImageView> swapchainImageViews;
    std::vector<vk::Framebuffer> swapchainFramebuffers;

    vk::RenderPass firstRenderPass;
    vk::RenderPass followUpRenderPass;

    vk::Semaphore imageAvailableSemaphore;
    vk::Fence renderFinishedFence;

    Window_vulkan(std::shared_ptr<Delegate> delegate, const std::string &title, vk::SurfaceKHR surface);
    virtual ~Window_vulkan();

    virtual void buildForDeviceChange();
    virtual void teardownForDeviceChange();
    virtual bool rebuildForSwapchainChange();

protected:
    virtual bool render(bool blockOnVSync);

private:
    void buildSemaphores();
    void teardownSemaphores();
    std::pair<vk::SwapchainKHR, bool> buildSwapchain(vk::SwapchainKHR oldSwapchain = {});
    void teardownSwapchain();
    void buildRenderPasses();
    void teardownRenderPasses();
    void buildFramebuffers();
    void teardownFramebuffers();
    void buildPipelines();
    void teardownPipelines();

    void waitIdle();
    std::pair<uint32_t, vk::Extent2D> getImageCountAndImageExtent();
    bool isOnScreen();
};
}}