// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_widget.hpp"
#include "../file/path_location.hpp"
#include "../GUI/theme_book.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace hi;

class text_widget_tests : public ::testing::Test {
protected:
    std::unique_ptr<hi::font_book> font_book;
    std::unique_ptr<hi::theme_book> theme_book;
    hi::theme theme;

    set_constraints_context c_context;

    observer<std::string> text;
    std::unique_ptr<hi::text_widget> widget;

    void SetUp() override
    {
        // Cursor movement (including editing) requires the text to be shaped.
        // text shaping requires fonts and text styles.
        font_book = std::make_unique<hi::font_book>(make_vector(get_paths(path_location::font_dirs)));
        theme_book = std::make_unique<hi::theme_book>(*font_book, make_vector(get_paths(path_location::theme_dirs)));
        theme = theme_book->find("default", theme_mode::light);
        c_context = set_constraints_context{font_book.get(), &theme};

        widget = std::make_unique<hi::text_widget>(nullptr, text);
        widget->mode = hi::widget_mode::enabled;

        auto constraints = widget->set_constraints(c_context);
        auto l_context = widget_layout{};
        l_context.size = constraints.preferred;
        l_context.font_book = font_book.get();
        l_context.theme = &theme;
        widget->set_layout(l_context);
    }
};

TEST_F(text_widget_tests, add_character)
{
    text = std::string{"hllo"};

    widget->handle_event(gui_event{gui_event_type::text_cursor_right_char});
    widget->handle_event(gui_event::keyboard_grapheme(grapheme{'e'}));

    ASSERT_EQ(text, "hello");
}

TEST_F(text_widget_tests, add_combining_character)
{
    text = std::string{"hllo"};

    widget->handle_event(gui_event{gui_event_type::text_cursor_right_char});
    widget->handle_event(gui_event::keyboard_partial_grapheme(grapheme{'"'}));
    widget->handle_event(gui_event::keyboard_grapheme(grapheme{U'\u00EB'})); // e-umlaut

    ASSERT_EQ(text, "h\u00EBllo");
}
