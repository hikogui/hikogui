#pragma once

#include "View.hpp"

namespace TTauri {
namespace GUI {

class Window;

class WindowView : public View {
public:
    enum class Type {
        WINDOW,
        PANEL,
        FULLSCREEN,
    };

    WindowView(const std::shared_ptr<Window> &window);
    ~WindowView(){};

    WindowView(const WindowView &) = delete;
    WindowView &operator=(const WindowView &) = delete;
    WindowView(WindowView &&) = delete;
    WindowView &operator=(WindowView &&) = delete;
};

}}
