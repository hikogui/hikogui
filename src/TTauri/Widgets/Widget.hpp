// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Window_forward.hpp"
#include "TTauri/GUI/GUIDevice_forward.hpp"
#include "TTauri/GUI/MouseEvent.hpp"
#include "TTauri/GUI/HitBox.hpp"
#include "TTauri/GUI/KeyboardEvent.hpp"
#include "TTauri/GUI/Theme.hpp"
#include "TTauri/Text/ShapedText.hpp"
#include "TTauri/Foundation/attributes.hpp"
#include "TTauri/Foundation/Path.hpp"
#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"
#include "TTauri/Foundation/R32G32SFloat.hpp"
#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/pickle.hpp"
#include "TTauri/Foundation/vspan.hpp"
#include "TTauri/Foundation/utils.hpp"
#include "TTauri/Foundation/Trigger.hpp"
#include "TTauri/Foundation/cpu_utc_clock.hpp"
#include "TTauri/Foundation/observable.hpp"
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
 *         hitBoxTest(), needs()
 *     All public members should be thread-safe, for example:
 *         std::atomic and tt::observer
 *     The following methods should only be called from the render thread:
 *         needs(), layout(), layoutChildren(), draw(), 
 */
class Widget {
protected:
    mutable std::recursive_mutex mutex;

    /** Convenient reference to the Window.
    */
    Window &window;

    /** Pointer to the parent widget.
    * May be a nullptr only when this is the top level widget.
    */
    Widget *parent;

    std::vector<std::unique_ptr<Widget>> children;

    /** The content area of this widget.
    * This is a widget that contains the widgets that are added
    * by the user, as opposed to the child widgets that controls this widget.
    */
    Widget *content = nullptr;

    /** Transformation matrix from window coords to local coords.
    */
    mat fromWindowTransform;

    /** Transformation matrix from local coords to window coords.
    */
    mat toWindowTransform;

    /** The minimum size the widget should be.
    * This value could change based on the content of the widget.
    */
    vec minimumExtent;

    rhea::constraint minimumWidthConstraint;
    rhea::constraint minimumHeightConstraint;

    /** The minimum size the widget should be.
    * This value could change based on the content of the widget.
    */
    vec preferedExtent;

    rhea::constraint preferedWidthConstraint;
    rhea::constraint preferedHeightConstraint;

    /** The fixed size the widget should be.
    * 0.0f in either x or y means that direction is not fixed.
    */
    vec fixedExtent;

    rhea::constraint fixedWidthConstraint;
    rhea::constraint fixedHeightConstraint;

    /** Mouse cursor is hovering over the widget.
    */
    bool hover = false;

    /** The widget has keyboard focus.
    */
    bool focus = false;

public:
    /** Location of the frame compared to the window.
     * Thread-safety: the box is not modified by the class.
     */
    rhea::variable const left;
    rhea::variable const bottom;
    rhea::variable const width;
    rhea::variable const height;

    rhea::linear_expression const right = left + width;
    rhea::linear_expression const centre = left + width * 0.5;
    rhea::linear_expression const top = bottom + height;
    rhea::linear_expression const middle = bottom + height * 0.5;

    float elevation;

    std::atomic<R32G32SFloat> _extent;
    std::atomic<R32G32SFloat> _offsetFromParent;
    std::atomic<R32G32SFloat> _offsetFromWindow;

    mutable std::atomic<bool> forceLayout = true;
    mutable std::atomic<bool> forceRedraw = true;

    /** The widget is enabled.
     */
    observable<bool> enabled = true;

    /*! Constructor for creating sub views.
     */
    Widget(Window &window, Widget *parent, vec defaultExtent) noexcept;
    virtual ~Widget();

    Widget(const Widget &) = delete;
    Widget &operator=(const Widget &) = delete;
    Widget(Widget &&) = delete;
    Widget &operator=(Widget &&) = delete;

    /** Add a widget directly to this widget.
     * Thread safety: locks.
     */
    virtual Widget &addWidget(Alignment alignment, std::unique_ptr<Widget> childWidget) noexcept;

    /** Add a widget directly to this widget.
     *
     * Thread safety: calls _addWidget
     */
    template<typename T, typename... Args>
    T &makeWidgetDirectly(Args &&... args) {
        return static_cast<T &>(
            addWidget(Alignment::TopLeft, std::make_unique<T>(window, this, std::forward<Args>(args)...))
        );
    }

    /** Add a widget directly to this widget.
    *
    * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
    */
    template<typename T, typename... Args>
    T &makeWidget(Args &&... args) {
        if (content != nullptr) {
            return content->makeWidget<T>(std::forward<Args>(args)...);
        } else {
            return makeWidgetDirectly<T>(std::forward<Args>(args)...);
        }
    }

    /** Add a widget directly to this widget.
    *
    * Thread safety: calls _addWidget
    */
    template<typename T, typename... Args>
    T &makeAlignedWidgetDirectly(Alignment alignement, Args &&... args) {
        return static_cast<T &>(
            addWidget(alignement, std::make_unique<T>(window, this, std::forward<Args>(args)...))
        );
    }

    /** Add a widget directly to this widget.
    *
    * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
    */
    template<typename T, typename... Args>
    T &makeAlignedWidget(Alignment alignment, Args &&... args) {
        if (content != nullptr) {
            return content->makeAlignedWidget<T>(alignment, std::forward<Args>(args)...);
        } else {
            return makeAlignedWidgetDirectly<T>(alignment, std::forward<Args>(args)...);
        }
    }

    /** Create a window rectangle from left, bottom, width and height
     * Thread-safety: locks window.widgetSolverMutex
     */
    aarect makeWindowRectangle() const noexcept;

    void setMinimumExtent(vec newMinimumExtent) noexcept;
    void setMinimumExtent(float width, float height) noexcept;

    void setPreferedExtent(vec newPreferedExtent) noexcept;

    void setFixedExtent(vec newFixedExtent) noexcept;
    void setFixedHeight(float height) noexcept;
    void setFixedWidth(float width) noexcept;

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
    [[nodiscard]] virtual HitBox hitBoxTest(vec position) const noexcept;

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

    /** Request the needs of the widget.
     * This function will be called for each widget on each frame.
     * Therefor it is important to optimize this function.
     *
     * Thread safety: reads atomics, must be called from render-thread.
     *
     * @return If the widgets needs to be redrawn or layed out on this frame.
     */
    [[nodiscard]] virtual int needs(hires_utc_clock::time_point displayTimePoint) noexcept;

    /** Layout the widget.
     * super::layout() should be called at start of the overriden function.
     *
     * Thread safety: locks, must be called from render-thread
     */
    virtual void layout(hires_utc_clock::time_point displayTimePoint) noexcept;

    /** Layout children of this widget.
    *
    * Thread safety: locks, must be called from render-thread
    *
    * @param force Force the layout of the widget.
    */
    [[nodiscard]] int layoutChildren(hires_utc_clock::time_point displayTimePoint, bool force) noexcept;

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
    virtual void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept;

    /** Handle command.
     *
     * Thread safety: locks
     */
    virtual void handleCommand(string_ltag command) noexcept;

    /*! Handle mouse event.
    * Called by the operating system to show the position and button state of the mouse.
    * This is called very often so it must be made efficient.
    * This function is also used to determine the mouse cursor.
    *
    * Thread safety: locks
    */
    virtual void handleMouseEvent(MouseEvent const &event) noexcept {
        auto lock = std::scoped_lock(mutex);

        if (event.type == MouseEvent::Type::Entered) {
            hover = true;
            forceRedraw = true;
        } else if (event.type == MouseEvent::Type::Exited) {
            hover = false;
            forceRedraw = true;
        }
    }

    [[nodiscard]] std::vector<Widget *> childPointers(bool reverse) const noexcept;

    [[nodiscard]] virtual Widget *nextKeyboardWidget(Widget const *currentKeyboardWidget, bool reverse) const noexcept;

    /*! Handle keyboard event.
    * Called by the operating system when editing text, or entering special keys
    *
    * Thread safety: locks
    */
    virtual void handleKeyboardEvent(KeyboardEvent const &event) noexcept {
        auto lock = std::scoped_lock(mutex);

        switch (event.type) {
        case KeyboardEvent::Type::Entered:
            focus = true;
            forceRedraw = true;
            break;

        case KeyboardEvent::Type::Exited:
            focus = false;
            forceRedraw = true;
            break;

        case KeyboardEvent::Type::Key:
            for (ttlet command : event.getCommands()) {
                handleCommand(command);
            }
            break;

        default:;
        }
    }
};

inline Widget * const foundWidgetPtr = reinterpret_cast<Widget *>(1);

}
