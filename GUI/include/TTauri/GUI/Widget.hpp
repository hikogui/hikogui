// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/BoxModel.hpp"
#include "TTauri/GUI/PipelineImage_Backing.hpp"
#include "TTauri/GUI/Window_forward.hpp"
#include "TTauri/GUI/Device_forward.hpp"
#include "TTauri/GUI/Mouse.hpp"
#include "TTauri/GUI/KeyboardEvent.hpp"
#include "TTauri/Text/ShapedText.hpp"
#include "TTauri/Foundation/attributes.hpp"
#include "TTauri/Foundation/Path.hpp"
#include "TTauri/Foundation/wsRGBA.hpp"
#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"
#include "TTauri/Foundation/URL.hpp"
#include <TTauri/Foundation/pickle.hpp>
#include <TTauri/Foundation/vspan.hpp>
#include <TTauri/Foundation/utils.hpp>
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
namespace TTauri::GUI::PipelineBox {
struct Vertex;
}

namespace TTauri::GUI::Widgets {

/*! View of a widget.
 * A view contains the dynamic data for a Widget. It is often accompanied with a Backing
 * which contains that static data of an Widget and the drawing code. Backings are shared
 * between Views.
 */
class Widget {
private:
    /** Incremented when the widget's state was modified.
    */
    std::atomic<uint64_t> modificationRequest = 1;

    /** Copied from modificationRequest before processing the modificationRequest.
    */
    uint64_t modificationVersion = 0;


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

    //! Rectangle, extracted from the box
    rect rectangle; 

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

    [[nodiscard]] Device *device() const noexcept;

    /** Should be called after the internal state of the widget was modified.
    */
    force_inline bool setModified(bool x=true) noexcept {
        if (x) {
            modificationRequest.fetch_add(1, std::memory_order::memory_order_relaxed);
        }
        return x;
    }

    /** Update and place vertices.
    *
    * This function is called by external functions.
    * @see handleMouseEvent
    */
    [[nodiscard]] bool _updateAndPlaceVertices(
        vspan<PipelineFlat::Vertex> &flat_vertices,
        vspan<PipelineBox::Vertex> &box_vertices,
        vspan<PipelineImage::Vertex> &image_vertices,
        vspan<PipelineSDF::Vertex> &sdf_vertices
    ) noexcept {
        auto _modified = modified();
        _modified |= assign_and_compare(rectangle, box.currentRectangle());
        unsetModified();
        return setModified(updateAndPlaceVertices(_modified, flat_vertices, box_vertices, image_vertices, sdf_vertices));
    }

    /** Handle mouse event.
    * This function is called by external functions.
    * @see handleMouseEvent
    */
    [[nodiscard]] virtual bool _handleMouseEvent(MouseEvent const &event) noexcept {
        return setModified(handleMouseEvent(event));
    }

    [[nodiscard]] virtual bool _handleKeyboardEvent(KeyboardEvent const &event) noexcept {
        return setModified(handleKeyboardEvent(event));
    }

    [[nodiscard]] virtual HitBox hitBoxTest(vec position) const noexcept;

protected:
    /*! Handle mouse event.
    * Called by the operating system to show the position and button state of the mouse.
    * This is called very often so it must be made efficient.
    * This function is also used to determine the mouse cursor.
    *
    * @return true when a widgets wants to change its appearance in the next frame.
    */
    [[nodiscard]] virtual bool handleMouseEvent(MouseEvent const &event) noexcept;

    /*! Handle keyboard event.
    * Called by the operating system when editing text, or entering special keys
    *
    * @return true when a widgets wants to change its appearance in the next frame.
    */
    [[nodiscard]] virtual bool handleKeyboardEvent(KeyboardEvent const &event) noexcept;

    /** Update and place vertices.
    *
    * The overriding function should call the base class's update(), the place
    * where the call this function will determine the order of the vertices into
    * each buffer. This is important when needing to do the painters algorithm
    * for alpha-compositing. However the pipelines are always drawn in the same
    * order.
    *
    * @param modified The data in the widget has been modified.
    * @param flat_vertices Vertex buffer of the flat-pipeline.
    * @param box_vertices Vertex buffer of the box-pipeline.
    * @param image_vertices Vertex buffer of the image-pipeline.
    * @param sdf_vertices Vertex buffer of the sdf-pipeline.
    * @return true when a widgets is currently running an animation and wants
    *         to change its appearance in the next frame.
    */
    [[nodiscard]] virtual bool updateAndPlaceVertices(
        bool modified,
        vspan<PipelineFlat::Vertex> &flat_vertices,
        vspan<PipelineBox::Vertex> &box_vertices,
        vspan<PipelineImage::Vertex> &image_vertices,
        vspan<PipelineSDF::Vertex> &sdf_vertices
    ) noexcept;

private:

    force_inline void unsetModified(void) noexcept {
        modificationVersion = modificationRequest.load(std::memory_order::memory_order_acquire);
    }

    [[nodiscard]] force_inline bool modified() noexcept {
        return modificationVersion != modificationRequest.load(std::memory_order::memory_order_acquire);
    }
};

}
