#include "WindowView.hpp"
#include "Window.hpp"

namespace TTauri {

WindowView::WindowView(Window *window) :
    View()
{
    window = window;
}

WindowView::~WindowView()
{
}

}
