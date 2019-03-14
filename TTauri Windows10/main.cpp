#include "TTauri/Application_win32.hpp"
#include "TTauri/GUI/GUI.hpp"
#include "TTauri/Logging.hpp"

#include <vulkan/vulkan.hpp>

#include <Windows.h>

#include <boost/filesystem.hpp>

#include <memory>
#include <vector>

using namespace std;
using namespace TTauri;
using namespace TTauri::GUI;


class MyWindowDelegate : public Window::Delegate {
public:
    virtual void initialize(Window *window) {
        auto view1 = std::make_shared<ImageView>(Application::shared->resourceDir / "lena.png");
        view1->setRectangle({ 100.0, 100.0, 1.0 }, { 200.0, 100.0, 0.0 });
        window->view->add(view1);

        auto view2 = std::make_shared<ImageView>(Application::shared->resourceDir / "lena.png");
        view2->setRectangle({ 200.0, 200.0, 1.0 }, { 200.0, 100.0, 0.0 });
        window->view->add(view2);
    }
};

class MyApplicationDelegate : public Application::Delegate {
public:
    virtual void initialize() {
        auto myWindowDelegate = make_shared<MyWindowDelegate>();

        Application::shared->createWindow(myWindowDelegate, "Hello World");
    }
};

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previousInstance, PWSTR commandLine, int show)
{
    auto myApplicationDelegate = std::make_shared<MyApplicationDelegate>();

    Application::shared = std::make_shared<Application_win32>(myApplicationDelegate, instance, commandLine, show);
    Application::shared->initialize();
    return Application::shared->loop();
}
