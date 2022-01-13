
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

auto create_main_window(tt::gui_system &gui, std::shared_ptr<my_preferences_window_controller> preferences_controller) noexcept
{
    using namespace tt;

    auto window_label = label{URL{"resource:ttauri_demo.png"}, l10n("TTauri demo")};
    auto &main_window = gui.make_window(window_label);

    auto &preferences_button =
        main_window.toolbar().make_widget<toolbar_button_widget>(label{elusive_icon::Wrench, l10n("Preferences")});
    auto &column = main_window.content().make_widget<column_widget>("A1");
    column.make_widget<momentary_button_widget>(l10n("Hello \u4e16\u754c"));
    column.make_widget<momentary_button_widget>(l10n("Hello world"));
    auto &vma_dump_button = column.make_widget<momentary_button_widget>(l10n("vma\ncalculate stats"));


    return std::array{
        preferences_button.subscribe([&gui, preferences_controller] {
            gui.make_window(
                label{icon{URL{"resource:ttauri_demo.png"}}, l10n("TTauri Demo - Preferences")}, preferences_controller);
        }),
        vma_dump_button.subscribe([&gui]{
            gui.gfx->log_memory_usage();
        })
    };
}

int tt_main(int argc, char *argv[])
{
    // Set the version at the very beginning, because file system paths depend on it.
    auto &m = tt::metadata::application();
    m.name = "ttauri-demo";
    m.display_name = "TTauri Demo";
    m.vendor = tt::metadata::library().vendor;
    m.version = tt::metadata::library().version;

    // Start the logger system, so logging is done asynchronously.
    tt::log::start_subsystem(tt::global_state_type::log_level_info);
    tt::time_stamp_count::start_subsystem();

    // Startup renderdoc for debugging
    auto render_doc = tt::RenderDoc();

    auto preferences = tt::preferences(tt::URL::urlFromApplicationPreferencesFile());
    auto preferences_controller = std::make_shared<my_preferences_window_controller>(preferences);

    auto gui = tt::gui_system::make_unique();
    auto audio = tt::audio_system::make_unique(gui->event_queue(), preferences_controller);

    auto callback = create_main_window(*gui, preferences_controller);

    return gui->loop();
}

// extern "C" const char *__asan_default_options() {
//    return "help=1:log_path=asan.log";
//}
