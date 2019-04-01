#include "WindowView.hpp"
#include "Window.hpp"

namespace TTauri::GUI {

using namespace std;

WindowView::WindowView(const std::weak_ptr<Window> window) :
    View()
{
    this->window = move(window);
}

}
