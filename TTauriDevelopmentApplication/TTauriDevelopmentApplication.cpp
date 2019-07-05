
#include "TTauri/all.hpp"
#include "TTauri/GUI/all.hpp"
#include "TTauri/Draw/all.hpp"
#include "TTauri/GUI/Widgets/all.hpp"

#include <vulkan/vulkan.hpp>

#include <Windows.h>

#include <boost/filesystem.hpp>

#include <memory>
#include <vector>

using namespace std;
using namespace TTauri;
using namespace TTauri::Draw;
using namespace TTauri::GUI;
using namespace TTauri::GUI::Widgets;

class MyWindowDelegate : public WindowDelegate {
public:
    void openingWindow(Window &window) override
    {
        auto button1 = window.widget->addWidget<ButtonWidget>(u8"Hëllö Wörld");
        window.addConstraint(button1->box.width == 100);
        window.addConstraint(button1->box.height == 30);
        window.addConstraint(button1->box.outerLeft() == window.widget->box.left);
        window.addConstraint(button1->box.outerBottom() == window.widget->box.bottom);
        window.addConstraint(button1->box.outerTop() <= window.widget->toolbar->box.bottom);

        auto button2 = window.widget->addWidget<ButtonWidget>(u8"Foo Bar");
        window.addConstraint(button2->box.width >= 100);
        window.addConstraint(button2->box.width <= 1500);
        window.addConstraint(button2->box.height == 30);
        window.addConstraint(button2->box.outerLeft() == button1->box.right());
        window.addConstraint(button2->box.outerBottom() == window.widget->box.bottom);
        window.addConstraint(button2->box.outerRight() == window.widget->box.right());
        window.addConstraint(button2->box.outerTop() <= window.widget->toolbar->box.bottom);
    }

    void closingWindow(const Window &window) override
    {
        LOG_INFO("Window being destroyed.");
    }
};

class MyApplicationDelegate : public ApplicationDelegate {
public:
    void startingLoop() override
    {
        auto myWindowDelegate = make_shared<MyWindowDelegate>();

        instance->addWindow<Window>(myWindowDelegate, "Hello World 1");
        //GUI::instance->createWindow(myWindowDelegate, "Hello World 2");
    }

    void lastWindowClosed() override
    {
    }
};

#include "TTauri/Draw/TrueTypeParser.hpp"

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
    auto myApplicationDelegate = make_shared<MyApplicationDelegate>();

    singleton<Application>->initialize(myApplicationDelegate, hInstance, hPrevInstance, pCmdLine, nCmdShow);

    fonts = std::make_unique<Fonts>();
    let font = parseTrueTypeFile(std::filesystem::path("../TTauri/Draw/TestFiles/Roboto-Regular.ttf"));

    instance = std::make_unique<Instance>();
    instance->initialize();

    return singleton<Application>->loop();
}
