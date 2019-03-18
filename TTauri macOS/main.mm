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

using namespace std;
using namespace TTauri;

class MyWindowDelegate : public GUI::Window::Delegate {
public:
    virtual void creatingWindow(const std::shared_ptr<GUI::Window> &window)
    {
        auto view1 = TTauri::make_shared<GUI::ImageView>(get_singleton<Application>()->resourceDir / "lena.png");
        view1->setRectangle({ 100.0, 100.0, 1.0 }, { 200.0, 100.0, 0.0 });
        window->view->add(view1);

        auto view2 = TTauri::make_shared<GUI::ImageView>(get_singleton<Application>()->resourceDir / "lena.png");
        view2->setRectangle({ 200.0, 200.0, 1.0 }, { 200.0, 100.0, 0.0 });
        window->view->add(view2);
    }
};

class MyApplicationDelegate : public Application::Delegate {
public:
    virtual void startingLoop()
    {
        auto myWindowDelegate = TTauri::make_shared<MyWindowDelegate>();

        get_singleton<GUI::Instance>()->createWindow(myWindowDelegate, "Hello World");
    }
};

int main(int argc, const char **argv)
{
    int result;

    @autoreleasepool {
        auto myApplicationDelegate = TTauri::make_shared<MyApplicationDelegate>();

        TTauri::make_shared<Application_macos>(myApplicationDelegate, argc, argv);
        TTauri::make_shared<GUI::Instance_vulkan_macos>();

        result = get_singleton<Application>()->loop();
    }

    return result;
}

