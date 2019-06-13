// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "globals.hpp"
#include "WindowDelegate.hpp"
#include "WindowWidget.hpp"
#include "Device_forward.hpp"
#include "Mouse.hpp"
#include "TTauri/geometry.hpp"
#include "TTauri/logging.hpp"
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
        INITIALIZING, //!< The window has not been initialized yet.
        NO_WINDOW, //!< The window was destroyed, the device will drop the window on the next render cycle.
        NO_DEVICE, //!< No device is associated with the Window and can therefor not be rendered on.
        NO_SURFACE, //!< Need to request a new surface before building a swapchain
        NO_SWAPCHAIN, //! Need to request a swapchain before rendering.
        READY_TO_RENDER, //!< The swapchain is ready drawing is allowed.
        SWAPCHAIN_LOST, //!< The window was resized, the swapchain needs to be rebuild and can not be rendered on.
        SURFACE_LOST, //!< The Vulkan surface on the window was destroyed.
        DEVICE_LOST, //!< The device was lost, but the window could move to a new device, or the device can be recreated.
        WINDOW_LOST, //!< The window was destroyed, need to cleanup.
    };

    struct SwapChainError : virtual boost::exception, virtual std::exception {};

    State state = State::NO_DEVICE;

    /*! The current cursor that is being displayed.
     */
    Cursor currentCursor = Cursor::None;

    /*! The window is currently being resized by the user.
     * We can disable expensive redraws during rendering until this
     * is false again.
     */
    bool resizing = false;

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

    Window_base(const std::shared_ptr<WindowDelegate> delegate, const std::string title);
    virtual ~Window_base();

    Window_base(const Window_base &) = delete;
    Window_base &operator=(const Window_base &) = delete;
    Window_base(Window_base &&) = delete;
    Window_base &operator=(Window_base &&) = delete;

    virtual void initialize();

    /*! Set GPU device to manage this window.
     * Change of the device may be done at runtime.
     */
    void setDevice(const std::weak_ptr<Device> device);

    /*! Remove the GPU device from the window, making it an orphan.
     */
    void unsetDevice() { setDevice({}); }

    /*! Update window.
     * This will update animations and redraw all widgets managed by this window.
     */
    virtual void render() = 0;

    bool isClosed() {
        std::scoped_lock lock(TTauri::GUI::mutex);
        return state == State::NO_WINDOW;
    }

    BoxModel &box() {
        return widget->box;
    }

    rhea::solver& addConstraint(rhea::constraint const& constraint) {
        auto &r = widgetSolver.add_constraint(constraint);
        calculateMinimumAndMaximumWindowExtent();
        return r;
    }

    rhea::solver& removeConstraint(rhea::constraint const& constraint) {
        auto &r = widgetSolver.remove_constraint(constraint);
        calculateMinimumAndMaximumWindowExtent();
        return r;
    }

    virtual void setCursor(Cursor cursor) = 0;

protected:
    /*! The current rectangle which has been set by the operating system.
     * This value may lag behind the actual window extent as seen by the GPU
     * library. This value should only be read by the GPU library during
     * resize to determine the extent of the surface when the GPU library can
     * not figure this out by itself.
     */
    u32rect2 OSWindowRectangle;

    //! The minimum window extent as calculated by laying out all the widgets.
    extent2 minimumWindowExtent;

    //! The maximum window extent as calculated by laying out all the widgets.
    extent2 maximumWindowExtent;

    //! The current window extent as set by the GPU library.
    extent2 currentWindowExtent;


    /*! Called when the GPU library has changed the window size.
     */
    virtual void windowChangedSize(extent2 extent) {
        removeCurrentWindowExtentConstraints();
        currentWindowExtent = extent;
        addCurrentWindowExtentConstraints();
    }

    /*! call openingWindow() on the delegate. 
     */
    virtual void openingWindow();

    /*! call closingWindow() on the delegate.
     */
    virtual void closingWindow();

    /*! Teardown Window based on State::*_LOST
     */
    virtual void teardown() = 0;

    /*! Build Windows based on State::NO_*
     */
    virtual void build() = 0;

    /*! Mouse moved.
     * Called by the operating system to show the position of the mouse.
     * This is called very often so it must be made efficient.
     * Most often this function is used to determine the mouse cursor.
     */
    void handleMouseEvent(MouseEvent event) {
        widget->handleMouseEvent(event);
    }

private:
    //! This solver determines size and position of all widgets in this window.
    rhea::simplex_solver widgetSolver;

    //! Stay constraint for the currentWindowExtent width.
    rhea::constraint currentWindowExtentWidthConstraint;

    //! Stay constraint for the currentWindowExtent height.
    rhea::constraint currentWindowExtentHeightConstraint;

    //! Flag to determine in the currentWindowExtent constrains are active.
    bool currentWindowExtentConstraintActive = false;

    void removeCurrentWindowExtentConstraints() {
        if (currentWindowExtentConstraintActive) {
            widgetSolver.remove_constraint(currentWindowExtentWidthConstraint);
            widgetSolver.remove_constraint(currentWindowExtentHeightConstraint);
            currentWindowExtentConstraintActive = false;
        }
    }

    void addCurrentWindowExtentConstraints() {
        if (!currentWindowExtentConstraintActive) {
            auto widthEquation = widget->box.width == currentWindowExtent.width();
            auto heightEquation = widget->box.width == currentWindowExtent.width();

            currentWindowExtentWidthConstraint = rhea::constraint(widthEquation, rhea::strength::weak(), 1.0);
            currentWindowExtentHeightConstraint = rhea::constraint(heightEquation, rhea::strength::weak(), 1.0);
            widgetSolver.add_constraint(currentWindowExtentWidthConstraint);
            widgetSolver.add_constraint(currentWindowExtentHeightConstraint);
            currentWindowExtentConstraintActive = true;
        }
    }

    void calculateMinimumAndMaximumWindowExtent() {
        removeCurrentWindowExtentConstraints();

        widgetSolver.suggest(widget->box.width, 0);
        widgetSolver.suggest(widget->box.height, 0);
        minimumWindowExtent = widget->box.currentExtent();

        widgetSolver.suggest(widget->box.width, static_cast<double>(std::numeric_limits<uint32_t>::max()));
        widgetSolver.suggest(widget->box.height, static_cast<double>(std::numeric_limits<uint32_t>::max()));
        maximumWindowExtent = widget->box.currentExtent();

        LOG_INFO("Window '%s' minimumExtent(%i,%i) maximumExtent(%i,%i)",
            title,
            minimumWindowExtent.width(), minimumWindowExtent.height(),
            maximumWindowExtent.width(), maximumWindowExtent.height()
        );

        addCurrentWindowExtentConstraints();
    }

};

}
