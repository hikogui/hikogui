// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/module.hpp"
#include "hikogui/GUI/gui_system.hpp"
#include "hikogui/widgets/text_widget.hpp"
#include "hikogui/widgets/radio_button_widget.hpp"
#include "hikogui/GFX/RenderDoc.hpp"
#include "hikogui/crt.hpp"
#include "hikogui/log.hpp"
#include "hikogui/loop.hpp"

using namespace hi;

int hi_main(int argc, char *argv[])
{
    auto gui = gui_system::make_unique();
    auto [window, widget] = gui->make_window<hi::window_widget<>>(tr("Label example"));

    // Start the logger system, so logging is done asynchronously.
    hi::log::start_subsystem(hi::global_state_type::log_level_info);
    hi::time_stamp_count::start_subsystem();

    // Startup renderdoc for debugging
    auto render_doc = hi::RenderDoc();

    auto latin_text = std::string(
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, "
        "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
        "quis nostrud exercitation ullamco (laboris) nisi ut aliquip ex ea commodo consequat. "
        "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
        "Excepteur sint occaecat cupidatat non proident, "
        "sunt in culpa qui officia deserunt mollit anim id est laborum.");

    auto hebrew_text = std::string(
        "\xd7\xa6\xd7\x99\xd7\x9c\xd7\x95\xd7\x9d \xd7\xaa\xd7\x97\xd7\x91\xd7\x95\xd7\xa8\xd7\x94 \xd7\xa2\xd7\x9c \xd7"
        "\xa2\xd7\x96\xd7\x94, \xd7\x90\xd7\x9d \xd7\x9e\xd7\x95\xd7\xa2\xd7\x9e\xd7\x93\xd7\x99\xd7\x9d \xd7\x9c\xd7"
        "\x99\xd7\xa6\xd7\x99\xd7\xa8\xd7\xaa\xd7\x94 \xd7\x9e\xd7\xaa\xd7\x9f, \xd7\x90\xd7\x9c \xd7\xa9\xd7\x9e\xd7"
        "\x95 \xd7\xa9\xd7\xaa\xd7\xa4\xd7\x95 \xd7\x91\xd7\xa9\xd7\xa4\xd7\x94 \xd7\x9c\xd7\x99\xd7\xa6\xd7\x99\xd7\xa8"
        "\xd7\xaa\xd7\x94. \xd7\xa4\xd7\x99\xd7\xa1\xd7\x95\xd7\x9c \xd7\x9b\xd7\x9c\xd7\x9b\xd7\x9c\xd7\x94 \xd7\x91"
        "\xd7\xa9\xd7\xa4\xd7\x95\xd7\xaa \xd7\x90\xd7\x9c \xd7\xa9\xd7\x9e\xd7\x95. \xd7\xa9\xd7\x9c \xd7\x9b\xd7"
        "\xaa\xd7\x91 \xd7\x94\xd7\x9e\xd7\x9c\xd7\xa6\xd7\xaa \xd7\x95\xd7\xaa\xd7\xa9\xd7\x95\xd7\x91\xd7\x95\xd7\xaa,"
        " \xd7\x90\xd7\xa0\xd7\x90 \xd7\x91\xd7\x94 \xd7\xa7\xd7\x94\xd7\x99\xd7\x9c\xd7\x94 \xd7\x99\xd7\x99\xd6\xb4"
        "\xd7\x93\xd7\x99\xd7\xa9. \xd7\x91\xd7\xa7\xd7\xa8 \xd7\xa9\xd7\x9c \xd7\xaa\xd7\x99\xd7\x91\xd7\xaa \xd7"
        "\x91\xd7\x90\xd7\xa8\xd7\x92\xd7\x96 \xd7\x95\xd7\x99\xd7\xa7\xd7\x99\xd7\x9e\xd7\x93\xd7\x99\xd7\x94, \xd7\xaa"
        "\xd7\x97\xd7\x91\xd7\x95\xd7\xa8\xd7\x94 \xd7\x9e\xd7\x95\xd7\xa0\xd7\x97\xd7\x95\xd7\xa0\xd7\x99\xd7\x9d \xd7\x94"
        "\xd7\x90\xd7\xa0\xd7\xa6\xd7\x99\xd7\xa7\xd7\x9c\xd7\x95\xd7\xa4\xd7\x93\xd7\x99\xd7\x94 \xd7\x90\xd7\xaa \xd7\x9b"
        "\xd7\x93\xd7\x99. \xd7\x91\xd7\x93\xd7\xa4\xd7\x99\xd7\x9d \xd7\xa0\xd7\x95\xd7\xa1\xd7\x97\xd7\x90\xd7\x95\xd7"
        "\xaa \xd7\x94\xd7\x90\xd7\x98\xd7\x9e\xd7\x95\xd7\xa1\xd7\xa4\xd7\x99\xd7\xa8\xd7\x94 \xd7\x9b\xd7\x93\xd7\x99 "
        "\xd7\x9e\xd7\x94, \xd7\x9b\xd7\xaa\xd7\x91 \xd7\x90\xd7\x9c \xd7\x9e\xd7\x93\xd7\xa2\xd7\x99 \xd7\x94\xd7"
        "\x9e\xd7\x9c\xd7\xa6\xd7\xaa \xd7\xa8\xd7\x91\xd6\xbe\xd7\x9c\xd7\xa9\xd7\x95\xd7\xa0\xd7\x99.");

    auto mixed_rtl_text = std::string(
        "\xd7\xa6\xd7\x99\xd7\x9c\xd7\x95\xd7\x9d ipsum \xd7\xa2\xd7\x9c \xd7"
        "\xa2\xd7\x96\xd7\x94, \xd7\x90\xd7\x9d \xd7\x9e\xd7\x95\xd7\xa2\xd7\x9e\xd7\x93\xd7\x99\xd7\x9d \xd7\x9c\xd7"
        "\x99\xd7\xa6\xd7\x99\xd7\xa8\xd7\xaa\xd7\x94 \xd7\x9e\xd7\xaa\xd7\x9f, \xd7\x90\xd7\x9c (\xd7\xa9\xd7\x9e\xd7"
        "\x95 (laboris)) \xd7\x91\xd7\xa9\xd7\xa4\xd7\x94 aliqua. "
        "\xd7\xa4\xd7\x99\xd7\xa1\xd7\x95\xd7\x9c \xd7\x9b\xd7\x9c\xd7\x9b\xd7\x9c\xd7\x94 \xd7\x91"
        "\xd7\xa9\xd7\xa4\xd7\x95\xd7\xaa \xd7\x90\xd7\x9c \xd7\xa9\xd7\x9e\xd7\x95. \xd7\xa9\xd7\x9c \xd7\x9b\xd7"
        "\xaa\xd7\x91 \xd7\x94\xd7\x9e\xd7\x9c\xd7\xa6\xd7\xaa \xd7\x95\xd7\xaa\xd7\xa9\xd7\x95\xd7\x91\xd7\x95\xd7\xaa,"
        " \xd7\x90\xd7\xa0\xd7\x90 12345.67 \xd7\xa7\xd7\x94\xd7\x99\xd7\x9c\xd7\x94 \xd7\x99\xd7\x99\xd6\xb4"
        "\xd7\x93\xd7\x99\xd7\xa9. \xd7\x91\xd7\xa7\xd7\xa8 \xd7\xa9\xd7\x9c \xd7\xaa\xd7\x99\xd7\x91\xd7\xaa \xd7"
        "\x91\xd7\x90\xd7\xa8\xd7\x92\xd7\x96 \xd7\x95\xd7\x99\xd7\xa7\xd7\x99\xd7\x9e\xd7\x93\xd7\x99\xd7\x94, \xd7\xaa"
        "\xd7\x97\xd7\x91\xd7\x95\xd7\xa8\xd7\x94 voluptate velit (esse cillum) dolore \xd7\x94"
        "\xd7\x90\xd7\xa0\xd7\xa6\xd7\x99\xd7\xa7\xd7\x9c\xd7\x95\xd7\xa4\xd7\x93\xd7\x99\xd7\x94 $ 23.4 \xd7\x9b"
        "\xd7\x93\xd7\x99. sunt \xd7\xa0\xd7\x95\xd7\xa1\xd7\x97\xd7\x90\xd7\x95\xd7"
        "\xaa \xd7\x94\xd7\x90\xd7\x98\xd7\x9e\xd7\x95\xd7\xa1\xd7\xa4\xd7\x99\xd7\xa8\xd7\x94 \xd7\x9b\xd7\x93\xd7\x99 "
        "\xd7\x9e\xd7\x94, \xd7\x9b\xd7\xaa\xd7\x91 \xd7\x90\xd7\x9c \xd7\x9e\xd7\x93\xd7\xa2\xd7\x99 \xd7\x94\xd7"
        "\x9e\xd7\x9c\xd7\xa6\xd7\xaa \xd7\xa8\xd7\x91\xd6\xbe\xd7\x9c\xd7\xa9\xd7\x95\xd7\xa0\xd7\x99.");

    auto mixed_ltr_text = std::string(
        "Lorem ipsum dolor \xd7\x95\xd7\x99\xd7\xa7\xd7\x99\xd7\x9e\xd7\x93\xd7\x99\xd7\x94 amet, consectetur adipiscing elit, "
        "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
        "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
        "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
        "Excepteur sint occaecat cupidatat non \xd7\x95\xd7\x99\xd7\xa7\xd7\x99\xd7\x9e\xd7\x93\xd7\x99\xd7\x94, "
        "sunt in culpa qui officia deserunt mollit anim id est laborum.");

    auto text = to_text(latin_text + "\n" + mixed_rtl_text + "\n" + mixed_ltr_text + "\n" + hebrew_text);

    auto& tw = widget.content().make_widget<text_widget<>>("A1", text, hi::alignment::top_justified());
    tw.mode = hi::widget_mode::enabled;

    auto close_cb = window->closing.subscribe(
        [&] {
            window.reset();
        },
        hi::callback_flags::main);
    return loop::main().resume();
}
