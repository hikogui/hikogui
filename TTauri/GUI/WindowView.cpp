#include "WindowView.hpp"
#include "Window.hpp"

namespace TTauri {
namespace GUI {

WindowView::WindowView(const std::shared_ptr<Window> &window) :
    View()
{
    this->window = window;
}

}}
