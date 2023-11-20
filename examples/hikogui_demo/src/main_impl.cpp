// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/hikogui.hpp"
#include "hikogui/crt.hpp"
#include <memory>
#include <stacktrace>

//import hikogui;

class my_preferences : public hi::preferences {
public:
    hi::observer<std::string> audio_output_device_id;
    hi::observer<bool> audio_output_exclusive;
    hi::observer<double> audio_output_sample_rate;
    hi::observer<hi::speaker_mapping> audio_output_speaker_mapping;

    hi::observer<std::string> audio_input_device_id;
    hi::observer<bool> audio_input_exclusive;
    hi::observer<double> audio_input_sample_rate;
    hi::observer<hi::speaker_mapping> audio_input_speaker_mapping;

    hi::observer<int> tab_index = 1;
    hi::observer<bool> toggle_value;
    hi::observer<int> radio_value = 0;
    hi::observer<std::vector<std::pair<std::string, hi::label>>> _audio_device_list;

    hi::observer<std::string> selected_theme;

    my_preferences(std::filesystem::path path) : hi::preferences(std::move(path))
    {
        add("audio_output_device_id", audio_output_device_id);
        add("audio_output_exclusive", audio_output_exclusive);
        add("audio_output_sample_rate", audio_output_sample_rate);
        add("audio_input_device_id", audio_input_device_id);
        add("audio_input_exclusive", audio_input_exclusive);
        add("audio_input_sample_rate", audio_input_sample_rate);
        add("tab_index", tab_index);
        add("toggle_value", toggle_value);
        add("radio_value", radio_value);
        add("selected_theme", selected_theme);
    }
};

hi::scoped_task<> init_audio_tab(hi::grid_widget& grid, my_preferences& preferences) noexcept
{
    using namespace hi;

    grid.emplace<label_widget>("A1", txt("Input audio device:"), alignment::top_right());
    auto& input_config = grid.emplace<audio_device_widget>("B1");
    input_config.direction = audio_direction::input;
    input_config.device_id = preferences.audio_input_device_id;

    grid.emplace<label_widget>("A2", txt("Output audio device:"), alignment::top_right());
    auto& output_config = grid.emplace<audio_device_widget>("B2");
    output_config.direction = audio_direction::output;
    output_config.device_id = preferences.audio_output_device_id;

    co_await std::suspend_always{};
}

hi::scoped_task<> init_theme_tab(hi::grid_widget& grid, my_preferences& preferences) noexcept
{
    using namespace hi;

    hi::observer<std::vector<std::pair<std::string, hi::label>>> theme_list;

    {
        auto proxy = theme_list.copy();
        for (hilet& name : theme_names()) {
            proxy->emplace_back(name, txt(name));
        }
    }

    grid.emplace<label_widget>("A1", txt("Theme:"), alignment::top_right());
    grid.emplace<selection_widget>("B1", preferences.selected_theme, theme_list);

    co_await std::suspend_always{};
}

hi::scoped_task<> init_license_tab(hi::grid_widget& grid, my_preferences& preferences) noexcept
{
    using namespace hi;

    grid.emplace<label_widget>(
        "A1",
        txt("This is a [he-IL]\xd7\x9c\xd6\xb0\xd7\x9e\xd6\xb7\xd7\xaa\xd6\xb5\xd7\x92[.].\nAnd another sentence. One more:"),
        alignment::top_right());
    grid.emplace<toggle_with_label_widget>("B1", preferences.toggle_value, txt("true"), txt("false"), txt("other"));

    grid.emplace<label_widget>("A2", txt("These is a disabled checkbox:"), alignment::top_right());
    auto& checkbox2 = grid.emplace<checkbox_with_label_widget>(
        "B2", preferences.radio_value, 2, txt("Checkbox, with a pretty large label."), txt("off"), txt("other"));
    //auto& checkbox2 = grid.emplace<checkbox_widget>("B2", preferences.radio_value, 2);

    grid.emplace<label_widget>("A3", txt("These are radio buttons:"), alignment::top_right());
    grid.emplace<radio_button_with_label_widget>("B3", preferences.radio_value, 0, txt("Radio 1"));
    grid.emplace<radio_button_with_label_widget>("B4", preferences.radio_value, 1, txt("Radio 2 (on)"), txt("Radio 2 (off)"));
    grid.emplace<radio_button_with_label_widget>("B5", preferences.radio_value, 2, txt("Radio 3"));

    auto option_list = std::vector{
        std::pair{0, label{txt("first")}},
        std::pair{1, label{txt("second")}},
        std::pair{2, label{txt("third")}},
        std::pair{3, label{txt("four")}},
        std::pair{4, label{txt("five")}},
        std::pair{5, label{txt("six")}},
        std::pair{6, label{txt("seven")}}};

    grid.emplace<label_widget>("A6", txt("This is a selection box at the bottom:"), alignment::top_right());
    auto& selection3 = grid.emplace<selection_widget>("B6", preferences.radio_value, option_list);

    grid.emplace<label_widget>("A7", txt("Sample Rate:"), alignment::top_right());
    grid.emplace<text_field_widget>("B7", preferences.audio_output_sample_rate);

    auto toggle_value_cbt = preferences.toggle_value.subscribe(
        [&](bool value) {
            checkbox2.mode = value ? widget_mode::enabled : widget_mode::disabled;
            selection3.mode = value ? widget_mode::enabled : widget_mode::disabled;
        },
        callback_flags::main);

    grid.emplace<label_widget>("A8:B8", txt("This is large number locale formatted: {:L}", 1234.56));


    co_await std::suspend_always{};
}

hi::task<> preferences_window(my_preferences& preferences)
{
    using namespace hi;

    auto window_label = label{png::load(URL{"resource:hikogui_demo.png"}), txt("Preferences")};
    auto top = std::make_unique<window_widget>(window_label);

    top->toolbar().emplace<toolbar_tab_button_widget>(preferences.tab_index, 0, label{elusive_icon::Speaker, txt("Audio")});
    top->toolbar().emplace<toolbar_tab_button_widget>(preferences.tab_index, 1, label{elusive_icon::Key, txt("License")});
    top->toolbar().emplace<toolbar_tab_button_widget>(preferences.tab_index, 2, label{elusive_icon::Brush, txt("Theme")});

    auto& tabs = top->content().emplace<tab_widget>("A1", preferences.tab_index);
    auto& audio_tab_grid = tabs.emplace<grid_widget>(0);
    auto& license_tab_grid = tabs.emplace<scroll_widget<axis::both>>(1).emplace<grid_widget>();
    auto& theme_tab_grid = tabs.emplace<grid_widget>(2);

    auto audio_tab = init_audio_tab(audio_tab_grid, preferences);
    auto license_tab = init_license_tab(license_tab_grid, preferences);
    auto theme_tab = init_theme_tab(theme_tab_grid, preferences);

    auto window = gui_window{std::move(top)};

    co_await window.closing;
}

inline size_t target = 0;

hi::task<> main_window(my_preferences& preferences)
{
    using namespace hi;

    auto window_label = label{png::load(URL{"resource:hikogui_demo.png"}), txt("HikoGUI demo")};
    auto top = std::make_unique<window_widget>(window_label);

    auto preferences_label = label{elusive_icon::Wrench, txt("Preferences")};
    hilet& preferences_button = top->toolbar().emplace<hi::toolbar_button_widget>(preferences_label);

    top->content().emplace_bottom<toggle_with_label_widget>(preferences.toggle_value);
    hilet& hello_world_button = top->content().emplace_bottom<momentary_button_widget>(txt("Hello world AV"));
    hilet& vma_dump_button = top->content().emplace_bottom<momentary_button_widget>(txt("vma\ncalculate stats"));
    hilet& abort_button = top->content().emplace_bottom<momentary_button_widget>(txt("abort"));
    hilet& break_button = top->content().emplace_bottom<momentary_button_widget>(txt("break"));

    auto window = gui_window{std::move(top)}; 

    while (true) {
        hilet result = co_await when_any(
            preferences_button,
            vma_dump_button,
            hello_world_button,
            abort_button,
            break_button,
            preferences.toggle_value,
            window.closing);

        switch (result.index()) {
        case 0:
            preferences_window(preferences);
            break;
        case 1:
            gfx_system::global().log_memory_usage();
            break;
        case 2:
            hi_log_info("Hello World");
            break;
        case 3:
            //target = 1 / (result.index() - 3);
            hi_assert_abort("my abort");
            break;
        case 4:
            hi_debug_break();
            break;
        case 5:
            hi_log_info("Toggle value {}", get<bool>(result));
            break;
        case 6:
            co_return;
        default:
            hi_no_default();
        }
    }
}

int hi_main(int argc, char *argv[])
{
    using namespace hi;

    set_application_name("HikoGUI Demo");
    set_application_vendor("HikoGUI");
    set_application_version({1, 0, 0});

    // Start the logger system, so logging is done asynchronously.
    log::start_subsystem(global_state_type::log_level_info);
    start_render_doc();

    auto preferences = my_preferences(preferences_file());

    theme_book::global().selected_theme = preferences.selected_theme;

    main_window(preferences);
    return loop::main().resume();
}

// extern "C" const char *__asan_default_options() {
//    return "help=1:log_path=asan.log";
//}
