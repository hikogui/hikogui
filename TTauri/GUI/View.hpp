//
//  Frame.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "PipelineRectanglesFromAtlas.hpp"

#include <limits>
#include <memory>
#include <vector>

namespace TTauri::GUI {

class Window;
class Instance;

/*! View of a widget.
 * A view contains the dynamic data for a Widget. It is often accompanied with a Backing
 * which contains that static data of an Widget and the drawing code. Backings are shared
 * between Views.
 */
class View : public std::enable_shared_from_this<View>, public PipelineRectanglesFromAtlas::Delegate {
public:
    //! Convenient reference to the Window.
    std::weak_ptr<Window> window;

    std::weak_ptr<View> parent;

    std::vector<std::shared_ptr<View>> children;

    //! Location of the frame compared to the parent-frame.
    glm::vec2 position = { 0.0, 0.0 };
    glm::u16vec2 extent = { 0, 0 };

    /*! Constructor for creating subviews.
     */
    View();
    virtual ~View() {}

    View(const View &) = delete;
    View &operator=(const View &) = delete;
    View(View &&) = delete;
    View &operator=(View &&) = delete;

    virtual void setParent(const std::shared_ptr<View> &parent);
    virtual void setRectangle(glm::vec2 position, u16vec2 extent);

    virtual void add(std::shared_ptr<View> view);

    template<typename T>
    std::shared_ptr<T> device()
    {
        return lock_dynamic_cast<T>(window.lock()->device);
    }

    size_t piplineRectangledFromAtlasPlaceVertices(const gsl::span<PipelineRectanglesFromAtlas::Vertex> &vertices, size_t offset) override;
};

}