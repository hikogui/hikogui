// Copyright Take Vos 2021-2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <hikogui/hikogui.hpp>
#include <hikogui/crt.hpp>

using namespace hi;

task<void> checkbox_example()
{
    auto widget = std::make_unique<window_widget>(txt("Checkbox example"));

    /// [Create a label]
    widget->content().emplace<label_widget>("A1", txt("checkbox:"));
    /// [Create a label]

    /// [Create a checkbox]
    observer<int> value = 0;

    auto& cb = widget->content().emplace<checkbox_with_label_widget>("B1", value, 1, 2);
    cb.attributes.on_label = txt("on");
    cb.attributes.off_label = txt("off");
    cb.attributes.other_label = txt("other");
    /// [Create a checkbox]

    auto window = gui_window{std::move(widget)};
    co_await window.closing;
}

int hi_main(int argc, char* argv[])
{
    set_application_name("Install check");
    set_application_vendor("HikoGUI");
    set_application_version({1, 0, 0});

    checkbox_example();
    return loop::main().resume();
}
