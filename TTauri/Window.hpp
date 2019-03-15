//
//  Window.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "BackingPipeline.hpp"
#include "View.hpp"

#include <unordered_set>
#include <vulkan/vulkan.hpp>

#include <memory>
#include <mutex>

namespace TTauri {

class Instance;
class Device;



class View;

/*! A Window.
 * This Window is backed by a native operating system window with a Vulkan surface.
 * The Window should not have any decorations, which are to be drawn by the GUI, because
 * modern design requires drawing of user interface elements in the border.
 */
class Window {
private:
    enum class State {
        NO_DEVICE, //!< Can transition to: LINKED_TO_DEVICE
        LINKED_TO_DEVICE, //!< Can transition to: READY_TO_DRAW, MINIMIZED, NO_DEVICE
        SWAPCHAIN_OUT_OF_DATE, //!< Can transition to: READY_TO_DRAW, MINIMIZED, LINKED_TO_DEVICE
        READY_TO_DRAW, //!< Can transition to: SWAPCHAIN_OUT_OF_DATE, LINKED_TO_DEVICE
        MINIMIZED //!< Can transition to: READY_TO_DRAW, LINKED_TO_DEVICE
    };

    std::recursive_mutex stateMutex;
    State state;

public:
    class Delegate {
    public:
        virtual void initialize(Window *window) = 0;
    };

    struct StateError : virtual boost::exception, virtual std::exception {};
    struct SwapChainError : virtual boost::exception, virtual std::exception {};

    vk::SurfaceKHR intrinsic;
    std::shared_ptr<Delegate> delegate;

    std::string title;

    vk::SwapchainCreateInfoKHR swapchainCreateInfo;

    vk::SwapchainKHR swapchain;

    std::vector<vk::Image> swapchainImages;
    std::vector<vk::ImageView> swapchainImageViews;
    std::vector<vk::Framebuffer> swapchainFramebuffers;

    vk::RenderPass firstRenderPass;
    vk::RenderPass followUpRenderPass;

    vk::Semaphore imageAvailableSemaphore;
    vk::Fence renderFinishedFence;

    Instance *instance;
    Device *device;

    //! Location of the window on the screen.
    glm::vec3 position;
    glm::vec3 extent;

    /*! Dots-per-inch of the screen where the window is located.
     * If the window is located on multiple screens then one of the screens is used as
     * the source for the DPI value.
     */
    float dpi;

    /*! Pixels-per-Point
     * A point references a typefraphic point, 1/72 inch.
     * Scale all drawing and sizing on the window using this attribute.
     * This value is rounded to an integer value for drawing clean lines.
     */
    float ppp;

    //! The view covering the complete window.
    std::shared_ptr<View> view;

    std::shared_ptr<BackingPipeline> backingPipeline;

    Window(Instance *instance, std::shared_ptr<Delegate> delegate, const std::string &title, vk::SurfaceKHR surface);

    ~Window();

    void initialize();

    /*! Build the swapchain, frame buffers and pipeline.
     */
    void buildForDeviceChange();

    /*! Teardown the swapchain, frame buffers and pipeline.
     */
    void teardownForDeviceChange();

    /*! Set GPU device to manage this window.
     * Change of the device may be done at runtime.
     */
    void setDevice(Device *device);

    /*! Update window.
     * This will update animations and redraw all views managed by this window.
     * This may be called on a low latency thread, it is careful to not block on operations.
     *
     * blockOnVSync should only be called on the first window in the system. This allows a
     * non-vsync thread to call this method with minimal CPU usage.
     *
     * \param outTimestamp Number of nanoseconds since system start.
     * \param outputTimestamp Number of nanoseconds since system start until the frame will be displayed on the screen.
     * \param blockOnVSync May block on VSync.
     */
    void updateAndRender(uint64_t nowTimestamp, uint64_t outputTimestamp, bool blockOnVSync);

    /*! Maintanance
     * Maintain the window on a low performance thread.
     *
     * For example: rebuilding the swapchain on window size changes.
     */
    void maintenance();

    void setWindowRectangle(vk::Rect2D rect) { windowRectangle = rect; }

private:
    // The extent of the window rectangle should only be read when creating the swapchain.
    vk::Rect2D windowRectangle;

    /*! Render views.
     * \returns false when swapchain is out of date.
     */
    bool render(bool blockOnVSync);
    void buildSemaphores();
    void teardownSemaphores();
    std::pair<vk::SwapchainKHR, bool> buildSwapchain(vk::SwapchainKHR oldSwapchain = {});
    void buildRenderPasses();
    void teardownRenderPasses();
    void buildFramebuffers();
    void teardownFramebuffers();
    void buildPipelines();
    void teardownPipelines();
    bool rebuildForSwapchainChange();
    void waitIdle();
    std::pair<uint32_t, vk::Extent2D> getImageCountAndImageExtent();
    bool isOnScreen();
};

}
