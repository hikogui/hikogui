// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/BoxModel.hpp"
#include "TTauri/GUI/PipelineImage_Backing.hpp"
#include "TTauri/GUI/Window_forward.hpp"
#include "TTauri/GUI/Device_forward.hpp"
#include "TTauri/GUI/Mouse.hpp"
#include "TTauri/Text/ShapedText.hpp"
#include "TTauri/Foundation/attributes.hpp"
#include "TTauri/Foundation/Path.hpp"
#include "TTauri/Foundation/wsRGBA.hpp"
#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/geometry.hpp"
#include <TTauri/Foundation/pickle.hpp>
#include <TTauri/Foundation/vspan.hpp>
#include <limits>
#include <memory>
#include <vector>
#include <mutex>
#include <typeinfo>

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

namespace TTauri::GUI::Widgets {

/*! View of a widget.
 * A view contains the dynamic data for a Widget. It is often accompanied with a Backing
 * which contains that static data of an Widget and the drawing code. Backings are shared
 * between Views.
 */
class Widget {
protected:
    mutable bool _modified = true;

public:
    //! Convenient reference to the Window.
    Window *window;

    Widget *parent;

    std::vector<std::unique_ptr<Widget>> children;

    Widget *currentMouseTarget = nullptr;

    /** A key for checking if the state of the widget has changed.
     */
    std::string current_state_key;

    /** Temporary for calculation of the current_state_key.
    */
    mutable std::string next_state_key;

    //! Location of the frame compared to the window.
    BoxModel box;
     
    float depth = 0;

    /*! Constructor for creating sub views.
     */
    Widget() noexcept;
    virtual ~Widget() {}

    Widget(const Widget &) = delete;
    Widget &operator=(const Widget &) = delete;
    Widget(Widget &&) = delete;
    Widget &operator=(Widget &&) = delete;

    virtual void setParent(Widget *parent) noexcept;

    template<typename T, typename... Args>
    T *addWidget(Args... args) noexcept {
        auto widget = std::make_unique<T>(args...);
        auto widget_ptr = widget.get();

        widget->setParent(this);

        children.push_back(move(widget));
        return widget_ptr;
    }

    Device *device() const noexcept;

    /** Check if the data in the widget is modified.
     * Overriding functions should use clearAndPickleAppend(next_state_key, *)
     * to add data to the key. Those overriding functions should call
     * the base class's isModified() at the end.
     */
    virtual bool modified() const noexcept;

    /** Update the widget before placing vertices.
     * The overriding function should call the base class's update() at the end.
     * @param modified The data in the widget has been modified.
     */
    virtual void update(
        bool modified,
        vspan<PipelineFlat::Vertex> &flat_vertices,
        vspan<PipelineImage::Vertex> &image_vertices,
        vspan<PipelineSDF::Vertex> &sdf_vertices
    ) noexcept;

    /*! Mouse moved.
     * Called by the operating system to show the position of the mouse.
     * This is called very often so it must be made efficient.
     * Most often this function is used to determine the mouse cursor.
     */
    virtual void handleMouseEvent(MouseEvent event) noexcept;

    virtual HitBox hitBoxTest(glm::vec2 position) const noexcept;
};

}
