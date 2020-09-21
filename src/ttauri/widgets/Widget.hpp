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
#include "../attributes.hpp"
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

/** Result of `Widget::updateConstraints()` and `Widget::updateLayout()`.
 */
enum class WidgetUpdateResult { Nothing, Children, Self };

[[nodiscard]] constexpr WidgetUpdateResult operator|(WidgetUpdateResult const &lhs, WidgetUpdateResult const &rhs) noexcept
{
    return std::max(lhs, rhs);
}

[[nodiscard]] constexpr WidgetUpdateResult operator&(WidgetUpdateResult const &lhs, WidgetUpdateResult const &rhs) noexcept
{
    return std::min(lhs, rhs);
}

constexpr WidgetUpdateResult &operator|=(WidgetUpdateResult &lhs, WidgetUpdateResult const &rhs) noexcept
{
    lhs = lhs | rhs;
    return lhs;
}

constexpr WidgetUpdateResult &operator&=(WidgetUpdateResult &lhs, WidgetUpdateResult const &rhs) noexcept
{
    lhs = lhs & rhs;
    return lhs;
}

/*! View of a widget.
 * A view contains the dynamic data for a Widget. It is often accompanied with a Backing
 * which contains that static data of an Widget and the drawing code. Backings are shared
 * between Views.
 *
 * All methods should lock the mutex, unless they use only the atomic data.
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
    interval_vec2 _size;
    base_line _preferred_base_line = base_line{};
    aarect _window_rectangle;
    float _window_base_line_position;

    /** Transformation matrix from window coords to local coords.
     */
    mat::T fromWindowTransform;

    /** Transformation matrix from local coords to window coords.
     */
    mat::T toWindowTransform;

    /** Offset to apply to mouse position.
     */
    vec offsetFromParent;

    mutable std::atomic<bool> requestConstraint = true;
    mutable std::atomic<bool> requestLayout = true;

    /** The widget is enabled.
     */
    observable<bool> enabled = true;

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
    [[nodiscard]] interval_vec2 size() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return _size;
    }

    [[nodiscard]] base_line preferred_base_line() const noexcept
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

    /** Get the clipping rectangle in window coordinates
     * Thread safety: requires external lock on `mutex`.
     */
    [[nodiscard]] aarect clipping_rectangle() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return expand(_window_rectangle, margin);
    }

    void set_window_rectangle_and_base_line_position(aarect window_rectangle, float window_base_line_position) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        _window_rectangle = window_rectangle;
        _window_base_line_position = window_base_line_position;
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
    [[nodiscard]] float base_line_position() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return _window_base_line_position - _window_rectangle.y();
    }

    [[nodiscard]] GUIDevice *device() const noexcept;

    /** Find the widget that is under the mouse cursor.
     *
     * Thread safety: locks.
     */
    [[nodiscard]] virtual HitBox hitBoxTest(vec position) const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
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
     * This function is called by the window on every frame. It should recursively
     * call this function on every visible child.
     *
     * This function should update the constraints whenever the data changes,
     * such as changing the text of a label, or when adding or removing child widgets.
     *
     * @return If true then the constraints have been updated by this widget, or
     *         any of its children.
     *         This value should be used by the overriding function to determine if
     *         it should update the constraints.
     */
    [[nodiscard]] virtual WidgetUpdateResult updateConstraints() noexcept;

    /** Update the internal layout of the widget.
     * This function is called by the window on every frame. It should recursively
     * call this function on every visible child.
     *
     * This function should update the internal layout whenever data changes or during
     * an animation.
     *
     * @param displayTimePoint The time point when the widget will be shown on the screen.
     * @param forceLayout Force the layout of this widget and its children
     * @return If true then the internal layout has been updated by this widget, or
     *         any of its children.
     *         This value should be used by the overriding function to determine if
     *         it should update the internal layout.
     */
    [[nodiscard]] virtual WidgetUpdateResult
    updateLayout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept;

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

    [[nodiscard]] virtual Widget *nextKeyboardWidget(Widget const *currentKeyboardWidget, bool reverse) const noexcept;

    /*! Handle keyboard event.
     * Called by the operating system when editing text, or entering special keys
     *
     * Thread safety: locks
     */
    virtual void handleKeyboardEvent(KeyboardEvent const &event) noexcept;

protected:
};

inline Widget *const foundWidgetPtr = reinterpret_cast<Widget *>(1);

} // namespace tt
