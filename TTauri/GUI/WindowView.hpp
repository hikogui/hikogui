#pragma once

#include "View.hpp"

namespace TTauri {
namespace GUI {

class Window;

class WindowView : public View
{
public:
    enum class Type {
        WINDOW,
        PANEL,
        FULLSCREEN,
    };

    WindowView(Window *window);

    virtual ~WindowView();
};

}
}
