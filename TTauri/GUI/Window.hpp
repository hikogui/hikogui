// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "globals.hpp"
#include "WindowWidget.hpp"
#include "TTauri/all.hpp"
#include <rhea/simplex_solver.hpp>
#include <unordered_set>
#include <memory>
#include <mutex>

namespace TTauri::GUI {

class Instance;
class Device;

/*! A Window.
 * This Window is backed by a native operating system window with a Vulkan surface.
 * The Window should not have any decorations, which are to be drawn by the GUI, because
 * modern design requires drawing of user interface elements in the border.
 */
class Window : public std::enable_shared_from_this<Window> {
public:
    enum class State {
        NO_DEVICE, //!< state. No device is associated with the Window and can therefor not be rendered on.
        MINIMIZED, //!< state. The window is minimized, the current swapchain is still out-of-date and can not be rendered on.
        SWAPCHAIN_OUT_OF_DATE, //!< state. The window was resized, the swapchain needs to be rebuild and can not be rendered on.
        READY_TO_DRAW, //!< state. The swapchain is ready drawing is allowed.
        SURFACE_LOST, //!< state. The window was destroyed, everything needs to be destroyed.
        DEVICE_LOST, //!< state. The device was last, but the window could move to a new device, or the device can be recreated.
    };

    enum class SizeState {
        MINIMIZED,
        NORMAL,
        MAXIMIZED,
    };

    class Delegate {
    public:
        Delegate() = default;
        virtual ~Delegate() = default;
        Delegate(const Delegate &) = delete;
        Delegate &operator=(const Delegate &) = delete;
        Delegate(Delegate &&) = delete;
        Delegate &operator=(Delegate &&) = delete;

        virtual void openingWindow(const std::shared_ptr<Window> &window) = 0;
        virtual void closingWindow(const std::shared_ptr<Window> &window) = 0;
    };

    struct SwapChainError : virtual boost::exception, virtual std::exception {};

    State state = State::NO_DEVICE;

    std::shared_ptr<Delegate> delegate;

    std::string title;

    std::weak_ptr<Device> device;

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

    //! The widget covering the complete window.
    std::shared_ptr<WindowWidget> widget;

    //! This solver determines size and position of all widgets in this window.
    rhea::constraint widthConstraint;
    rhea::constraint heightConstraint;
    bool widthHeightContraintsAdded = false;
    rhea::simplex_solver widgetSolver;

    Window(const std::shared_ptr<Delegate> delegate, const std::string title);
    virtual ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;
    Window(Window &&) = delete;
    Window &operator=(Window &&) = delete;

    virtual void initialize();

    bool hasLostSurface() { return state == State::SURFACE_LOST; }
    bool hasLostDevice() { return state == State::DEVICE_LOST; }
    virtual void openingWindow() { delegate->openingWindow(shared_from_this()); }
    virtual void closingWindow() { delegate->closingWindow(shared_from_this()); }

    /*! Build the swapchain, frame buffers and pipeline.
     */
    virtual State buildForDeviceChange() = 0;

    /*! Teardown the swapchain, frame buffers and pipeline.
     */
    virtual void teardownForDeviceChange() = 0;

    virtual State rebuildForSwapchainChange() = 0;

    /*! Set GPU device to manage this window.
     * Change of the device may be done at runtime.
     */
    void setDevice(const std::weak_ptr<Device> device);

    /*! Remove the GPU device from the window, making it an orphan.
     */
    void unsetDevice() { setDevice({}); }

    /*! Update window.
     * This will update animations and redraw all widgets managed by this window.
     * This may be called on a low latency thread, it is careful to not block on operations.
     *
     * blockOnVSync should only be called on the first window in the system. This allows a
     * non-vsync thread to call this method with minimal CPU usage.
     *
     * \param outTimestamp Number of nanoseconds since system start.
     * \param outputTimestamp Number of nanoseconds since system start until the frame will be displayed on the screen.
     */
    void updateAndRender(uint64_t nowTimestamp, uint64_t outputTimestamp);

    /*! Maintanance
     * Maintain the window on a low performance thread.
     *
     * For example: rebuilding the swapchain on window size changes.
     */
    void maintenance();

    BoxModel &box() {
        return widget->box;
    }

    rhea::solver& addConstraint(rhea::constraint const& constraint) {
        return widgetSolver.add_constraint(constraint);
    }

    rhea::solver& removeConstraint(rhea::constraint const& constraint) {
        return widgetSolver.remove_constraint(constraint);
    }



protected:
    u64rect2 windowRectangle;

    virtual void setWindowPosition(uint32_t x, uint32_t y);
    virtual void setWindowSize(uint32_t width, uint32_t height);

    /*! Render widgets.
     * \param should this window block on vertical-sync.
     * \returns true if this function has blocked on vertical-sync.
     */
    virtual void render() = 0;

private:
    bool isOnScreen();
};

}
