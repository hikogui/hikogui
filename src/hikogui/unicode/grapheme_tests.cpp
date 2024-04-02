// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gstring.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(grapheme_suite) {

[[nodiscard]] constexpr static uint64_t grapheme_tests_default_grapheme_intrinsic(char32_t code_point)
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

[[nodiscard]] constexpr static hi::grapheme grapheme_tests_default_grapheme(char32_t code_point)
{
    return hi::grapheme{std::in_place, grapheme_tests_default_grapheme_intrinsic(code_point)};
}

TEST_CASE(intrinsic_init)
{
    auto c = grapheme_tests_default_grapheme(U'\U000e0061');
    REQUIRE(c.size() == 1);
    REQUIRE(c.starter() == U'\U000e0061');
    REQUIRE(to_string(c.language()) == "fr");
    REQUIRE(to_string(c.script()) == "Cyrl");
    REQUIRE(to_string(c.region()) == "BE");
    REQUIRE(to_string(c.language_tag()) == "fr-Cyrl-BE");
    REQUIRE(c.phrasing() == hi::phrasing::success);
}

TEST_CASE(set_language)
{
    auto c = grapheme_tests_default_grapheme(U'\U000e0061');
    c.set_language(hi::iso_639{"nl"});

    REQUIRE(c.size() == 1);
    REQUIRE(c.starter() == U'\U000e0061');
    REQUIRE(to_string(c.language()) == "nl");
    REQUIRE(to_string(c.script()) == "Cyrl");
    REQUIRE(to_string(c.region()) == "BE");
    REQUIRE(to_string(c.language_tag()) == "nl-Cyrl-BE");
    REQUIRE(c.phrasing() == hi::phrasing::success);
}

TEST_CASE(set_script)
{
    auto c = grapheme_tests_default_grapheme(U'a');
    c.set_script(hi::iso_15924{"Latn"});

    REQUIRE(c.size() == 1);
    REQUIRE(c.starter() == U'a');
    REQUIRE(to_string(c.language()) == "fr");
    REQUIRE(to_string(c.script()) == "Latn");
    REQUIRE(to_string(c.region()) == "BE");
    REQUIRE(to_string(c.language_tag()) == "fr-Latn-BE");
    REQUIRE(c.phrasing() == hi::phrasing::success);
}

TEST_CASE(set_wrong_script)
{
    auto c = grapheme_tests_default_grapheme(U'\U000e0061');
    c.set_script(hi::iso_15924{"Latn"});

    REQUIRE(c.size() == 1);
    REQUIRE(c.starter() == U'\U000e0061');
    REQUIRE(to_string(c.language()) == "fr");
    REQUIRE(to_string(c.script()) == "Zinh");
    REQUIRE(to_string(c.region()) == "BE");
    REQUIRE(to_string(c.language_tag()) == "fr-Zinh-BE");
    REQUIRE(c.phrasing() == hi::phrasing::success);
}

TEST_CASE(set_region)
{
    auto c = grapheme_tests_default_grapheme(U'\U000e0061');
    c.set_region(hi::iso_3166{"NL"});

    REQUIRE(c.size() == 1);
    REQUIRE(c.starter() == U'\U000e0061');
    REQUIRE(to_string(c.language()) == "fr");
    REQUIRE(to_string(c.script()) == "Cyrl");
    REQUIRE(to_string(c.region()) == "NL");
    REQUIRE(to_string(c.language_tag()) == "fr-Cyrl-NL");
    REQUIRE(c.phrasing() == hi::phrasing::success);
}

TEST_CASE(set_phrasing)
{
    auto c = grapheme_tests_default_grapheme(U'\U000e0061');
    c.set_phrasing(hi::phrasing::code);

    REQUIRE(c.size() == 1);
    REQUIRE(c.starter() == U'\U000e0061');
    REQUIRE(to_string(c.language()) == "fr");
    REQUIRE(to_string(c.script()) == "Cyrl");
    REQUIRE(to_string(c.region()) == "BE");
    REQUIRE(to_string(c.language_tag()) == "fr-Cyrl-BE");
    REQUIRE(c.phrasing() == hi::phrasing::code);
}

TEST_CASE(set_language_tag)
{
    auto c = grapheme_tests_default_grapheme(U'a');
    c.set_language_tag(hi::language_tag{"nl-Latn-NL"});

    REQUIRE(c.size() == 1);
    REQUIRE(c.starter() == U'a');
    REQUIRE(to_string(c.language()) == "nl");
    REQUIRE(to_string(c.script()) == "Latn");
    REQUIRE(to_string(c.region()) == "NL");
    REQUIRE(to_string(c.language_tag()) == "nl-Latn-NL");
    REQUIRE(c.phrasing() == hi::phrasing::success);
}

TEST_CASE(set_language_tag_wrong_script)
{
    auto c = grapheme_tests_default_grapheme(U'\U000e0061');
    c.set_language_tag(hi::language_tag{"nl-Latn-NL"});

    REQUIRE(c.size() == 1);
    REQUIRE(c.starter() == U'\U000e0061');
    REQUIRE(to_string(c.language()) == "nl");
    REQUIRE(to_string(c.script()) == "Zinh");
    REQUIRE(to_string(c.region()) == "NL");
    REQUIRE(to_string(c.language_tag()) == "nl-Zinh-NL");
    REQUIRE(c.phrasing() == hi::phrasing::success);
}

};
