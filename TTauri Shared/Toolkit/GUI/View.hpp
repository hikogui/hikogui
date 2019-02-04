//
//  Frame.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <memory>
#include <limits>
#include <vector>
#include "Rectangle.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

class Window;
class Device;

class View {
public:
    //! Convenient reference to the GUI.
    std::weak_ptr<Device> device;

    //! Convenient reference to the GUI.
    std::weak_ptr<Window> window;

    std::weak_ptr<View> parent;
    
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
    View(std::weak_ptr<View> parent);

    /*! Constructor for creating the main view of a window.
     */
    View(std::weak_ptr<Window> window);

    virtual ~View();
};

}}}
