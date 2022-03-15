
#include "ttauri/GFX/RenderDoc.hpp"
#include "ttauri/GFX/gfx_system.hpp"
#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/GUI/theme_book.hpp"
#include "ttauri/audio/audio_system.hpp"
#include "ttauri/widgets/toolbar_button_widget.hpp"
#include "ttauri/widgets/momentary_button_widget.hpp"
#include "ttauri/widgets/row_column_widget.hpp"
#include "ttauri/widgets/selection_widget.hpp"
#include "ttauri/widgets/toggle_widget.hpp"
#include "ttauri/widgets/checkbox_widget.hpp"
#include "ttauri/widgets/radio_button_widget.hpp"
#include "ttauri/widgets/text_field_widget.hpp"
#include "ttauri/widgets/tab_widget.hpp"
#include "ttauri/widgets/toolbar_tab_button_widget.hpp"
#include "ttauri/log.hpp"
#include "ttauri/crt.hpp"
#include "ttauri/time_stamp_count.hpp"
#include "ttauri/metadata.hpp"
#include "ttauri/preferences.hpp"
#include "ttauri/when_any.hpp"
#include "ttauri/task.hpp"
#include <Windows.h>
#include <memory>

// void init_audio_tab(tt::grid_widget &grid) noexcept
//{
//     using namespace tt;
//
//     grid.make_widget<label_widget>("A1", l10n("Audio device:"));
//     grid.make_widget<selection_widget>("B1", _audio_device_list, audio_output_device_id);
//
//     grid.make_widget<label_widget>("A2", l10n("Sample Rate:"));
//     grid.make_widget<text_field_widget>("B2", audio_output_sample_rate);
// }

void init_theme_tab(
    tt::grid_widget &grid,
    tt::observable<std::vector<std::pair<std::string, tt::label>>> &theme_list,
    tt::observable<std::string> &selected_theme) noexcept
{
    using namespace tt;

    {
        auto &theme_book = *grid.window.gui.theme_book;
        auto proxy = theme_list.get();
        for (ttlet &name : theme_book.theme_names()) {
            proxy->emplace_back(name, l10n{name});
        }
    }

    grid.make_widget<label_widget>("A1", l10n("Theme:"));
    grid.make_widget<selection_widget>("B1", theme_list, grid.window.gui.selected_theme);
}

void init_license_tab(tt::grid_widget &grid, tt::observable<bool> &toggle_value, tt::observable<int> &radio_value) noexcept
{
    using namespace tt;

    grid.make_widget<label_widget>(
        "A1", l10n("This is a \xd7\x9c\xd6\xb0\xd7\x9e\xd6\xb7\xd7\xaa\xd6\xb5\xd7\x92.\nAnd another sentence. One more:"));
    auto &checkbox1 = grid.make_widget<toggle_widget>("B1", toggle_value);
    checkbox1.on_label = l10n("true");
    checkbox1.off_label = l10n("false");
    checkbox1.other_label = l10n("other");

    grid.make_widget<label_widget>("A2", l10n("These is a disabled checkbox:"));
    auto &checkbox2 = grid.make_widget<checkbox_widget>("B2", radio_value, 2, 0);
    checkbox2.on_label = l10n("Checkbox, with a pretty large label.");
    checkbox2.enabled = toggle_value;

    grid.make_widget<label_widget>("A3", l10n("These are radio buttons:"));
    grid.make_widget<radio_button_widget>("B3", l10n("Radio 1"), radio_value, 0);
    grid.make_widget<radio_button_widget>("B4", l10n("Radio 2"), radio_value, 1);
    grid.make_widget<radio_button_widget>("B5", l10n("Radio 3"), radio_value, 2);

    auto option_list = std::vector{
        std::pair{0, label{l10n("first")}},
        std::pair{1, label{l10n("second")}},
        std::pair{2, label{l10n("third")}},
        std::pair{3, label{l10n("four")}},
        std::pair{4, label{l10n("five")}},
        std::pair{5, label{l10n("six")}},
        std::pair{6, label{l10n("seven")}}};
    grid.make_widget<label_widget>("A6", l10n("This is a selection box at the bottom:"));
    auto &selection3 = grid.make_widget<selection_widget>("B6", option_list, radio_value);
    selection3.enabled = toggle_value;
}

tt::task<> preferences_window(tt::gui_system &gui, tt::preferences &preferences)
{
    using namespace tt;

    observable<audio_device_id> audio_output_device_id;
    observable<bool> audio_output_exclusive;
    observable<double> audio_output_sample_rate;
    observable<speaker_mapping> audio_output_speaker_mapping;

    observable<int> tab_index = 1;
    observable<bool> toggle_value;
    observable<int> radio_value = 0;
    observable<std::vector<std::pair<tt::audio_device_id, label>>> _audio_device_list;

    observable<std::vector<std::pair<std::string, label>>> theme_list;
    observable<std::string> selected_theme;

    auto window_label = label{URL{"resource:ttauri_demo.png"}, l10n("Preferences")};
    auto window = gui.make_window(window_label);

    // window->toolbar().make_widget<toolbar_tab_button_widget>(label{elusive_icon::Speaker, l10n("Audio")}, tab_index, 0);
    window->toolbar().make_widget<toolbar_tab_button_widget>(label{elusive_icon::Key, l10n("License")}, tab_index, 1);
    window->toolbar().make_widget<toolbar_tab_button_widget>(label{elusive_icon::Brush, l10n("Theme")}, tab_index, 2);

    auto &tabs = window->content().make_widget<tab_widget>("A1", tab_index);

    // init_audio_tab(tabs.make_widget<grid_widget>(0));
    init_license_tab(tabs.make_widget<scroll_widget<axis::both, true>>(1).make_widget<grid_widget>(), toggle_value, radio_value);
    init_theme_tab(tabs.make_widget<grid_widget>(2), theme_list, selected_theme);

    co_await window->closing;
}

tt::task<> main_window(tt::gui_system &gui, tt::preferences &preferences)
{
    using namespace tt;

    auto window_label = label{URL{"resource:ttauri_demo.png"}, l10n("TTauri demo")};
    auto window = gui.make_window(window_label);

    auto preferences_label = label{elusive_icon::Wrench, l10n("Preferences")};
    ttlet &preferences_button = window->toolbar().make_widget<tt::toolbar_button_widget>(preferences_label);

    auto &column = window->content().make_widget<column_widget>("A1");
    column.make_widget<momentary_button_widget>(l10n("Hello \u4e16\u754c"));
    ttlet &hello_world_button = column.make_widget<momentary_button_widget>(l10n("Hello world"));

    ttlet &vma_dump_button = column.make_widget<momentary_button_widget>(l10n("vma\ncalculate stats"));

    while (true) {
        ttlet result = co_await when_any(preferences_button.pressed, vma_dump_button.pressed, hello_world_button.pressed, window->closing);

        if (result == preferences_button.pressed) {
            preferences_window(gui, preferences);

        } else if (result == vma_dump_button.pressed) {
            gui.gfx->log_memory_usage();

        } else if (result == hello_world_button.pressed) {
            tt_log_info("Hello World");

        } else if (result == window->closing) {
            co_return;

        } else {
            tt_no_default();
        }
    }
}

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

    auto gui = gui_system::make_unique();
    // auto audio = audio_system::make_unique(gui->event_queue());

    main_window(*gui, preferences);
    return gui->loop();
}

// extern "C" const char *__asan_default_options() {
//    return "help=1:log_path=asan.log";
//}
