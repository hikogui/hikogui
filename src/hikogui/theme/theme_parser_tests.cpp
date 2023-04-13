// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "theme_parser.hpp"
#include <gtest/gtest.h>
#include <string>

TEST(theme_parser, width)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    width : 100;\n"
        "}\n"};
    auto style_sheet = hi::parse_theme(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, "width");
    ASSERT_EQ(std::get<hi::points>(rule_set[0].value), hi::points{100});
}

TEST(theme_parser, width_pt)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    width : 100pt;\n"
        "}\n"};
    auto style_sheet = hi::parse_theme(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, "width");
    ASSERT_EQ(std::get<hi::points>(rule_set[0].value), hi::points{100});
}

TEST(theme_parser, width_in)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    width : 2in;\n"
        "}\n"};
    auto style_sheet = hi::parse_theme(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, "width");
    ASSERT_EQ(std::get<hi::points>(rule_set[0].value), hi::inches{2});
}

TEST(theme_parser, width_mm)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    width : 20mm;\n"
        "}\n"};
    auto style_sheet = hi::parse_theme(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, "width");
    ASSERT_EQ(std::get<hi::points>(rule_set[0].value), hi::millimeters{20});
}

TEST(theme_parser, width_cm)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    width : 2cm;\n"
        "}\n"};
    auto style_sheet = hi::parse_theme(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, "width");
    ASSERT_EQ(std::get<hi::points>(rule_set[0].value), hi::centimeters{2});
}

TEST(theme_parser, width_px)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    width : 100px;\n"
        "}\n"};
    auto style_sheet = hi::parse_theme(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, "width");
    ASSERT_EQ(std::get<hi::pixels>(rule_set[0].value), hi::pixels{100});
}

TEST(theme_parser, width_em)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    width : 100 em;\n"
        "}\n"};
    auto style_sheet = hi::parse_theme(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, "width");
    ASSERT_EQ(std::get<hi::em_quads>(rule_set[0].value), hi::em_quads{100});
}

TEST(theme_parser, background_color_hex6)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    background-color : #123456;\n"
        "}\n"};
    auto style_sheet = hi::parse_theme(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "foo");
    ASSERT_EQ(rule_set.size(), 1);

    auto& declaration = rule_set[0];
    ASSERT_EQ(declaration.name, "background-color");

    auto& value = std::get<hi::color>(declaration.value);
    ASSERT_EQ(value, hi::color_from_sRGB(0x12, 0x34, 0x56, 0xff));
}

TEST(theme_parser, background_color_hex8)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    background-color : #12345678;\n"
        "}\n"};
    auto style_sheet = hi::parse_theme(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "foo");
    ASSERT_EQ(rule_set.size(), 1);

    auto& declaration = rule_set[0];
    ASSERT_EQ(declaration.name, "background-color");

    auto& value = std::get<hi::color>(declaration.value);
    ASSERT_EQ(value, hi::color_from_sRGB(0x12, 0x34, 0x56, 0x78));
}

TEST(theme_parser, background_color_rgb_int)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    background-color : rgb(12, 34, 56);\n"
        "}\n"};
    auto style_sheet = hi::parse_theme(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "foo");
    ASSERT_EQ(rule_set.size(), 1);

    auto& declaration = rule_set[0];
    ASSERT_EQ(declaration.name, "background-color");

    auto& value = std::get<hi::color>(declaration.value);
    ASSERT_EQ(value, hi::color_from_sRGB(12, 34, 56, 255));
}

TEST(theme_parser, background_color_rgba_int)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    background-color : rgb(12, 34 56, 50%);\n"
        "}\n"};
    auto style_sheet = hi::parse_theme(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "foo");
    ASSERT_EQ(rule_set.size(), 1);

    auto& declaration = rule_set[0];
    ASSERT_EQ(declaration.name, "background-color");

    auto& value = std::get<hi::color>(declaration.value);
    auto expected_value = hi::color_from_sRGB(12, 34, 56, 255);
    expected_value.a() = 0.5f;
    ASSERT_EQ(value, expected_value);
}

TEST(theme_parser, background_color_rgb_float)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    background-color : rgb(0.12, -0.34, 0.56);\n"
        "}\n"};
    auto style_sheet = hi::parse_theme(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "foo");
    ASSERT_EQ(rule_set.size(), 1);

    auto& declaration = rule_set[0];
    ASSERT_EQ(declaration.name, "background-color");

    auto& value = std::get<hi::color>(declaration.value);
    ASSERT_EQ(value, hi::color(0.12f, -0.34f, 0.56f, 1.0f));
}

TEST(theme_parser, background_color_rgba_float)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    background-color : rgb(0.12, -0.34, 0.56, 0.78);\n"
        "}\n"};
    auto style_sheet = hi::parse_theme(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "foo");
    ASSERT_EQ(rule_set.size(), 1);

    auto& declaration = rule_set[0];
    ASSERT_EQ(declaration.name, "background-color");

    auto& value = std::get<hi::color>(declaration.value);
    ASSERT_EQ(value, hi::color(0.12f, -0.34f, 0.56f, 0.78f));
}

TEST(theme_parser, margin_1)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    margin : 10;\n"
        "}\n"};
    auto style_sheet = hi::parse_theme(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "foo");
    ASSERT_EQ(rule_set.size(), 4);

    ASSERT_EQ(rule_set[0].name, "margin-top");
    ASSERT_EQ(rule_set[1].name, "margin-right");
    ASSERT_EQ(rule_set[2].name, "margin-bottom");
    ASSERT_EQ(rule_set[3].name, "margin-left");

    ASSERT_EQ(std::get<hi::points>(rule_set[0].value), hi::points{10});
    ASSERT_EQ(std::get<hi::points>(rule_set[1].value), hi::points{10});
    ASSERT_EQ(std::get<hi::points>(rule_set[2].value), hi::points{10});
    ASSERT_EQ(std::get<hi::points>(rule_set[3].value), hi::points{10});
}

TEST(theme_parser, margin_2)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    margin : 10 20pt;\n"
        "}\n"};
    auto style_sheet = hi::parse_theme(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "foo");
    ASSERT_EQ(rule_set.size(), 4);

    ASSERT_EQ(rule_set[0].name, "margin-top");
    ASSERT_EQ(rule_set[1].name, "margin-right");
    ASSERT_EQ(rule_set[2].name, "margin-bottom");
    ASSERT_EQ(rule_set[3].name, "margin-left");

    ASSERT_EQ(std::get<hi::points>(rule_set[0].value), hi::points{10});
    ASSERT_EQ(std::get<hi::points>(rule_set[1].value), hi::points{20});
    ASSERT_EQ(std::get<hi::points>(rule_set[2].value), hi::points{10});
    ASSERT_EQ(std::get<hi::points>(rule_set[3].value), hi::points{20});
}

TEST(theme_parser, margin_3)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    margin : 10 20pt 30;\n"
        "}\n"};
    auto style_sheet = hi::parse_theme(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "foo");
    ASSERT_EQ(rule_set.size(), 4);

    ASSERT_EQ(rule_set[0].name, "margin-top");
    ASSERT_EQ(rule_set[1].name, "margin-right");
    ASSERT_EQ(rule_set[2].name, "margin-bottom");
    ASSERT_EQ(rule_set[3].name, "margin-left");

    ASSERT_EQ(std::get<hi::points>(rule_set[0].value), hi::points{10});
    ASSERT_EQ(std::get<hi::points>(rule_set[1].value), hi::points{20});
    ASSERT_EQ(std::get<hi::points>(rule_set[2].value), hi::points{30});
    ASSERT_EQ(std::get<hi::points>(rule_set[3].value), hi::points{20});
}

TEST(theme_parser, margin_4)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    margin : 10 20pt 30 40;\n"
        "}\n"};
    auto style_sheet = hi::parse_theme(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "foo");
    ASSERT_EQ(rule_set.size(), 4);

    ASSERT_EQ(rule_set[0].name, "margin-top");
    ASSERT_EQ(rule_set[1].name, "margin-right");
    ASSERT_EQ(rule_set[2].name, "margin-bottom");
    ASSERT_EQ(rule_set[3].name, "margin-left");

    ASSERT_EQ(std::get<hi::points>(rule_set[0].value), hi::points{10});
    ASSERT_EQ(std::get<hi::points>(rule_set[1].value), hi::points{20});
    ASSERT_EQ(std::get<hi::points>(rule_set[2].value), hi::points{30});
    ASSERT_EQ(std::get<hi::points>(rule_set[3].value), hi::points{40});
}
