// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "../GUI/Window_forward.hpp"
#include "../GUI/GUIDevice_forward.hpp"
#include "../GUI/MouseEvent.hpp"
#include "../GUI/HitBox.hpp"
#include "../GUI/KeyboardEvent.hpp"
#include "../GUI/Theme.hpp"
#include "../GUI/DrawContext.hpp"
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
#include "../ranged_numeric.hpp"
#include <limits>
#include <memory>
#include <vector>
#include <mutex>
#include <typeinfo>

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
 * All methods should make sure the mutex is locked by the current thread.
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
 * A widget should return true if the window needs to be redrawn.
 *
 * ## Drawing (optional)
 * A widget can draw itself when the `draw()` function is called. This phase is only
 * entered when one of the widget's layout was changed. But if this phase is entered
 * then all the widgets' `draw()` functions are called.
 */
class Widget {
public:
    /** The mutex of a widget.
     * Certain accessor method of the Widget class require the mutex
     * to be already locked by the caller.
     */
    mutable unfair_recursive_mutex mutex;

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

    /** Get the margin around the Widget.
     * A container widget should layout the children in such
     * a way that the maximum margin of neighbouring widgets is maintained.
     *
     * @pre `mutex` must be locked by current thread.
     * @return The margin for this widget.
     */
    [[nodiscard]] float margin() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return _margin;
    }

    /** The first drawing layer of the widget.
     * Drawing layers start at 0.0 and go up to 100.0.
     *
     * Each child widget that has drawing to do increases the layer by 1.0.
     *
     * The widget should draw within 0.0 and 1.0 of its drawing layer.
     * The toWindowTransfer and the DrawingContext will already include
     * the draw_layer.
     *
     * An overlay widget such as popups will increase the layer by 25.0,
     * to make sure the overlay will draw above other widgets in the window.
     *
     * @pre `mutex` must be locked by current thread.
     * @return The draw layer of this widget.
     */
    [[nodiscard]] float draw_layer() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return _draw_layer;
    }

    /** The logical layer of the widget.
     * The logical layer can be used to determine how for away
     * from the window-widget (root) the current widget is.
     * 
     * Logical layers start at 0 for the window-widget.
     * Each child widget increases the logical layer by 1.
     *
     * @pre `mutex` must be locked by current thread.
     * @return The draw layer of this widget.
     */
    [[nodiscard]] int logical_layer() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return _logical_layer;
    }

    /** The semantic layer of the widget.
     *
     * The semantic layer is used mostly by the `draw()` function
     * for selecting colors from the theme, to denote nesting widgets
     * inside other widgets.
     * 
     * Semantic layers start at 0 for the window-widget and for any pop-up
     * widgets.
     * 
     * The semantic layer is increased by one, whenever a user of the
     * user-interface would understand the next layer to begin.
     *
     * In most cases it would mean that a container widget that does not
     * draw itself will not increase the semantic_layer number.
     *
     * @pre `mutex` must be locked by current thread.
     * @return The draw layer of this widget.
     */
    [[nodiscard]] int semantic_layer() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return _semantic_layer;
    }

    /** Get the resistance in width.
     * The amount of resistance the widget has for resizing to larger than the minimum size.
     *
     * @pre `mutex` must be locked by current thread.
     * @return The amount of resistance to horizontal resize.
     * @retval 0 Greedy: Widget will try to become the largest.
     * @retval 1 Normal: Widget will share the space with others.
     * @retval 2 Resist: Widget will try to maintain minimum size.
     */
    [[nodiscard]] ranged_int<3> width_resistance() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return _width_resistance;
    }

    /** Get the resistance in height.
     * The amount of resistance the widget has for resizing to larger than the minimum size.
     *
     * @pre `mutex` must be locked by current thread.
     * @return The amount of resistance to vertical resize.
     * @retval 0 Greedy: Widget will try to become the largest.
     * @retval 1 Normal: Widget will share the space with others.
     * @retval 2 Resist: Widget will try to maintain minimum size.
     */
    [[nodiscard]] ranged_int<3> height_resistance() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return _height_resistance;
    }

    /** Get the size-range of the widget.
     * This size-range is used to determine the size of container widgets
     * and eventually the size of the window.
     *
     * @pre `mutex` must be locked by current thread.
     * @pre `updateConstraint()` must be called first.
     * @return The minimum and maximum size as an interval of a 2D vector.
     */
    [[nodiscard]] interval_vec2 preferred_size() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return _preferred_size;
    }

    /** Get the relative_base_line of the widget.
     * This value is used by a container widget to align the text
     * of its children on the same base-line.
     *
     * By taking `std::max()`  of the `preferred_base_line()` of all children,
     * you will get the highest priority relative-base-line. The container
     * can then use the `relative_base_line::position()` method to get the
     * base-line position that should be used for all widgets in a row.
     * 
     * @pre `mutex` must be locked by current thread.
     * @pre `updateConstraint()` must be called first.
     * @return A relative base-line position preferred by this widget.
     */
    [[nodiscard]] relative_base_line preferred_base_line() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return _preferred_base_line;
    }

    /** Set the location and size of the widget inside the window.
     *
     * @pre `mutex` must be locked by current thread.
     * @param _window_rectangle The location and size of the widget inside the window
     * @param _window_base_line The position of the text base-line from the bottom of the window. When not given
     *                          the middle of the _window_rectangle is used together with the preferred
     *                          relative base-line.
     */
    void set_window_rectangle(aarect const &_window_rectangle, float _window_base_line = std::numeric_limits<float>::infinity()) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        window_rectangle = _window_rectangle;
        if (std::isinf(_window_base_line)) {
            window_base_line = _preferred_base_line.position(window_rectangle.bottom(), window_rectangle.top());
        } else {
            window_base_line = _window_base_line;
        }
    }

    /** Get the clipping rectangle in window coordinates
     *
     * @pre `mutex` must be locked by current thread.
     */
    [[nodiscard]] aarect clipping_rectangle() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return expand(window_rectangle, _margin);
    }

    /** Get the rectangle in local coordinates.
     *
     * @pre `mutex` must be locked by current thread.
     */
    [[nodiscard]] aarect rectangle() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return aarect{window_rectangle.extent()};
    }

    /** Get the base-line in local coordinates.
     *
     * @pre `mutex` must be locked by current thread.
     */
    [[nodiscard]] float base_line() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return window_base_line - window_rectangle.y();
    }

    [[nodiscard]] GUIDevice *device() const noexcept;

    /** Find the widget that is under the mouse cursor.
     * This function will recursively test with visual child widgets, when
     * widgets overlap on the screen the hitbox object with the highest elevation is returned.
     * 
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
     * @pre `mutex` must be locked by current thread.
     */
    [[nodiscard]] virtual bool acceptsFocus() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return false;
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
     * @pre `mutex` must be locked by current thread.
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
     * @pre `mutex` must be locked by current thread.
     * @param display_time_point The time point when the widget will be shown on the screen.
     * @param need_layout Force the widget to layout
     * @retrun True if the widget, or any of the children requires the window to be redrawn.
     */
    [[nodiscard]] virtual bool updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept;

    /** Make a draw context for this widget.
     * This function will make a draw context with the correct transformation
     * and default color values.
     * 
     * @pre `mutex` must be locked by current thread.
     * @param context A template drawing context. This template may be taken
     *                from the parent's draw call.
     * @return A new draw context for drawing the current widget in the
     *         local coordinate system.
     */
    DrawContext makeDrawContext(DrawContext context) const noexcept;

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
     * @pre `mutex` must be locked by current thread.
     * @param drawContext The context to where the widget will draw.
     * @param displayTimePoint The time point when the widget will be shown on the screen.
     */
    virtual void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
    }

    /** Handle command.
     *
     * @pre `mutex` must be locked by current thread.
     */
    virtual void handleCommand(command command) noexcept;

    /*! Handle mouse event.
     * Called by the operating system to show the position and button state of the mouse.
     * This is called very often so it must be made efficient.
     * This function is also used to determine the mouse cursor.
     *
     * @param event The mouse event, positions are in window coordinates.
     */
    virtual void handleMouseEvent(MouseEvent const &event) noexcept;

    /** Find the next widget that handles keyboard focus.
     * This recursively looks for the current keyboard widget, then returns the next (or previous) widget
     * that returns true from `acceptsFocus()`.
     * 
     * @param currentKeyboardWidget The widget that currently has focus, or nullptr to get the first widget
     *                              that accepts focus.
     * @param reverse Walk the widget tree in reverse order.
     * @return A pointer to the next widget.
     * @retval currentKeyboardWidget when currentKeyboardWidget was found but no next widget was found.
     * @retval nullptr when currentKeyboardWidget is not found in this Widget.
     */
    [[nodiscard]] virtual Widget const *nextKeyboardWidget(Widget const *currentKeyboardWidget, bool reverse) const noexcept;

    /** Handle keyboard event.
     * Called by the operating system when editing text, or entering special keys
     *
     * Thread safety: locks
     */
    virtual void handleKeyboardEvent(KeyboardEvent const &event) noexcept;

protected:

    /** Convenient reference to the Window.
     */
    Window &window;

    /** Pointer to the parent widget.
     * May be a nullptr only when this is the top level widget.
     */
    Widget *parent;

    /** Mouse cursor is hovering over the widget.
     */
    bool hover = false;

    /** The widget has keyboard focus.
     */
    bool focus = false;

    /** Transformation matrix from window coords to local coords.
     */
    mat::T fromWindowTransform;

    /** Transformation matrix from local coords to window coords.
     */
    mat::T toWindowTransform;

    /** When set to true the widget will recalculate the constraints on the next call to `updateConstraints()`
     */
    bool requestConstraint = true;

    /** When set to true the widget will recalculate the layout on the next call to `updateLayout()`
     */
    bool requestLayout = true;

    /** The position of the widget on the window.
     */
    aarect window_rectangle;

    /** The height of the base line from the bottom of the window.
     */
    float window_base_line;

    interval_vec2 _preferred_size;

    relative_base_line _preferred_base_line = relative_base_line{};

    ranged_int<3> _width_resistance = 1;
    ranged_int<3> _height_resistance = 1;

    float _margin = Theme::margin;

    /** The draw layer of the widget.
     * @sa draw_layer() const
     */
    float _draw_layer;

    /** The draw layer of the widget.
     * @sa semantic_layer() const
     */
    int _semantic_layer;

    /** The draw layer of the widget.
     * @sa logical_layer() const
     */
    int _logical_layer;
};

} // namespace tt
