// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "style_sheet_parser.hpp"
#include <gtest/gtest.h>
#include <string>

TEST(style_sheet_parser, at_mode_light)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 0);
}

TEST(style_sheet_parser, at_mode_dark)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode dark;\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::dark);
    ASSERT_EQ(style_sheet.size(), 0);
}

TEST(style_sheet_parser, missing_at_name_or_at_mode)
{
    {
        auto css = std::string{
            "@name \"default\";\n"
            "\n"
            "foo {\n"
            "    width : 100;\n"
            "}\n"};
        ASSERT_THROW((void)hi::parse_style_sheet(css, std::filesystem::path{"theme.css"}), hi::parse_error);
    }
    {
        auto css = std::string{
            "@mode light;\n"
            "\n"
            "foo {\n"
            "    width : 100;\n"
            "}\n"};
        ASSERT_THROW((void)hi::parse_style_sheet(css, std::filesystem::path{"theme.css"}), hi::parse_error);
    }
    {
        auto css = std::string{
            "foo {\n"
            "    width : 100;\n"
            "}\n"};
        ASSERT_THROW((void)hi::parse_style_sheet(css, std::filesystem::path{"theme.css"}), hi::parse_error);
    }
}

TEST(style_sheet_parser, width)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    width : 100;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::width);
    ASSERT_FALSE(rule_set[0].important);
    ASSERT_EQ(std::get<hi::dips>(rule_set[0].value), hi::dips{100});
}

TEST(style_sheet_parser, width_pt)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    width : 100pt;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::width);
    ASSERT_FALSE(rule_set[0].important);
    ASSERT_EQ(std::get<hi::dips>(rule_set[0].value), hi::points{100});
}

TEST(style_sheet_parser, width_in)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    width : 2in;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::width);
    ASSERT_FALSE(rule_set[0].important);
    ASSERT_EQ(std::get<hi::dips>(rule_set[0].value), hi::inches{2});
}

TEST(style_sheet_parser, width_mm_important)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    width : 20mm !important;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::width);
    ASSERT_TRUE(rule_set[0].important);
    ASSERT_EQ(std::get<hi::dips>(rule_set[0].value), hi::millimeters{20});
}

TEST(style_sheet_parser, width_cm)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    width : 2cm;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::width);
    ASSERT_FALSE(rule_set[0].important);
    ASSERT_EQ(std::get<hi::dips>(rule_set[0].value), hi::centimeters{2});
}

TEST(style_sheet_parser, width_px)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    width : 100px;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::width);
    ASSERT_FALSE(rule_set[0].important);
    ASSERT_EQ(std::get<hi::pixels>(rule_set[0].value), hi::pixels{100});
}

TEST(style_sheet_parser, width_incorrect_type)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    width : rgb(0, 0, 0);\n"
        "}\n"};
    ASSERT_THROW((void)hi::parse_style_sheet(css, std::filesystem::path{"theme.css"}), hi::parse_error);
}

TEST(style_sheet_parser, invalid_declaration_name_underscore)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    font_weight : 100;\n"
        "}\n"};
    ASSERT_THROW((void)hi::parse_style_sheet(css, std::filesystem::path{"theme.css"}), hi::parse_error);
}

TEST(style_sheet_parser, font_weight_integer)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    font-weight : 100;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::font_weight);
    ASSERT_FALSE(rule_set[0].important);
    ASSERT_EQ(std::get<hi::font_weight>(rule_set[0].value), hi::font_weight::thin);
}

TEST(style_sheet_parser, font_weight_identifier)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    font-weight : extra-light;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::font_weight);
    ASSERT_FALSE(rule_set[0].important);
    ASSERT_EQ(std::get<hi::font_weight>(rule_set[0].value), hi::font_weight::extra_light);
}

TEST(style_sheet_parser, font_style_normal)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    font-style : normal;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::font_style);
    ASSERT_FALSE(rule_set[0].important);
    ASSERT_EQ(std::get<hi::font_style>(rule_set[0].value), hi::font_style::normal);
}

TEST(style_sheet_parser, font_style_italic)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    font-style : italic;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::font_style);
    ASSERT_FALSE(rule_set[0].important);
    ASSERT_EQ(std::get<hi::font_style>(rule_set[0].value), hi::font_style::italic);
}

TEST(style_sheet_parser, font_style_oblique_important)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    font-style : oblique ! important;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::font_style);
    ASSERT_TRUE(rule_set[0].important);
    ASSERT_EQ(std::get<hi::font_style>(rule_set[0].value), hi::font_style::oblique);
}

TEST(style_sheet_parser, font_family_string)
{
    hi::register_font_directories(hi::get_paths(hi::path_location::font_dirs));

    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    font-family : \"Times New Roman\", serif;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::font_family);
    ASSERT_FALSE(rule_set[0].important);
    ASSERT_TRUE(std::get<hi::font_family_id>(rule_set[0].value));
}

TEST(style_sheet_parser, font_family_id)
{
    hi::register_font_directories(hi::get_paths(hi::path_location::font_dirs));

    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    font-family : \"Helvetica\", sans-serif;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::font_family);
    ASSERT_FALSE(rule_set[0].important);
    ASSERT_TRUE(std::get<hi::font_family_id>(rule_set[0].value));
}

TEST(style_sheet_parser, font_family_missing)
{
    hi::register_font_directories(hi::get_paths(hi::path_location::font_dirs));

    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    font-family : \"Non existing font\";\n"
        "}\n"};
    ASSERT_THROW((void)hi::parse_style_sheet(css, std::filesystem::path{"theme.css"}), hi::parse_error);
}

TEST(style_sheet_parser, width_em)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    width : 100 em;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);
    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::width);
    ASSERT_FALSE(rule_set[0].important);
    ASSERT_EQ(std::get<hi::em_quads>(rule_set[0].value), hi::em_quads{100});
}

TEST(style_sheet_parser, background_color_hex6)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    background-color : #123456;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);

    auto& declaration = rule_set[0];
    ASSERT_EQ(declaration.name, hi::style_sheet_declaration_name::background_color);
    ASSERT_FALSE(declaration.important);

    auto& value = std::get<hi::color>(declaration.value);
    ASSERT_EQ(value, hi::color_from_sRGB(0x12, 0x34, 0x56, 0xff));
}

TEST(style_sheet_parser, background_color_hex8)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    background-color : #12345678;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);

    auto& declaration = rule_set[0];
    ASSERT_EQ(declaration.name, hi::style_sheet_declaration_name::background_color);
    ASSERT_FALSE(declaration.important);

    auto& value = std::get<hi::color>(declaration.value);
    ASSERT_EQ(value, hi::color_from_sRGB(0x12, 0x34, 0x56, 0x78));
}

TEST(style_sheet_parser, background_color_rgb_int)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    background-color : rgb(12, 34, 56);\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);

    auto& declaration = rule_set[0];
    ASSERT_EQ(declaration.name, hi::style_sheet_declaration_name::background_color);
    ASSERT_FALSE(declaration.important);

    auto& value = std::get<hi::color>(declaration.value);
    ASSERT_EQ(value, hi::color_from_sRGB(12, 34, 56, 255));
}

TEST(style_sheet_parser, background_color_rgba_int)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    background-color : rgb(12, 34 56, 50%);\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);

    auto& declaration = rule_set[0];
    ASSERT_EQ(declaration.name, hi::style_sheet_declaration_name::background_color);
    ASSERT_FALSE(declaration.important);

    auto& value = std::get<hi::color>(declaration.value);
    auto expected_value = hi::color_from_sRGB(12, 34, 56, 255);
    expected_value.a() = 0.5f;
    ASSERT_EQ(value, expected_value);
}

TEST(style_sheet_parser, background_color_rgb_float)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    background-color : rgb(0.12, -0.34, 0.56);\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);

    auto& declaration = rule_set[0];
    ASSERT_EQ(declaration.name, hi::style_sheet_declaration_name::background_color);
    ASSERT_FALSE(declaration.important);

    auto& value = std::get<hi::color>(declaration.value);
    ASSERT_EQ(value, hi::color(0.12f, -0.34f, 0.56f, 1.0f));
}

TEST(style_sheet_parser, background_color_rgba_float)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    background-color : rgb(0.12, -0.34, 0.56, 0.78);\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 1);

    auto& declaration = rule_set[0];
    ASSERT_EQ(declaration.name, hi::style_sheet_declaration_name::background_color);
    ASSERT_FALSE(declaration.important);

    auto& value = std::get<hi::color>(declaration.value);
    ASSERT_EQ(value, hi::color(0.12f, -0.34f, 0.56f, 0.78f));
}

TEST(style_sheet_parser, margin_1)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    margin : 10;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 4);

    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::margin_top);
    ASSERT_EQ(rule_set[1].name, hi::style_sheet_declaration_name::margin_right);
    ASSERT_EQ(rule_set[2].name, hi::style_sheet_declaration_name::margin_bottom);
    ASSERT_EQ(rule_set[3].name, hi::style_sheet_declaration_name::margin_left);

    ASSERT_FALSE(rule_set[0].important);
    ASSERT_FALSE(rule_set[1].important);
    ASSERT_FALSE(rule_set[2].important);
    ASSERT_FALSE(rule_set[3].important);

    ASSERT_EQ(std::get<hi::dips>(rule_set[0].value), hi::dips{10});
    ASSERT_EQ(std::get<hi::dips>(rule_set[1].value), hi::dips{10});
    ASSERT_EQ(std::get<hi::dips>(rule_set[2].value), hi::dips{10});
    ASSERT_EQ(std::get<hi::dips>(rule_set[3].value), hi::dips{10});
}

TEST(style_sheet_parser, margin_2)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    margin : 10 20pt;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 4);

    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::margin_top);
    ASSERT_EQ(rule_set[1].name, hi::style_sheet_declaration_name::margin_right);
    ASSERT_EQ(rule_set[2].name, hi::style_sheet_declaration_name::margin_bottom);
    ASSERT_EQ(rule_set[3].name, hi::style_sheet_declaration_name::margin_left);

    ASSERT_FALSE(rule_set[0].important);
    ASSERT_FALSE(rule_set[1].important);
    ASSERT_FALSE(rule_set[2].important);
    ASSERT_FALSE(rule_set[3].important);

    ASSERT_EQ(std::get<hi::dips>(rule_set[0].value), hi::dips{10});
    ASSERT_EQ(std::get<hi::dips>(rule_set[1].value), hi::points{20});
    ASSERT_EQ(std::get<hi::dips>(rule_set[2].value), hi::dips{10});
    ASSERT_EQ(std::get<hi::dips>(rule_set[3].value), hi::points{20});
}

TEST(style_sheet_parser, margin_3_important)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    margin : 10 20pt 30 !important;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 4);

    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::margin_top);
    ASSERT_EQ(rule_set[1].name, hi::style_sheet_declaration_name::margin_right);
    ASSERT_EQ(rule_set[2].name, hi::style_sheet_declaration_name::margin_bottom);
    ASSERT_EQ(rule_set[3].name, hi::style_sheet_declaration_name::margin_left);

    ASSERT_TRUE(rule_set[0].important);
    ASSERT_TRUE(rule_set[1].important);
    ASSERT_TRUE(rule_set[2].important);
    ASSERT_TRUE(rule_set[3].important);

    ASSERT_EQ(std::get<hi::dips>(rule_set[0].value), hi::dips{10});
    ASSERT_EQ(std::get<hi::dips>(rule_set[1].value), hi::points{20});
    ASSERT_EQ(std::get<hi::dips>(rule_set[2].value), hi::dips{30});
    ASSERT_EQ(std::get<hi::dips>(rule_set[3].value), hi::points{20});
}

TEST(style_sheet_parser, margin_4)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo {\n"
        "    margin:10em 20px 30 40;\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);

    auto& rule_set = style_sheet[0];
    ASSERT_EQ(rule_set.get_selector_as_string(), "/**/foo");
    ASSERT_EQ(rule_set.size(), 4);

    ASSERT_EQ(rule_set[0].name, hi::style_sheet_declaration_name::margin_top);
    ASSERT_EQ(rule_set[1].name, hi::style_sheet_declaration_name::margin_right);
    ASSERT_EQ(rule_set[2].name, hi::style_sheet_declaration_name::margin_bottom);
    ASSERT_EQ(rule_set[3].name, hi::style_sheet_declaration_name::margin_left);

    ASSERT_FALSE(rule_set[0].important);
    ASSERT_FALSE(rule_set[1].important);
    ASSERT_FALSE(rule_set[2].important);
    ASSERT_FALSE(rule_set[3].important);

    ASSERT_EQ(std::get<hi::em_quads>(rule_set[0].value), hi::em_quads{10});
    ASSERT_EQ(std::get<hi::pixels>(rule_set[1].value), hi::pixels{20});
    ASSERT_EQ(std::get<hi::dips>(rule_set[2].value), hi::dips{30});
    ASSERT_EQ(std::get<hi::dips>(rule_set[3].value), hi::dips{40});
}

TEST(style_sheet_parser, state_disabled)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo:disabled {\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);
    ASSERT_EQ(style_sheet[0].state, hi::theme_state::disabled);
    ASSERT_EQ(style_sheet[0].state_mask, hi::theme_state_mask::mouse);
}

TEST(style_sheet_parser, state_enabled)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo:enabled {\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);
    ASSERT_EQ(style_sheet[0].state, hi::theme_state::enabled);
    ASSERT_EQ(style_sheet[0].state_mask, hi::theme_state_mask::mouse);
}

TEST(style_sheet_parser, state_hover_and_focus)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo:hover:focus {\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);
    ASSERT_EQ(style_sheet[0].state, hi::theme_state::hover | hi::theme_state::focus);
    ASSERT_EQ(style_sheet[0].state_mask, hi::theme_state_mask::mouse | hi::theme_state_mask::focus);
}

TEST(style_sheet_parser, state_active_and_layer)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo:active:layer(2) {\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);
    ASSERT_EQ(style_sheet[0].state, hi::theme_state::active | hi::theme_state::layer_2);
    ASSERT_EQ(style_sheet[0].state_mask, hi::theme_state_mask::mouse | hi::theme_state_mask::layers);
}

TEST(style_sheet_parser, state_no_focus_and_layer_and_on)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo:no-focus:layer(1):on {\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);
    ASSERT_EQ(style_sheet[0].state, hi::theme_state::no_focus | hi::theme_state::layer_1 | hi::theme_state::on);
    ASSERT_EQ(
        style_sheet[0].state_mask, hi::theme_state_mask::focus | hi::theme_state_mask::layers | hi::theme_state_mask::value);
}

TEST(style_sheet_parser, state_lang)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo:lang(en-US) {\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);
    ASSERT_EQ(style_sheet[0].language_mask, hi::language_tag{"en-US"});
}

TEST(style_sheet_parser, state_lang_star)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo:lang(*-US) {\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);
    ASSERT_EQ(style_sheet[0].language_mask, hi::language_tag{"*-US"});
}

TEST(style_sheet_parser, state_phrasing)
{
    auto css = std::string{
        "@name \"default\";\n"
        "@mode light;\n"
        "\n"
        "foo:phrasing(se) {\n"
        "}\n"};
    auto style_sheet = hi::parse_style_sheet(css, std::filesystem::path{"theme.css"});

    ASSERT_EQ(style_sheet.name, "default");
    ASSERT_EQ(style_sheet.mode, hi::theme_mode::light);
    ASSERT_EQ(style_sheet.size(), 1);
    ASSERT_EQ(style_sheet[0].phrasing_mask, hi::text_phrasing_mask::strong | hi::text_phrasing_mask::emphesis);
}
