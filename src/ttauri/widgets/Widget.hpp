// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "../GUI/Window_forward.hpp"
#include "../GUI/GUIDevice_forward.hpp"
#include "../GUI/MouseEvent.hpp"
#include "../GUI/HitBox.hpp"
#include "../GUI/KeyboardEvent.hpp"
#include "../GUI/Theme.hpp"
#include "../text/ShapedText.hpp"
#include "../alignment.hpp"
#include "../Path.hpp"
#include "../R16G16B16A16SFloat.hpp"
#include "../R32G32SFloat.hpp"
#include "../URL.hpp"
#include "../vspan.hpp"
#include "../utils.hpp"
#include "../Trigger.hpp"
#include "../cpu_utc_clock.hpp"
#include "../observable.hpp"
#include "../command.hpp"
#include "../unfair_recursive_mutex.hpp"
#include "../interval_vec2.hpp"
#include "../flow_layout.hpp"
#include <limits>
#include <memory>
#include <vector>
#include <mutex>
#include <typeinfo>

namespace tt {
class DrawContext;
}
namespace tt::PipelineImage {
struct Image;
struct Vertex;
} // namespace tt::PipelineImage
namespace tt::PipelineSDF {
struct Vertex;
}
namespace tt::PipelineFlat {
struct Vertex;
}
namespace tt::PipelineBox {
struct Vertex;
}

namespace tt {

/*! View of a widget.
 * A view contains the dynamic data for a Widget. It is often accompanied with a Backing
 * which contains that static data of an Widget and the drawing code. Backings are shared
 * between Views.
 *
 * All methods should lock make sure the mutex is locked by the current thread.
 *
 * Rendering is done in three distinct phases:
 *  1. Updating Constraints
 *  2. Updating Layout
 *  3. Drawing (optional phase)
 *
 * Each of these phases is executed to completion for all widget belonging to a
 * window before the next phases is executed.
 *
 * ## Updating Constraints
 * In this phases the widget will update the constraints to determine the position
 * and size of the widget inside the window.
 *
 * The `updateConstraints()` function will be called on each widget recursively.
 * You should minimize the cost of this function as much as possible.
 *
 * Since this function is called on each frame, the widget should first check
 * if constraint changes are needed.
 *
 * A widget should return true if any of the constraints has changed.
 *
 * ## Updating Layout
 * A widget may update its internal (expensive) layout calculations from the
 * `updateLayout()` function.
 *
 * Since this function is called on each frame, the widget should first check
 * if layout calculations are needed. If a constraint has changed (the window size
 * is also a constraint) then the `forceLayout` flag is set.
 *
 * A widget should return true if a widget has changed its layout.
 *
 * ## Drawing (optional)
 * A widget can draw itself when the `draw()` function is called. This phase is only
 * entered when one of the widget's layout was changed. But if this phase is entered
 * then all the widget's `draw()` functions are called.
 */
class Widget {
protected:
    /** Pointer to the parent widget.
     * May be a nullptr only when this is the top level widget.
     */
    Widget *parent;

public:
    /** Convenient reference to the Window.
     */
    Window &window;

    mutable unfair_recursive_mutex mutex;

    /** Mouse cursor is hovering over the widget.
     */
    bool hover = false;

    /** The widget has keyboard focus.
     */
    bool focus = false;

    float elevation;

    /** The margin outside the widget.
     */
    float margin = Theme::margin;
    interval_vec2 _preferred_size;
    flow_resistance height_resistance = flow_resistance::normal;
    flow_resistance width_resistance = flow_resistance::normal;
    relative_base_line _preferred_base_line = relative_base_line{};

    aarect _window_rectangle;
    float _window_base_line;

    

    /** When set to true the widget will recalculate the constraints on the next call to `updateConstraints()`
     */
    bool requestConstraint = true;

    /** When set to true the widget will recalculate the layout on the next call to `updateLayout()`
     */
    bool requestLayout = true;

    /** The widget is enabled.
     */
    observable<bool> enabled = true;

    /** Transformation matrix from window coords to local coords.
     */
    mat::T fromWindowTransform;

    /** Transformation matrix from local coords to window coords.
     */
    mat::T toWindowTransform;

    /*! Constructor for creating sub views.
     */
    Widget(Window &window, Widget *parent) noexcept;

    virtual ~Widget();

    Widget(const Widget &) = delete;
    Widget &operator=(const Widget &) = delete;
    Widget(Widget &&) = delete;
    Widget &operator=(Widget &&) = delete;

    /** Get the minimum size of the widget.
     * Thread safety: requires external lock on `mutex`.
     */
    [[nodiscard]] interval_vec2 preferred_size() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return _preferred_size;
    }

    [[nodiscard]] relative_base_line preferred_base_line() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return _preferred_base_line;
    }

    /** Get the rectangle in window coordinates.
     * Thread safety: requires external lock on `mutex`.
     */
    [[nodiscard]] aarect window_rectangle() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return _window_rectangle;
    }

    void set_window_rectangle(aarect const &rhs) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        _window_rectangle = rhs;
    }

    [[nodiscard]] float window_base_line() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return _window_base_line;
    }

    void set_window_base_line(float const &rhs) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        _window_base_line = rhs;
    }

    /** Get the clipping rectangle in window coordinates
     * Thread safety: requires external lock on `mutex`.
     */
    [[nodiscard]] aarect clipping_rectangle() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return expand(_window_rectangle, margin);
    }

    /** Get the rectangle in local coordinates.
     * Thread safety: requires external lock on `mutex`.
     */
    [[nodiscard]] aarect rectangle() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return aarect{_window_rectangle.extent()};
    }

    /** Get the base-line in local coordinates.
     * Thread safety: requires external lock on `mutex`.
     */
    [[nodiscard]] float base_line() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return _window_base_line - _window_rectangle.y();
    }

    [[nodiscard]] GUIDevice *device() const noexcept;

    /** Find the widget that is under the mouse cursor.
     * This function will recursively test with visual child widgets, when
     * widgets overlap on the screen the hitbox object with the highest elevation is returned.
     * 
     * Thread safety: locks.
     * @param window_position The coordinate of the mouse on the window.
     *                        Use `fromWindowTransform` to convert to widget-local coordinates.
     * @return A HitBox object with the cursor-type and a reference to the widget.
     */
    [[nodiscard]] virtual HitBox hitBoxTest(vec window_position) const noexcept
    {
        return {};
    }

    /** Check if the widget will accept keyboard focus.
     *
     * Thread safety: requires external lock on `mutex`.
     */
    [[nodiscard]] virtual bool acceptsFocus() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return false;
    }

    /** Get nesting level used for selecting colors for the widget.
     */
    [[nodiscard]] ssize_t nestingLevel() noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return numeric_cast<ssize_t>(elevation);
    }

    /** Get z value for compositing order.
     */
    [[nodiscard]] float z() noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return elevation * 0.01f;
    }

    /** Update the constraints of the widget.
     * This function is called on each vertical sync, even if no drawing is to be done.
     * It should recursively call `updateConstraints()` on each of the visible children,
     * so they get a chance to update.
     * 
     * This function may be used for expensive calculations, such as text-shaping, which
     * should only be done when the data changes. Because this function is called on every
     * vertical sync it should cache these calculations.
     * 
     * Subclasses should call `updateConstraints()` on its base-class to check if the constraints where
     * changed. `Widget::updateConstraints()` will check if `requestConstraints` was set.
     * `Container::updateConstraints()` will check if any of the children changed constraints.
     * 
     * If the container, due to a change in constraints, wants the window to resize to the minimum size
     * it should set window.requestResize to true.
     * 
     * This function will change what is returned by `preferred_size()` and `preferred_base_line()`.
     * 
     * @return True if its or any children's constraints has changed.
     */
    [[nodiscard]] virtual bool updateConstraints() noexcept;

    /** Update the internal layout of the widget.
     * This function is called on each vertical sync, even if no drawing is to be done.
     * It should recursively call `updateLayout()` on each of the visible children,
     * so they get a chance to update.
     *
     * This function may be used for expensive calculations, such as geometry calculations,
     * which should only be done when the data or sizes change. Because this function is called
     * on every vertical sync it should cache these calculations.
     *
     * This function will likely call `set_window_rectangle()` and `set_window_base_line()` on
     * its children, before calling `updateLayout()` on that child.
     * 
     * Subclasses should call `updateLayout()` on its children, call `updateLayout()` on its
     * base class with `forceLayout` argument to the result of `layoutRequest.exchange(false)`.
     * 
     * @param display_time_point The time point when the widget will be shown on the screen.
     * @param need_layout Force the widget to layout
     * @retrun True if the widget, or any of the children requires the window to be redrawn.
     */
    [[nodiscard]] virtual bool updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept;

    /** Draw the widget.
     * This function is called by the window (optionally) on every frame.
     * It should recursively call this function on every visible child.
     * This function is only called when `updateLayout()` has returned true.
     *
     * The overriding function should call the base class's `draw()`, the place
     * where the call this function will determine the order of the vertices into
     * each buffer. This is important when needing to do the painters algorithm
     * for alpha-compositing. However the pipelines are always drawn in the same
     * order.
     *
     * @param drawContext The context to where the widget will draw.
     * @param displayTimePoint The time point when the widget will be shown on the screen.
     */
    virtual void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
    }

    /** Handle command.
     *
     * Thread safety: locks
     */
    virtual void handleCommand(command command) noexcept;

    /*! Handle mouse event.
     * Called by the operating system to show the position and button state of the mouse.
     * This is called very often so it must be made efficient.
     * This function is also used to determine the mouse cursor.
     *
     * Thread safety: locks
     */
    virtual void handleMouseEvent(MouseEvent const &event) noexcept;

    /** Find the next widget that handles keyboard focus.
     * This recursively looks for the current keyboard widget, then returns the next (or previous) widget
     * that returns true from `acceptsFocus()`.
     * 
     * @param currentKeyboardWidget The widget that currently has focus, or nullptr to get the first widget
     *                              that accepts focus.
     * @param reverse Walk the widget tree in reverse order.
     * @return A pointer to the next widget. The currentKeyboardWidget when it was found
     *         but no next widget was not yet found. nullptr when currentKeyboardWidget is
     *         not yet found.
     */
    [[nodiscard]] virtual Widget const *nextKeyboardWidget(Widget const *currentKeyboardWidget, bool reverse) const noexcept;

    /** Handle keyboard event.
     * Called by the operating system when editing text, or entering special keys
     *
     * Thread safety: locks
     */
    virtual void handleKeyboardEvent(KeyboardEvent const &event) noexcept;

protected:
};

} // namespace tt
