#include "TTauri/Application_macos.hpp"
#include "TTauri/GUI/ImageView.hpp"
#include "TTauri/GUI/Instance_vulkan_macos.hpp"
#include "TTauri/Logging.hpp"

#include <vulkan/vulkan.hpp>

#include <boost/filesystem.hpp>

#include <memory>
#include <vector>

using namespace std;
using namespace TTauri;

class MyWindowDelegate : public GUI::Window::Delegate {
public:
    virtual void initialize(GUI::Window *window)
    {
        auto view1 = std::make_shared<GUI::ImageView>(Application::shared->resourceDir / "lena.png");
        view1->setRectangle({ 100.0, 100.0, 1.0 }, { 200.0, 100.0, 0.0 });
        window->view->add(view1);

        auto view2 = std::make_shared<GUI::ImageView>(Application::shared->resourceDir / "lena.png");
        view2->setRectangle({ 200.0, 200.0, 1.0 }, { 200.0, 100.0, 0.0 });
        window->view->add(view2);
    }
};

class MyApplicationDelegate : public Application::Delegate {
public:
    virtual void initialize()
    {

    }

    virtual void startingLoop()
    {
        auto myWindowDelegate = make_shared<MyWindowDelegate>();

        getShared<GUI::Instance>()->createWindow(myWindowDelegate, "Hello World");
    }
};

int main(int argc, char *argv[])
{
    int result;

    @autoreleasepool {
        auto myApplicationDelegate = std::make_shared<MyApplicationDelegate>();

        makeShared<Application_macos>(myApplicationDelegate, argc, argv);
        makeShared<GUI::Instance_vulkan_macos>();

        result = getShared<Application>()->loop();
    }

    return r;
}

