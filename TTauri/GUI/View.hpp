//
//  Frame.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "Rectangle.hpp"
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
class View {
public:
    //! Convenient reference to the Instance.
    Instance *instance;

    //! Convenient reference to the Window.
    Window *window;

    View *parent;

    std::vector<std::shared_ptr<View>> children;

    //! Location of the frame compared to the parent-frame.
    Rectangle location;

    //! Minimum size allowed for this view.
    float2 minimumSize;

    //! Maximum size allowed for this view.
    float2 maximumSize;

    //! Location of the frame compared to the window.
    Rectangle windowLocation;

    /*! Constructor for creating subviews.
     */
    View(View *parent);

    /*! Constructor for creating the main view of a window.
     */
    View(Window *window);

    virtual ~View();
};

}}
