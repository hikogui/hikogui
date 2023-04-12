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

    auto& declaration = rule_set[0];
    ASSERT_EQ(declaration.name, "width");

    auto& value = std::get<hi::detail::theme_length>(declaration.value);
    ASSERT_EQ(value.type, hi::detail::theme_length::length_type::pt);
    ASSERT_EQ(value.value, 100);
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

    auto& declaration = rule_set[0];
    ASSERT_EQ(declaration.name, "width");

    auto& value = std::get<hi::detail::theme_length>(declaration.value);
    ASSERT_EQ(value.type, hi::detail::theme_length::length_type::pt);
    ASSERT_EQ(value.value, 100);
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

    auto& declaration = rule_set[0];
    ASSERT_EQ(declaration.name, "width");

    auto& value = std::get<hi::detail::theme_length>(declaration.value);
    ASSERT_EQ(value.type, hi::detail::theme_length::length_type::px);
    ASSERT_EQ(value.value, 100);
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

    auto& declaration = rule_set[0];
    ASSERT_EQ(declaration.name, "width");

    auto& value = std::get<hi::detail::theme_length>(declaration.value);
    ASSERT_EQ(value.type, hi::detail::theme_length::length_type::em);
    ASSERT_EQ(value.value, 100);
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
    ASSERT_EQ(value, hi::color_from_sRGB(12, 34, 56, 128));
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
