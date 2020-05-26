// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/BoxModel.hpp"
#include "TTauri/GUI/PipelineImage_Backing.hpp"
#include "TTauri/GUI/Window_forward.hpp"
#include "TTauri/GUI/Device_forward.hpp"
#include "TTauri/GUI/MouseEvent.hpp"
#include "TTauri/GUI/HitBox.hpp"
#include "TTauri/GUI/KeyboardEvent.hpp"
#include "TTauri/GUI/Theme.hpp"
#include "TTauri/Text/ShapedText.hpp"
#include "TTauri/Foundation/attributes.hpp"
#include "TTauri/Foundation/Path.hpp"
#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"
#include "TTauri/Foundation/URL.hpp"
#include <TTauri/Foundation/pickle.hpp>
#include <TTauri/Foundation/vspan.hpp>
#include <TTauri/Foundation/utils.hpp>
#include <TTauri/Foundation/Trigger.hpp>
#include <TTauri/Foundation/cpu_utc_clock.hpp>
#include <TTauri/Foundation/observer.hpp>
#include <rhea/constraint.hpp>
#include <limits>
#include <memory>
#include <vector>
#include <mutex>
#include <typeinfo>

namespace TTauri::GUI {
class DrawContext;
}
namespace TTauri::GUI::PipelineImage {
struct Image;
struct Vertex;
}
namespace TTauri::GUI::PipelineSDF {
struct Vertex;
}
namespace TTauri::GUI::PipelineFlat {
struct Vertex;
}
namespace TTauri::GUI::PipelineBox {
struct Vertex;
}

namespace TTauri::GUI::Widgets {

enum class WidgetNeed {
    None = 0,
    Redraw = 1,
    Layout = 3 // When Layout, Redraw is implied.
};

inline WidgetNeed operator|(WidgetNeed lhs, WidgetNeed rhs) noexcept {
    return static_cast<WidgetNeed>(static_cast<int>(lhs) | static_cast<int>(rhs));
}
inline WidgetNeed &operator|=(WidgetNeed &lhs, WidgetNeed rhs) noexcept {
    lhs = lhs | rhs;
    return lhs;
}

inline bool operator<(WidgetNeed lhs, WidgetNeed rhs) noexcept {
    return static_cast<int>(lhs) < static_cast<int>(rhs);
}
inline bool operator>(WidgetNeed lhs, WidgetNeed rhs) noexcept { return rhs < lhs; }
inline bool operator<=(WidgetNeed lhs, WidgetNeed rhs) noexcept { return !(lhs > rhs); }
inline bool operator>=(WidgetNeed lhs, WidgetNeed rhs) noexcept { return !(lhs < rhs); }

/*! View of a widget.
 * A view contains the dynamic data for a Widget. It is often accompanied with a Backing
 * which contains that static data of an Widget and the drawing code. Backings are shared
 * between Views.
 */
class Widget {
public:
    /** Convenient reference to the Window.
     */
    Window &window;

    /** Pointer to the parent widget.
     * May be a nullptr only when this is the top level widget.
     */
    Widget *parent;

    mutable std::atomic<bool> forceLayout = true;
    mutable std::atomic<bool> forceRedraw = true;

    std::vector<std::unique_ptr<Widget>> children;

    /** The content area of this widget.
     * This is a widget that contains the widgets that are added
     * by the user, as opposed to the child widgets that controls this widget.
     */
    Widgets::Widget *content = nullptr;

    /** The next widget to select when pressing tab.
    */
    Widgets::Widget *nextKeyboardWidget = nullptr;

    /** The previous widget to select when pressing shift-tab.
     */
    Widgets::Widget *prevKeyboardWidget = nullptr;

    /** A key for checking if the state of the widget has changed.
     */
    std::string current_state_key;

    /** Temporary for calculation of the current_state_key.
    */
    mutable std::string next_state_key;

    //! Location of the frame compared to the window.
    BoxModel box;

    /** The rectangle of the widget within the window.
     */
    aarect windowRectangle;

    /** The rectangle to clip drawing for this widget.
     * This is the same as the windowRectangle expanded with margins.
     */
    aarect clippingRectangle;
    
    /** The rectangle of the widget.
     * The rectangle's left bottom corner is at (0, 0)
     * in the current DrawContext's coordinate system.
     */
    aarect rectangle;

    /** Offset of this widget compared to its parent.
     */
    vec offsetFromParent;

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

    float elevation = 0.0;

    /** The widget is enabled.
     */
    observer<bool> enabled = true;

    /** Mouse cursor is hovering over the widget.
     */
    bool hover = false;

    /** The widget has keyboard focus.
     */
    bool focus = false;

    /*! Constructor for creating sub views.
     */
    Widget(Window &window, Widget *parent, vec defaultExtent) noexcept;
    virtual ~Widget();

    Widget(const Widget &) = delete;
    Widget &operator=(const Widget &) = delete;
    Widget(Widget &&) = delete;
    Widget &operator=(Widget &&) = delete;

    template<typename T, typename... Args>
    T &addWidgetDirectly(Args &&... args) {
        window.forceLayout = true;
        auto widget = std::make_unique<T>(window, this, std::forward<Args>(args)...);
        let widget_ptr = widget.get();
        children.push_back(move(widget));
        ttauri_assume(widget_ptr);
        return *widget_ptr;
    }

    template<typename T, typename... Args>
    T &addWidget(Args &&... args) {
        window.forceLayout = true;
        if (content != nullptr) {
            return content->addWidget<T>(std::forward<Args>(args)...);
        } else {
            return addWidgetDirectly<T>(std::forward<Args>(args)...);
        }
    }

    void setMinimumExtent(vec newMinimumExtent) noexcept;
    void setMinimumExtent(float width, float height) noexcept;

    void setPreferedExtent(vec newPreferedExtent) noexcept;

    void setFixedExtent(vec newFixedExtent) noexcept;
    void setFixedHeight(float height) noexcept;
    void setFixedWidth(float width) noexcept;

    void placeBelow(Widget const &rhs, float margin=theme->margin) const noexcept;
    void placeAbove(Widget const &rhs, float margin=theme->margin) const noexcept;

    void placeLeftOf(Widget const &rhs, float margin=theme->margin) const noexcept;

    void placeRightOf(Widget const &rhs, float margin=theme->margin) const noexcept;

    void placeAtTop(float margin=theme->margin) const noexcept;

    void placeAtBottom(float margin=theme->margin) const noexcept;

    void placeLeft(float margin=theme->margin) const noexcept;

    void placeRight(float margin=theme->margin) const noexcept;


    [[nodiscard]] Device *device() const noexcept;

    /** Find the widget that is under the mouse cursor.
     */
    [[nodiscard]] virtual HitBox hitBoxTest(vec position) const noexcept;

    /** Check if the widget will accept keyboard focus.
     */
    [[nodiscard]] virtual bool acceptsFocus() const noexcept {
        return false;
    }

    [[nodiscard]] ssize_t nestingLevel() noexcept {
        return numeric_cast<ssize_t>(elevation);
    }

    [[nodiscard]] float z() noexcept {
        return elevation * 0.01f;
    }

    /** Request the needs of the widget.
     * This function will be called for each widget on each frame.
     * Therefor it is important to optimize this function.
     *
     * @return If the widgets needs to be redrawn or layed out on this frame.
     */
    [[nodiscard]] virtual int needs(hires_utc_clock::time_point displayTimePoint) const noexcept;

    /** Layout the widget.
     * super::layout() should be called at start of the overriden function.
     */
    virtual void layout(hires_utc_clock::time_point displayTimePoint) noexcept;

    /** Layout children of this widget.
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
    */
    virtual void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept;

    /** Handle command.
     */
    virtual void handleCommand(string_ltag command) noexcept;

    /*! Handle mouse event.
    * Called by the operating system to show the position and button state of the mouse.
    * This is called very often so it must be made efficient.
    * This function is also used to determine the mouse cursor.
    *
    */
    virtual void handleMouseEvent(MouseEvent const &event) noexcept {
        if (event.type == MouseEvent::Type::Entered) {
            hover = true;
            forceRedraw = true;
        } else if (event.type == MouseEvent::Type::Exited) {
            hover = false;
            forceRedraw = true;
        }
    }

    /*! Handle keyboard event.
    * Called by the operating system when editing text, or entering special keys
    *
    */
    virtual void handleKeyboardEvent(KeyboardEvent const &event) noexcept {
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
            for (let command : event.getCommands()) {
                handleCommand(command);
            }
            break;

        default:;
        }
    }
};



}
