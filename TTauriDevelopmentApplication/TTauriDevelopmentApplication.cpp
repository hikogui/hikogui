
#include "TTauri/Widgets/widgets.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/WindowDelegate.hpp"
#include "TTauri/GUI/Instance.hpp"
#include "TTauri/Audio/globals.hpp"
#include "TTauri/Audio/AudioSystem.hpp"
#include "TTauri/Application/Application.hpp"
#include "TTauri/Foundation/CommandLineParser.hpp"

#include <vulkan/vulkan.hpp>

#include <Windows.h>

#include <memory>
#include <vector>

using namespace std;
using namespace TTauri;
using namespace TTauri::Audio;
using namespace TTauri;
using namespace TTauri;
using namespace TTauri;

class MyWindowDelegate : public WindowDelegate {
public:
    void openingWindow(Window &window) override
    {
        auto &button1 = window.addWidget<ButtonWidget>(u8"Hello \u4e16\u754c");
        button1.placeLeft();
        button1.placeAtTop();
        button1.placeAtBottom();

        auto &button2 = window.addWidget<ButtonWidget>(u8"Hello world");
        button2.placeRight();
        button2.placeAtTop();
        button2.placeRightOf(button1);
        button2.placeAtBottom();
    }

    void closingWindow(const Window &window) override
    {
        LOG_INFO("Window being destroyed.");
    }
};

class MyApplicationDelegate : public ApplicationDelegate {
public:
    std::string applicationName() const noexcept override {
        return "TTauri Development Application";
    }

    datum configuration(std::vector<std::string> arguments) const noexcept override {
        auto parser = CommandLineParser(
            "TTauri development application."
        );
        parser.add(
            "help"s,
            datum_type_t::Boolean,
            "This help message"s
        );
        parser.add(
            "log-level"s,
            datum_type_t::Integer,
            "Set the log level, possible values 'debug', 'info', 'audit', 'warning', 'error', 'critical' or 'fatal'."s,
            command_line_argument_to_log_level
        );

        auto default_configuration = datum{datum::map{}};
        default_configuration["help"] = false;
        default_configuration["log-level"] = static_cast<uint8_t>(log_level::Warning);

        let command_line_configuration = parser.parse(arguments);
        auto configuration = deep_merge(default_configuration, command_line_configuration);

        if (parser.has_error() || configuration["help"]) {
            parser.print_help();
            std::exit(parser.has_error() ? 2 : 0);
        }

        LOG_INFO("Configuration {}", configuration);
        return configuration;
    }

    bool startingLoop() override
    {
        auto myWindowDelegate = make_shared<MyWindowDelegate>();

        guiSystem->initialize();
        guiSystem->addWindow<Window>(myWindowDelegate, "Hello World 1");

        audioSystem->initialize();
        return true;
    }

    void lastWindowClosed() override
    {
    }

    void audioDeviceListChanged() override
    {
        LOG_INFO("MyApplicationDelegate::audioDeviceListChanged()");
    }
};

MAIN_DEFINITION
{
    auto myApplicationDelegate = make_shared<MyApplicationDelegate>();

    auto app = Application(myApplicationDelegate, MAIN_ARGUMENTS);

    return app.loop();
}
