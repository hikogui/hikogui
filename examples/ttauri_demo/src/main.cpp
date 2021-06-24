
#include "my_preferences_window_controller.hpp"
#include "my_preferences.hpp"
#include "ttauri/logger.hpp"
#include "ttauri/crt.hpp"
#include "ttauri/hires_utc_clock.hpp"
#include "ttauri/metadata.hpp"
#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/audio/audio_system.hpp"
#include "ttauri/widgets/widgets.hpp"
#include <Windows.h>
#include <memory>

auto create_main_window(my_preferences_window_controller &preferences_controller) noexcept
{
    using namespace tt;

    auto window_label = label{URL{"resource:ttauri_demo.png"}, l10n("ttauri_demo")};
    auto &main_window = gui_system::global().make_window(window_label);

    auto &preferences_button = main_window.make_toolbar_widget<toolbar_button_widget>(label{elusive_icon::Wrench, l10n("Preferences")});
    auto callback = preferences_button.subscribe([&preferences_controller]{
        gui_system::global().make_window(
            label{icon{URL{"resource:ttauri_demo.png"}}, l10n("TTauri Demo - Preferences")},
            preferences_controller
        );
    });

    auto &column = main_window.make_widget<column_layout_widget>("A1");
    column.make_widget<momentary_button_widget>(l10n("Hello \u4e16\u754c"));
    column.make_widget<momentary_button_widget>(l10n("Hello world"));
    column.make_widget<momentary_button_widget>(l10n("Hello earthlings"));

    return callback;
}

int tt_main(int argc, char *argv[])
{
    using namespace tt;

    // Set the version at the very beginning, because file system paths depend on it.
    auto version = library_metadata();
    version.name = "ttauri-demo";
    version.display_name = "TTauri Demo";
    set_application_metadata(version);

    // Start the logger system, so logging is done asynchronously.
    logger_start();

    auto preferences = my_preferences(tt::URL::urlFromExecutableDirectory() / "preferences.json");
    preferences.load();

    auto preferences_controller = my_preferences_window_controller(preferences);
    audio_system::global().set_delegate(preferences_controller);

    auto callback = create_main_window(preferences_controller);

    auto r = tt::gui_system::global().loop();
    preferences.save();
    return r;
}

//extern "C" const char *__asan_default_options() {
//    return "help=1:log_path=asan.log";
//}
