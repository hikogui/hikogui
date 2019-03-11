#pragma once

#include "View.hpp"

namespace TTauri {
namespace GUI {

class Window;

class WindowView : public View
{
public:
    WindowView(Window *window);

    virtual ~WindowView();
};

}
}
