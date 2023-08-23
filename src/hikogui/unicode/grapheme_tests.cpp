// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gstring.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>

[[nodiscard]] constexpr uint64_t grapheme_tests_default_grapheme_intrinsic(char32_t code_point)
{
    auto tmp = uint64_t{0};
    tmp |= std::to_underlying(hi::phrasing::success);
    tmp <<= 10;
    tmp |= hi::iso_3166{"BE"}.intrinsic();
    tmp <<= 10;
    tmp |= hi::iso_15924{"Cyrl"}.intrinsic();
    tmp <<= 15;
    tmp |= hi::iso_639{"fr"}.intrinsic();
    tmp <<= 21;
    tmp |= code_point;
    return tmp;
}

[[nodiscard]] constexpr hi::grapheme grapheme_tests_default_grapheme(char32_t code_point)
{
    return hi::grapheme{hi::intrinsic, grapheme_tests_default_grapheme_intrinsic(code_point)};
}

TEST(grapheme, intrinsic_init)
{
    auto c = grapheme_tests_default_grapheme(U'\U000e0061');
    ASSERT_EQ(c.size(), 1);
    ASSERT_EQ(c.starter(), U'\U000e0061');
    ASSERT_EQ(to_string(c.language()), "fr");
    ASSERT_EQ(to_string(c.script()), "Cyrl");
    ASSERT_EQ(to_string(c.region()), "BE");
    ASSERT_EQ(to_string(c.language_tag()), "fr-Cyrl-BE");
    ASSERT_EQ(c.phrasing(), hi::phrasing::success);
}

TEST(grapheme, set_language)
{
    auto c = grapheme_tests_default_grapheme(U'\U000e0061');
    c.set_language(hi::iso_639{"nl"});

    ASSERT_EQ(c.size(), 1);
    ASSERT_EQ(c.starter(), U'\U000e0061');
    ASSERT_EQ(to_string(c.language()), "nl");
    ASSERT_EQ(to_string(c.script()), "Cyrl");
    ASSERT_EQ(to_string(c.region()), "BE");
    ASSERT_EQ(to_string(c.language_tag()), "nl-Cyrl-BE");
    ASSERT_EQ(c.phrasing(), hi::phrasing::success);
}

TEST(grapheme, set_script)
{
    auto c = grapheme_tests_default_grapheme(U'a');
    c.set_script(hi::iso_15924{"Latn"});

    ASSERT_EQ(c.size(), 1);
    ASSERT_EQ(c.starter(), U'a');
    ASSERT_EQ(to_string(c.language()), "fr");
    ASSERT_EQ(to_string(c.script()), "Latn");
    ASSERT_EQ(to_string(c.region()), "BE");
    ASSERT_EQ(to_string(c.language_tag()), "fr-Latn-BE");
    ASSERT_EQ(c.phrasing(), hi::phrasing::success);
}

TEST(grapheme, set_wrong_script)
{
    auto c = grapheme_tests_default_grapheme(U'\U000e0061');
    c.set_script(hi::iso_15924{"Latn"});

    ASSERT_EQ(c.size(), 1);
    ASSERT_EQ(c.starter(), U'\U000e0061');
    ASSERT_EQ(to_string(c.language()), "fr");
    ASSERT_EQ(to_string(c.script()), "Zinh");
    ASSERT_EQ(to_string(c.region()), "BE");
    ASSERT_EQ(to_string(c.language_tag()), "fr-Zinh-BE");
    ASSERT_EQ(c.phrasing(), hi::phrasing::success);
}

TEST(grapheme, set_region)
{
    auto c = grapheme_tests_default_grapheme(U'\U000e0061');
    c.set_region(hi::iso_3166{"NL"});

    ASSERT_EQ(c.size(), 1);
    ASSERT_EQ(c.starter(), U'\U000e0061');
    ASSERT_EQ(to_string(c.language()), "fr");
    ASSERT_EQ(to_string(c.script()), "Cyrl");
    ASSERT_EQ(to_string(c.region()), "NL");
    ASSERT_EQ(to_string(c.language_tag()), "fr-Cyrl-NL");
    ASSERT_EQ(c.phrasing(), hi::phrasing::success);
}

TEST(grapheme, set_phrasing)
{
    auto c = grapheme_tests_default_grapheme(U'\U000e0061');
    c.set_phrasing(hi::phrasing::code);

    ASSERT_EQ(c.size(), 1);
    ASSERT_EQ(c.starter(), U'\U000e0061');
    ASSERT_EQ(to_string(c.language()), "fr");
    ASSERT_EQ(to_string(c.script()), "Cyrl");
    ASSERT_EQ(to_string(c.region()), "BE");
    ASSERT_EQ(to_string(c.language_tag()), "fr-Cyrl-BE");
    ASSERT_EQ(c.phrasing(), hi::phrasing::code);
}

TEST(grapheme, set_language_tag)
{
    auto c = grapheme_tests_default_grapheme(U'a');
    c.set_language_tag(hi::language_tag{"nl-Latn-NL"});

    ASSERT_EQ(c.size(), 1);
    ASSERT_EQ(c.starter(), U'a');
    ASSERT_EQ(to_string(c.language()), "nl");
    ASSERT_EQ(to_string(c.script()), "Latn");
    ASSERT_EQ(to_string(c.region()), "NL");
    ASSERT_EQ(to_string(c.language_tag()), "nl-Latn-NL");
    ASSERT_EQ(c.phrasing(), hi::phrasing::success);
}

TEST(grapheme, set_language_tag_wrong_script)
{
    auto c = grapheme_tests_default_grapheme(U'\U000e0061');
    c.set_language_tag(hi::language_tag{"nl-Latn-NL"});

    ASSERT_EQ(c.size(), 1);
    ASSERT_EQ(c.starter(), U'\U000e0061');
    ASSERT_EQ(to_string(c.language()), "nl");
    ASSERT_EQ(to_string(c.script()), "Zinh");
    ASSERT_EQ(to_string(c.region()), "NL");
    ASSERT_EQ(to_string(c.language_tag()), "nl-Zinh-NL");
    ASSERT_EQ(c.phrasing(), hi::phrasing::success);
}
