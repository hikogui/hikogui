//
//  Frame.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "BackingPipeline.hpp"
#include <limits>
#include <memory>
#include <vector>

namespace TTauri {
namespace GUI {

class Window;
class Instance;

/*! View of a widget.
 * A view contains the dynamic data for a Widget. It is often accompanied with a Backing
 * which contains that static data of an Widget and the drawing code. Backings are shared
 * between Views.
 */
class View : public std::enable_shared_from_this<View>, public BackingPipeline::Delegate {
public:
    //! Convenient reference to the Window.
    std::weak_ptr<Window> window;

    std::weak_ptr<View> parent;

    std::vector<std::shared_ptr<View>> children;

    //! Location of the frame compared to the parent-frame.
    glm::vec3 position = {0.0, 0.0, 0.0};
    glm::vec3 extent = {0.0, 0.0, 0.0};

    /*! Constructor for creating subviews.
     */
    View();

    virtual ~View();

    virtual void setParent(const std::shared_ptr<View> &parent);
    virtual void setRectangle(glm::vec3 position, glm::vec3 extent);

    virtual void add(std::shared_ptr<View> view);

    std::shared_ptr<Device> device();
    
    template <typename T>
    std::shared_ptr<T> device();

    virtual size_t BackingPipelineRender(BackingPipeline::Vertex *vertices, size_t offset, size_t size);
};

}}
