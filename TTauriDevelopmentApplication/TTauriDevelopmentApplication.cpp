
#include "pch.h"

#include "TTauri/utils.hpp"
#include "TTauri/Application_win32.hpp"
#include "TTauri/GUI/ImageView.hpp"
#include "TTauri/GUI/Instance_vulkan_win32.hpp"
#include "TTauri/Logging.hpp"

#include <vulkan/vulkan.hpp>

#include <Windows.h>

#include <boost/filesystem.hpp>

#include <memory>
#include <vector>

using namespace std;
using namespace TTauri;

class MyWindowDelegate : public GUI::Window::Delegate {
public:
    void creatingWindow(const std::shared_ptr<GUI::Window> &window) override
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
    void startingLoop() override
    {
        auto myWindowDelegate = TTauri::make_shared<MyWindowDelegate>();

        get_singleton<GUI::Instance>()->createWindow(myWindowDelegate, "Hello World");
    }
};

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
    auto myApplicationDelegate = TTauri::make_shared<MyApplicationDelegate>();

    make_singleton<Application_win32>(myApplicationDelegate, hInstance, hPrevInstance, pCmdLine, nCmdShow);
    make_singleton<GUI::Instance_vulkan_win32>();

    return get_singleton<Application>()->loop();
}
