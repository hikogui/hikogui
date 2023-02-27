// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_widget.hpp"
#include "../file/path_location.hpp"
#include "../GUI/module.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace hi;

class text_widget_tests : public ::testing::Test {
protected:
    class window_widget_moc : public hi::widget {
    public:
        window_widget_moc() noexcept : hi::widget(nullptr) {}
    };

    std::unique_ptr<hi::theme_book> theme_book;

    observer<hi::text> text;
    std::unique_ptr<window_widget_moc> window_widget;
    std::unique_ptr<hi::text_widget<>> widget;


    void SetUp() override
    {
        hi::start_system();
        os_settings::start_subsystem();

        // Cursor movement (including editing) requires the text to be shaped.
        // text shaping requires fonts and text styles.
        for (auto const &path : get_paths(path_location::font_dirs)) {
            register_font_directory(path);
        }


        theme_book = std::make_unique<hi::theme_book>(make_vector(get_paths(path_location::theme_dirs)));
        auto &theme = theme_book->find("default", theme_mode::light);
        theme.activate();

        widget = std::make_unique<hi::text_widget<>>(nullptr, text);
        widget->mode = hi::widget_mode::enabled;

        auto constraints = widget->update_constraints();
        auto layout = widget_layout{};
        layout.shape.rectangle = aarectanglei{constraints.preferred};
        layout.shape.baseline = constraints.preferred.height() / 2;
        // display_time_point is used to check for valid widget_layout.
        layout.display_time_point = std::chrono::utc_clock::now();
        widget->set_layout(layout);
    }
};

TEST_F(text_widget_tests, add_character)
{
    text = to_text(std::string{"hllo"});

    widget->handle_event(gui_event{gui_event_type::text_cursor_right_char});
    widget->handle_event(gui_event::keyboard_grapheme(grapheme{'e'}));

    ASSERT_EQ(text, to_text("hello"));
}

TEST_F(text_widget_tests, add_combining_character)
{
    text = to_text(std::string{"hllo"});

    widget->handle_event(gui_event{gui_event_type::text_cursor_right_char});
    widget->handle_event(gui_event::keyboard_partial_grapheme(grapheme{'"'}));
    widget->handle_event(gui_event::keyboard_grapheme(grapheme{U'\u00EB'})); // e-umlaut

    ASSERT_EQ(text, to_text("h\u00EBllo"));
}
