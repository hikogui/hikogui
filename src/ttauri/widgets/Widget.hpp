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
#include <rhea/constraint.hpp>
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
}
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
 * Thread-safety:
 *     All method of the widget should lock the mutex, exceptions are:
 *         hitBoxTest(), needLayout()
 *     All public members should be thread-safe, for example:
 *         std::atomic and tt::observer
 *     The following methods should only be called from the render thread:
 *         needLayout(), layout(), layoutChildren(), draw(), 
 */
class Widget {
private:
    float _minimumWidth;
    float _minimumHeight;
    float _preferredWidth;
    float _preferredHeight;
    float _maximumWidth;
    float _maximumHeight;

    rhea::constraint minimumWidthConstraint;
    rhea::constraint minimumHeightConstraint;
    rhea::constraint preferredWidthConstraint;
    rhea::constraint preferredHeightConstraint;
    rhea::constraint maximumWidthConstraint;
    rhea::constraint maximumHeightConstraint;

protected:
    mutable std::recursive_mutex mutex;

    /** Convenient reference to the Window.
    */
    Window &window;

    /** Pointer to the parent widget.
    * May be a nullptr only when this is the top level widget.
    */
    Widget *parent;

    rhea::constraint baseConstraint;

public:

    /** Transformation matrix from window coords to local coords.
    */
    mat fromWindowTransform;

    /** Transformation matrix from local coords to window coords.
    */
    mat toWindowTransform;

    /** Mouse cursor is hovering over the widget.
    */
    bool hover = false;

    /** The widget has keyboard focus.
    */
    bool focus = false;

    /** Location of the frame compared to the window.
     * Thread-safety: the box is not modified by the class.
     */
    rhea::variable const left;
    rhea::variable const bottom;
    rhea::variable const width;
    rhea::variable const height;
    rhea::variable const base;

    rhea::linear_expression const right = left + width;
    rhea::linear_expression const centre = left + width * 0.5;
    rhea::linear_expression const top = bottom + height;
    rhea::linear_expression const middle = bottom + height * 0.5;

    float elevation;

    std::atomic<R32G32SFloat> _extent;
    std::atomic<R32G32SFloat> _offsetFromParent;
    std::atomic<R32G32SFloat> _offsetFromWindow;

    mutable std::atomic<bool> requestLayout = true;

    /** The widget is enabled.
     */
    observable<bool> enabled = true;

    /*! Constructor for creating sub views.
     */
    Widget(Window &window, Widget *parent, float width, float height) noexcept;
    Widget(Window &window, Widget *parent, vec extent) noexcept :
        Widget(window, parent, extent.width(), extent.height()) {}

    virtual ~Widget();

    Widget(const Widget &) = delete;
    Widget &operator=(const Widget &) = delete;
    Widget(Widget &&) = delete;
    Widget &operator=(Widget &&) = delete;

    /** Create a window rectangle from left, bottom, width and height
     * Thread-safety: locks window.widgetSolverMutex
     */
    aarect makeWindowRectangle() const noexcept;

    void setMinimumWidth(float width) noexcept;
    void setMinimumHeight(float height) noexcept;
    void setMinimumExtent(float width, float height) noexcept;
    void setMinimumExtent(vec newMinimumExtent) noexcept;

    void setPreferredWidth(float width) noexcept;
    void setPreferredHeight(float height) noexcept;
    void setPreferredExtent(float width, float height) noexcept;
    void setPreferredExtent(vec newMinimumExtent) noexcept;

    void setMaximumWidth(float width) noexcept;
    void setMaximumHeight(float height) noexcept;
    void setMaximumExtent(float width, float height) noexcept;
    void setMaximumExtent(vec newMinimumExtent) noexcept;

    rhea::constraint placeBelow(Widget const &rhs, float margin=theme->margin) const noexcept;
    rhea::constraint placeAbove(Widget const &rhs, float margin=theme->margin) const noexcept;

    rhea::constraint placeLeftOf(Widget const &rhs, float margin=theme->margin) const noexcept;

    rhea::constraint placeRightOf(Widget const &rhs, float margin=theme->margin) const noexcept;

    rhea::constraint placeAtTop(float margin=theme->margin) const noexcept;

    rhea::constraint placeAtBottom(float margin=theme->margin) const noexcept;

    rhea::constraint placeLeft(float margin=theme->margin) const noexcept;

    rhea::constraint placeRight(float margin=theme->margin) const noexcept;


    [[nodiscard]] vec extent() const noexcept {
        return static_cast<vec>(_extent.load(std::memory_order::memory_order_relaxed));
    }

    void setExtent(vec rhs) noexcept {
        _extent.store(R32G32SFloat{rhs}, std::memory_order::memory_order_relaxed);
    }

    [[nodiscard]] vec offsetFromParent() const noexcept {
        return static_cast<vec>(_offsetFromParent.load(std::memory_order::memory_order_relaxed));
    }

    void setOffsetFromParent(vec rhs) noexcept {
        _offsetFromParent.store(R32G32SFloat{rhs}, std::memory_order::memory_order_relaxed);
    }

    [[nodiscard]] vec offsetFromWindow() const noexcept {
        return static_cast<vec>(_offsetFromWindow.load(std::memory_order::memory_order_relaxed));
    }

    void setOffsetFromWindow(vec rhs) noexcept {
        _offsetFromWindow.store(R32G32SFloat{rhs}, std::memory_order::memory_order_relaxed);
    }

    /** Get the rectangle in local coordinates.
     *
     * Thread safety: reads atomics.
     */
    [[nodiscard]] aarect rectangle() const noexcept {
        return aarect{extent()};      
    }

    /** Get the rectangle in window coordinates.
    *
    * Thread safety: reads atomics.
    */
    [[nodiscard]] aarect windowRectangle() const noexcept {
        return {
            vec::origin() + offsetFromWindow(),
            vec{extent()}
        };
    }

    [[nodiscard]] float baseHeight() const noexcept;

    /** Get the clipping-rectangle in window coordinates.
    *
    * Thread safety: calls windowRectangle()
    */
    [[nodiscard]] aarect clippingRectangle() const noexcept {
        return expand(windowRectangle(), Theme::margin);
    }

    [[nodiscard]] GUIDevice *device() const noexcept;

    /** Find the widget that is under the mouse cursor.
     *
     * Thread safety: locks.
     */
    [[nodiscard]] virtual HitBox hitBoxTest(vec position) const noexcept { return {}; }

    /** Check if the widget will accept keyboard focus.
     *
     * Thread safety: reads atomics.
     */
    [[nodiscard]] virtual bool acceptsFocus() const noexcept {
        return false;
    }

    /** Get nesting level used for selecting colors for the widget.
    */
    [[nodiscard]] ssize_t nestingLevel() noexcept {
        return numeric_cast<ssize_t>(elevation);
    }

    /** Get z value for compositing order.
    */
    [[nodiscard]] float z() noexcept {
        return elevation * 0.01f;
    }

    

    /** Layout the widget.
     * super::layout() should be called at start of the overriden function.
     *
     * This function will call needLayout().
     *
     * Thread safety: locks, must be called from render-thread
     *
     * @param displayTimePoint The time point when the widget will be shown on the screen.
     * @param forceLayout Force the layout of this widget and its children
     * @return True if this widget or its children have been laid out.
     *         In the overriden function use this value to determine if it should lay out.
     */
    virtual bool layout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept;

    /** Draw widget.
    *
    * The overriding function should call the base class's draw(), the place
    * where the call this function will determine the order of the vertices into
    * each buffer. This is important when needing to do the painters algorithm
    * for alpha-compositing. However the pipelines are always drawn in the same
    * order.
    *
    * Thread safety: locks, must be called from render-thread
    */
    virtual void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept {}

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
    /** Request if the widget needs to be layed out.
    * This function will be called for each widget on each frame.
    * Therefor it is important to optimize this function.
    *
    * Thread safety: reads atomics, must be called from render-thread.
    *
    * @return True if the widget needs a layout
    */
    [[nodiscard]] virtual bool needLayout(hires_utc_clock::time_point displayTimePoint) noexcept;
};

inline Widget * const foundWidgetPtr = reinterpret_cast<Widget *>(1);

}
