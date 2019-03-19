//
//  Window.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "BackingPipeline_vulkan.hpp"
#include "View.hpp"

#include <unordered_set>


#include <memory>
#include <mutex>

namespace TTauri { namespace GUI {

class Instance;
class Device;
class View;

/*! A Window.
 * This Window is backed by a native operating system window with a Vulkan surface.
 * The Window should not have any decorations, which are to be drawn by the GUI, because
 * modern design requires drawing of user interface elements in the border.
 */
class Window : public std::enable_shared_from_this<Window> {
public:
    enum class State {
        NO_DEVICE, //!< Can transition to: LINKED_TO_DEVICE
        LINKED_TO_DEVICE, //!< Can transition to: READY_TO_DRAW, MINIMIZED, NO_DEVICE
        SWAPCHAIN_OUT_OF_DATE, //!< Can transition to: READY_TO_DRAW, MINIMIZED, LINKED_TO_DEVICE
        READY_TO_DRAW, //!< Can transition to: SWAPCHAIN_OUT_OF_DATE, LINKED_TO_DEVICE
        MINIMIZED //!< Can transition to: READY_TO_DRAW, LINKED_TO_DEVICE
    };

    class Delegate {
    public:
        virtual void creatingWindow(const std::shared_ptr<Window> &window) = 0;
    };

    struct StateError : virtual boost::exception, virtual std::exception {};
    struct SwapChainError : virtual boost::exception, virtual std::exception {};

    std::recursive_mutex mutex;
    State state;

    std::shared_ptr<Delegate> delegate;

    std::string title;

 
    std::weak_ptr<Device> device;

    //! Location of the window on the screen.
    glm::vec3 position = {0.0, 0.0, 0.0};
    glm::vec3 extent = {0.0, 0.0, 0.0};

    /*! Dots-per-inch of the screen where the window is located.
     * If the window is located on multiple screens then one of the screens is used as
     * the source for the DPI value.
     */
    float dpi = 72.0;

    /*! Pixels-per-Point
     * A point references a typefraphic point, 1/72 inch.
     * Scale all drawing and sizing on the window using this attribute.
     * This value is rounded to an integer value for drawing clean lines.
     */
    float ppp = 1.0;

    //! The view covering the complete window.
    std::shared_ptr<View> view;

    std::shared_ptr<BackingPipeline_vulkan> backingPipeline;

    Window(const std::shared_ptr<Delegate> &delegate, const std::string &title);

    virtual ~Window();

    void initialize();

    /*! Build the swapchain, frame buffers and pipeline.
     */
    virtual void buildForDeviceChange() = 0;

    /*! Teardown the swapchain, frame buffers and pipeline.
     */
    virtual void teardownForDeviceChange() = 0;

    virtual bool rebuildForSwapchainChange() = 0;

    /*! Set GPU device to manage this window.
     * Change of the device may be done at runtime.
     */
    void setDevice(const std::shared_ptr<Device> &device);

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

protected:
    // The extent of the window rectangle should only be read when creating the swapchain.
    vk::Rect2D windowRectangle;

     /*! Render views.
     * \returns false when swapchain is out of date.
     */
    virtual bool render(bool blockOnVSync) = 0;

private:

    
    bool isOnScreen();
};

}}
