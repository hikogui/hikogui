// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "globals.hpp"
#include "WindowDelegate.hpp"
#include "WindowWidget.hpp"
#include "Device_forward.hpp"
#include "TTauri/all.hpp"
#include <rhea/simplex_solver.hpp>
#include <unordered_set>
#include <memory>
#include <mutex>

namespace TTauri::GUI {

/*! A Window.
 * This Window is backed by a native operating system window with a Vulkan surface.
 * The Window should not have any decorations, which are to be drawn by the GUI, because
 * modern design requires drawing of user interface elements in the border.
 */
class Window_base : public std::enable_shared_from_this<Window_base> {
public:
    enum class State {
        NO_WINDOW, //!< The window was destroyed.
        NO_DEVICE, //!< No device is associated with the Window and can therefor not be rendered on.
        NO_SURFACE, //!< Need to request a new surface before building a swapchain
        NO_SWAPCHAIN, //! Need to request a swapchain before rendering.
        READY_TO_RENDER, //!< The swapchain is ready drawing is allowed.
        SWAPCHAIN_LOST, //!< The window was resized, the swapchain needs to be rebuild and can not be rendered on.
        SURFACE_LOST, //!< The Vulkan surface on the window was destroyed.
        DEVICE_LOST, //!< The device was lost, but the window could move to a new device, or the device can be recreated.
        WINDOW_LOST, //!< The window was destroyed, need to cleanup.
    };

    enum class SizeState {
        MINIMIZED,
        NORMAL,
        MAXIMIZED,
    };

    struct SwapChainError : virtual boost::exception, virtual std::exception {};

    State state = State::NO_DEVICE;

    std::shared_ptr<WindowDelegate> delegate;

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

    Window_base(const std::shared_ptr<WindowDelegate> delegate, const std::string title);
    virtual ~Window_base();

    Window_base(const Window_base &) = delete;
    Window_base &operator=(const Window_base &) = delete;
    Window_base(Window_base &&) = delete;
    Window_base &operator=(Window_base &&) = delete;

    virtual void initialize();

    //bool hasLostSurface() { return state == State::SURFACE_LOST; }
    //bool hasLostDevice() { return state == State::DEVICE_LOST; }
    virtual void openingWindow();
    virtual void closingWindow();


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

    virtual void windowChangedSize(u64extent2 extent);

    virtual void render() = 0;

    /*! Teardown Window based on State::*_LOST
     */
    virtual void teardown() = 0;

    /*! Build Windows based on State::NO_*
     */
    virtual void build() = 0;

    void rebuild() {
        teardown();
        build();
    }

private:
    bool isOnScreen();
};

}
