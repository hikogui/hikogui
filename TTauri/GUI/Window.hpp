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
#include <vulkan/vulkan.hpp>
#include <memory>
#include <mutex>
#include <unordered_set>

namespace TTauri {
namespace GUI {

class Instance;
class Device;


enum class WindowType {
    WINDOW,
    PANEL,
    FULLSCREEN,
};

enum class SubpixelLayout {
    NONE,
    RGB_LEFT_TO_RIGHT,
    RGB_RIGHT_TO_LEFT,
    RGB_TOP_TO_BOTTOM,
    RGB_BOTTOM_TO_TOP,
};

enum class WindowState {
    NO_DEVICE,
    LINKED_TO_DEVICE,
    SWAPCHAIN_OUT_OF_DATE,
    READY_TO_DRAW,
};

class View;

/*! A Window.
 * This Window is backed by a native operating system window with a Vulkan surface.
 * The Window should not have any decorations, which are to be drawn by the GUI, because
 * modern design requires drawing of user interface elements in the border.
 */
class Window {
private:
    std::recursive_mutex stateMutex;
    WindowState state;

public:
    struct StateError : virtual boost::exception, virtual std::exception {};
    struct SwapChainError : virtual boost::exception, virtual std::exception {};

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

    /*! Definition on how subpixels are oriented on the window.
     * If the window is located on multiple screens with different pixel layout then
     * `SubpixelLayout::NONE` should be selected.
     */
    SubpixelLayout subpixelLayout;

    //! The view covering the complete window.
    std::shared_ptr<View> view;

    /*! Type of window.
     * The type of window dictates the way the window-decoration and possibly the
     * rest of the user interface is drawn. This may switch during execution
     * for example switching to `FULLSCREEN` and back to `WINDOW`.
     */
    WindowType windowType;

    std::shared_ptr<BackingPipeline> backingPipeline;

    Window(Instance *instance, vk::SurfaceKHR surface);

    ~Window();

    /*! Build the swapchain, frame buffers and pipeline.
     */
    void buildSwapchainAndPipeline();

    /*! Teardown the swapchain, frame buffers and pipeline.
     */
    void teardownSwapchainAndPipeline();

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

    /*! Wait until everything has been drawn.
     */
    void waitIdle();

    /*! Maintanance
     * Maintain the window on a low performance thread.
     *
     * For example: rebuilding the swapchain on window size changes.
     */
    void maintenance();

    void setWindowRectangle(vk::Rect2D rect) {
        windowRectangle = rect;
    }

private:
    // The extent of the window rectangle should only be read when creating the swapchain.
    vk::Rect2D windowRectangle;

    /*! Render views.
     * \returns false when swapchain is out of date.
     */
    bool render(bool blockOnVSync);
    void buildSemaphores();
    void teardownSemaphores();
    void buildSwapchain();
    void teardownSwapchain();
    void buildRenderPasses();
    void teardownRenderPasses();
    void buildFramebuffers();
    void teardownFramebuffers();
    void buildPipelines();
    void teardownPipelines();
};

}}
