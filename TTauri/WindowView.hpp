#pragma once

#include "View.hpp"

namespace TTauri {

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
