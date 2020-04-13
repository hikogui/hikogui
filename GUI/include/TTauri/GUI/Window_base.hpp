// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/globals.hpp"
#include "TTauri/GUI/WindowDelegate.hpp"
#include "TTauri/GUI/WindowWidget.hpp"
#include "TTauri/GUI/Device_forward.hpp"
#include "TTauri/GUI/Cursor.hpp"
#include "TTauri/GUI/HitBox.hpp"
#include "TTauri/GUI/MouseEvent.hpp"
#include "TTauri/GUI/KeyboardEvent.hpp"
#include "TTauri/Text/gstring.hpp"
#include "TTauri/Foundation/attributes.hpp"
#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/rect.hpp"
#include "TTauri/Foundation/ivec.hpp"
#include "TTauri/Foundation/irect.hpp"
#include "TTauri/Foundation/Trigger.hpp"
#include "TTauri/Foundation/cpu_utc_clock.hpp"
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
class Window_base {
public:
    enum class State {
        Initializing, //!< The window has not been initialized yet.
        NoWindow, //!< The window was destroyed, the device will drop the window on the next render cycle.
        NoDevice, //!< No device is associated with the Window and can therefor not be rendered on.
        NoSurface, //!< Need to request a new surface before building a swapchain
        NoSwapchain, //! Need to request a swapchain before rendering.
        ReadyToRender, //!< The swapchain is ready drawing is allowed.
        SwapchainLost, //!< The window was resized, the swapchain needs to be rebuild and can not be rendered on.
        SurfaceLost, //!< The Vulkan surface on the window was destroyed.
        DeviceLost, //!< The device was lost, but the window could move to a new device, or the device can be recreated.
        WindowLost, //!< The window was destroyed, need to cleanup.
    };

    enum class Size {
        Normal,
        Minimized,
        Maximized
    };

    static constexpr long long resizeFrameRateDivider = 1;

    State state = State::NoDevice;

    /** The current cursor.
     * Used for optimizing when the operating system cursor is updated.
     * Set to Cursor::None at the start (for the wait icon) and when the
     * operating system is going to display another icon to make sure
     * when it comes back in the application the cursor will be updated
     * correctly.
     */
    Cursor currentCursor = Cursor::None;

    /*! The current frame number that is being rendered.
     */
    long long frameCount = 0; 

    /*! The window is currently being resized by the user.
     * We can disable expensive redraws during rendering until this
     * is false again.
     */
    std::atomic<bool> resizing = false;

    /*! The window is currently active.
     * Widgets may want to reduce redraws, or change colors.
     */
    std::atomic<bool> active = false;

    /*! Current size state of the window.
     */
    Size size = Size::Normal;

    std::shared_ptr<WindowDelegate> delegate;

    std::string title;

    Device *device = nullptr;

    /*! Orientation of the RGB subpixels.
     */
    SubpixelOrientation subpixelOrientation = SubpixelOrientation::RedLeft;

    /*! Dots-per-inch of the screen where the window is located.
     * If the window is located on multiple screens then one of the screens is used as
     * the source for the DPI value.
     */
    float dpi = 72.0;

    /*! Pixels-per-Point
     * A point references a typographic point, 1/72 inch.
     * Scale all drawing and sizing on the window using this attribute.
     * This value is rounded to an integer value for drawing clean lines.
     */
    float ppp = 1.0;

    //! The widget covering the complete window.
    std::unique_ptr<Widgets::WindowWidget> widget;

    /** Target of the mouse
     * Since any mouse event will change the target this is used
     * to check if the target has changed, to send exit events to the previous mouse target.
     */
    Widgets::Widget *mouseTargetWidget = nullptr;

    /** Target of the keyboard
     * widget where keyboard events are sent to.
     */
    Widgets::Widget *keyboardTargetWidget = nullptr;

    /** The first widget in the window that needs to be selected.
     * This widget is selected when the window is opened and when
     * pressing tab when no other widget is selected.
     */
    Widgets::Widget *firstKeyboardWidget = nullptr;

    /** The first widget in the window that needs to be selected.
     * This widget is selected when pressing shift-tab when no other widget is selected.
     */
    Widgets::Widget *lastKeyboardWidget = nullptr;

    /** Trigger to check when to render the window.
     */
    Trigger<cpu_utc_clock> renderTrigger;

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
    void setDevice(Device *device);

    /*! Remove the GPU device from the window, making it an orphan.
     */
    void unsetDevice() { setDevice({}); }

    /*! Update window.
     * This will update animations and redraw all widgets managed by this window.
     */
    virtual void render(cpu_utc_clock::time_point displayTimePoint) = 0;

    bool isClosed() {
        std::scoped_lock lock(GUI_globals->mutex);
        return state == State::NoWindow;
    }

    template<typename T, typename... Args>
    T &addWidget(Args... args) {
        ttauri_assume(widget);
        return widget->addWidget<T>(args...);
    }

    rhea::solver& addConstraint(rhea::constraint const& constraint) noexcept {
        auto &r = widgetSolver.add_constraint(constraint);
        // During the construction of WindowWidget `widget` is not yet set.
        if (widget) {
            calculateMinimumAndMaximumWindowExtent();
            widget->handleWindowResize();
        }
        return r;
    }

    rhea::solver& removeConstraint(rhea::constraint const& constraint) noexcept {
        auto &r = widgetSolver.remove_constraint(constraint);
        // During the construction of WindowWidget `widget` is not yet set.
        if (widget) {
            calculateMinimumAndMaximumWindowExtent();
            widget->handleWindowResize();
        }
        return r;
    }

    virtual void setCursor(Cursor cursor) = 0;

    virtual void closeWindow() = 0;

    virtual void minimizeWindow() = 0;
    virtual void maximizeWindow() = 0;
    virtual void normalizeWindow() = 0;

    [[nodiscard]] virtual std::string getTextFromClipboard() const noexcept = 0;
    virtual void setTextOnClipboard(std::string str) noexcept = 0;

    void updateToNextKeyboardTarget(Widgets::Widget *currentTargetWidget) noexcept {
        Widgets::Widget *newTargetWidget =
            currentTargetWidget != nullptr ? currentTargetWidget->nextKeyboardWidget : firstKeyboardWidget;

        while (newTargetWidget != nullptr && !newTargetWidget->acceptsFocus()) {
            newTargetWidget = newTargetWidget->nextKeyboardWidget;
        }

        updateKeyboardTarget(newTargetWidget);
    }

    void updateToPrevKeyboardTarget(Widgets::Widget *currentTargetWidget) noexcept {
        Widgets::Widget *newTargetWidget =
            currentTargetWidget != nullptr ? currentTargetWidget->prevKeyboardWidget : lastKeyboardWidget;

        while (newTargetWidget != nullptr && !newTargetWidget->acceptsFocus()) {
            newTargetWidget = newTargetWidget->prevKeyboardWidget;
        }

        updateKeyboardTarget(newTargetWidget);
    }

protected:
    /*! The current rectangle which has been set by the operating system.
     * This value may lag behind the actual window extent as seen by the GPU
     * library. This value should only be read by the GPU library during
     * resize to determine the extent of the surface when the GPU library can
     * not figure this out by itself.
     */
    irect OSWindowRectangle;

    //! The minimum window extent as calculated by laying out all the widgets.
    ivec minimumWindowExtent;

    //! The maximum window extent as calculated by laying out all the widgets.
    ivec maximumWindowExtent;

    //! The current window extent as set by the GPU library.
    ivec currentWindowExtent;

    /*! Called when the GPU library has changed the window size.
     */
    virtual void windowChangedSize(ivec extent) {
        removeCurrentWindowExtentConstraints();
        currentWindowExtent = extent;
        addCurrentWindowExtentConstraints();

        widget->handleWindowResize();
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

    void updateMouseTarget(Widgets::Widget *newTargetWidget) noexcept {
        if (newTargetWidget != mouseTargetWidget) {
            if (mouseTargetWidget != nullptr) {
                mouseTargetWidget->handleMouseEvent(MouseEvent::exited());
            }
            mouseTargetWidget = newTargetWidget;
            if (mouseTargetWidget != nullptr) {
                mouseTargetWidget->handleMouseEvent(MouseEvent::entered());
            }
        }
    }

    void updateKeyboardTarget(Widgets::Widget *newTargetWidget) noexcept {
        if (newTargetWidget == nullptr || !newTargetWidget->acceptsFocus()) {
            newTargetWidget = nullptr;
        }

        if (newTargetWidget != keyboardTargetWidget) {
            if (keyboardTargetWidget != nullptr) {
                keyboardTargetWidget->handleKeyboardEvent(KeyboardEvent::exited());
            }
            keyboardTargetWidget = newTargetWidget;
            if (keyboardTargetWidget != nullptr) {
                keyboardTargetWidget->handleKeyboardEvent(KeyboardEvent::entered());
            }
        }
    }

    /*! Mouse moved.
     * Called by the operating system to show the position of the mouse.
     * This is called very often so it must be made efficient.
     * Most often this function is used to determine the mouse cursor.
     */
    void handleMouseEvent(MouseEvent const &event) noexcept {
        let hitbox = hitBoxTest(event.position);

        updateMouseTarget(hitbox.widget);

        // On click change keyboard-focus to the widget if it accepts it.
        if (event.down.leftButton) {
            updateKeyboardTarget(hitbox.widget);
        }

        // Send event to target-widget.
        if (mouseTargetWidget != nullptr) {
            mouseTargetWidget->handleMouseEvent(event);
        }
    }

    /*! Handle keyboard event.
    * Called by the operating system to show the character that was entered
    * or special key that was used.
    */
    void handleKeyboardEvent(KeyboardEvent const &event) noexcept {
        if (keyboardTargetWidget != nullptr) {
            keyboardTargetWidget->handleKeyboardEvent(event);

        } else if (event.type == KeyboardEvent::Type::Key) {
            // If no widgets have been selected handle the keyboard-focus changes.
            for (let command : event.getCommands()) {
                if (command == "gui.widget.next"_ltag) {
                    updateToNextKeyboardTarget(nullptr);
                } else if (command == "gui.widget.prev"_ltag) {
                    updateToPrevKeyboardTarget(nullptr);
                }
            }
        }
    }

    void handleKeyboardEvent(KeyboardState state, KeyboardModifiers modifiers, KeyboardVirtualKey key) noexcept {
        return handleKeyboardEvent(KeyboardEvent(state, modifiers, key));
    }

    void handleKeyboardEvent(Text::Grapheme grapheme, bool full=true) noexcept {
        return handleKeyboardEvent(KeyboardEvent(grapheme, full));
    }

    void handleKeyboardEvent(char32_t c, bool full=true) noexcept {
        return handleKeyboardEvent(Text::Grapheme(c), full);
    }

    /*! Test where the certain features of a window are located.
     */
    HitBox hitBoxTest(vec position) noexcept {
        return widget->hitBoxTest(position);
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
        ttauri_assume(widget);

        if (currentWindowExtentConstraintActive) {
            widgetSolver.remove_constraint(currentWindowExtentWidthConstraint);
            widgetSolver.remove_constraint(currentWindowExtentHeightConstraint);
            currentWindowExtentConstraintActive = false;
        }
    }

    void addCurrentWindowExtentConstraints() {
        ttauri_assume(widget);

        if (!currentWindowExtentConstraintActive) {
            auto widthEquation = widget->box.width == currentWindowExtent.x();
            auto heightEquation = widget->box.height == currentWindowExtent.y();

            currentWindowExtentWidthConstraint = rhea::constraint(widthEquation, rhea::strength::weak(), 1.0);
            currentWindowExtentHeightConstraint = rhea::constraint(heightEquation, rhea::strength::weak(), 1.0);
            widgetSolver.add_constraint(currentWindowExtentWidthConstraint);
            widgetSolver.add_constraint(currentWindowExtentHeightConstraint);
            currentWindowExtentConstraintActive = true;
        }
    }

    void calculateMinimumAndMaximumWindowExtent() {
        ttauri_assume(widget);

        removeCurrentWindowExtentConstraints();

        widgetSolver.suggest(widget->box.width, 0);
        widgetSolver.suggest(widget->box.height, 0);
        minimumWindowExtent = widget->box.currentExtent();

        widgetSolver.suggest(widget->box.width, static_cast<double>(std::numeric_limits<uint32_t>::max()));
        widgetSolver.suggest(widget->box.height, static_cast<double>(std::numeric_limits<uint32_t>::max()));
        maximumWindowExtent = widget->box.currentExtent();

        LOG_INFO("Window '{}' minimumExtent={} maximumExtent={}", title, minimumWindowExtent, maximumWindowExtent);

        addCurrentWindowExtentConstraints();
    }


};

}
