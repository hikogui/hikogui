
#include "my_preferences_window_controller.hpp"
#include "ttauri/log.hpp"
#include "ttauri/crt.hpp"
#include "ttauri/time_stamp_count.hpp"
#include "ttauri/metadata.hpp"
#include "ttauri/GFX/RenderDoc.hpp"
#include "ttauri/GFX/gfx_system.hpp"
#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/audio/audio_system.hpp"
#include "ttauri/widgets/toolbar_button_widget.hpp"
#include "ttauri/widgets/momentary_button_widget.hpp"
#include "ttauri/widgets/row_column_widget.hpp"
#include <Windows.h>
#include <memory>

int tt_main(int argc, char *argv[])
{
    using namespace tt;

    // Set the version at the very beginning, because file system paths depend on it.
    auto &m = metadata::application();
    m.name = "ttauri-demo";
    m.display_name = "TTauri Demo";
    m.vendor = metadata::library().vendor;
    m.version = metadata::library().version;

    // Start the logger system, so logging is done asynchronously.
    log::start_subsystem(global_state_type::log_level_info);
    time_stamp_count::start_subsystem();

    // Startup renderdoc for debugging
    auto render_doc = RenderDoc();

    auto preferences = tt::preferences(URL::urlFromApplicationPreferencesFile());
    auto preferences_controller = std::make_shared<my_preferences_window_controller>(preferences);

    auto gui = gui_system::make_unique();
    auto audio = audio_system::make_unique(gui->event_queue(), preferences_controller);

    auto window_label = label{URL{"resource:ttauri_demo.png"}, l10n("TTauri demo")};
    auto main_window = gui->make_window(window_label);

    auto &preferences_button =
        main_window->toolbar().make_widget<tt::toolbar_button_widget>(label{elusive_icon::Wrench, l10n("Preferences")});

    auto &column = main_window->content().make_widget<column_widget>("A1");
    column.make_widget<momentary_button_widget>(l10n("Hello \u4e16\u754c"));
    column.make_widget<momentary_button_widget>(l10n("Hello world"));
    auto &vma_dump_button = column.make_widget<momentary_button_widget>(l10n("vma\ncalculate stats"));

    std::shared_ptr<gui_window> preferences_window = nullptr;
    std::shared_ptr<std::function<void()>> preferences_close_cb = nullptr;
    auto preferences_button_cb = preferences_button.subscribe([&] {
        if (not preferences_window) {
            preferences_window = gui->make_window(
                label{icon{URL{"resource:ttauri_demo.png"}}, l10n("TTauri Demo - Preferences")}, preferences_controller);
            preferences_close_cb = preferences_window->subscribe_close([&] {
                preferences_window.reset();
            });
        }
    });

    auto vma_dump_button_cb = vma_dump_button.subscribe([&gui] {
        gui->gfx->log_memory_usage();
    });

    auto main_close_cb = main_window->subscribe_close([&] {
        main_window.reset();
    });

    return gui->loop();
}

// extern "C" const char *__asan_default_options() {
//    return "help=1:log_path=asan.log";
//}
