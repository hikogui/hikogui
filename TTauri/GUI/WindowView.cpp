#include "WindowView.hpp"
#include "Window.hpp"

namespace TTauri {
namespace GUI {

WindowView::WindowView(Window *window) :
    View()
{
    window = window;
}

WindowView::~WindowView()
{
}



}}
