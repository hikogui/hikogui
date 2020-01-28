// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/BoxModel.hpp"
#include "TTauri/GUI/PipelineFlat_Delegate.hpp"
#include "TTauri/GUI/PipelineImage_Delegate.hpp"
#include "TTauri/GUI/PipelineMSDF_Delegate.hpp"
#include "TTauri/GUI/PipelineImage_Backing.hpp"
#include "TTauri/GUI/Window_forward.hpp"
#include "TTauri/GUI/Device_forward.hpp"
#include "TTauri/GUI/Mouse.hpp"
#include "TTauri/Foundation/attributes.hpp"
#include "TTauri/Foundation/wsRGBA.hpp"
#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/geometry.hpp"
#include <limits>
#include <memory>
#include <vector>
#include <mutex>

namespace TTauri::GUI::PipelineImage {
struct Image;
struct Vertex;
}

namespace TTauri::GUI::Widgets {

/*! View of a widget.
 * A view contains the dynamic data for a Widget. It is often accompanied with a Backing
 * which contains that static data of an Widget and the drawing code. Backings are shared
 * between Views.
 */
class Widget : public PipelineImage::Delegate, public PipelineFlat::Delegate, public PipelineMSDF::Delegate {
public:
    //! Convenient reference to the Window.
    Window *window;

    Widget *parent;

    std::vector<std::unique_ptr<Widget>> children;

    Widget *currentMouseTarget = nullptr;

    //! Location of the frame compared to the window.
    BoxModel box;

    /*! current extent of the widget.
     * Calculated at the start of pipelineImagePlaceVertices, but may be
     * defered until the resizing of the window has been completed.
     * This allows for the widget to be scaled, instead of redrawn.
     */
    extent2 currentExtent;
     
    float depth = 0;

    /*! Constructor for creating subviews.
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

    void pipelineImagePlaceVertices(gsl::span<PipelineImage::Vertex> &vertices, int &offset) noexcept override;
    void pipelineFlatPlaceVertices(gsl::span<PipelineFlat::Vertex> &vertices, int &offset) noexcept override;
    void pipelineMSDFPlaceVertices(gsl::span<PipelineMSDF::Vertex> &vertices, int &offset) noexcept override;

    /*! Mouse moved.
     * Called by the operating system to show the position of the mouse.
     * This is called very often so it must be made efficient.
     * Most often this function is used to determine the mouse cursor.
     */
    virtual void handleMouseEvent(MouseEvent event) noexcept;

    virtual HitBox hitBoxTest(glm::vec2 position) const noexcept;
};

}
