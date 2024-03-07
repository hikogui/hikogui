// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "theme_selector.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(theme_selector_suite) {

TEST_CASE(parse_single_element)
{
    auto expected = hi::theme_selector{};
    expected.emplace_back();
    expected.back().name = "foo";

    REQUIRE(hi::parse_theme_selector("foo") == expected);
    REQUIRE(hi::parse_theme_selector(" foo") == expected);
    REQUIRE(hi::parse_theme_selector("foo ") == expected);
    REQUIRE(hi::parse_theme_selector(" foo ") == expected);
}

TEST_CASE(parse_single_element_with_id)
{
    auto expected = hi::theme_selector{};
    expected.emplace_back();
    expected.back().id = "foo";

    REQUIRE(hi::parse_theme_selector("#foo") == expected);
    REQUIRE(hi::parse_theme_selector(" #foo") == expected);
    REQUIRE(hi::parse_theme_selector("#foo ") == expected);
    REQUIRE(hi::parse_theme_selector(" #foo ") == expected);
}

TEST_CASE(parse_single_element_with_class)
{
    auto expected = hi::theme_selector{};
    expected.emplace_back();
    expected.back().add_class("foo");

    REQUIRE(hi::parse_theme_selector(".foo") == expected);
    REQUIRE(hi::parse_theme_selector(" .foo") == expected);
    REQUIRE(hi::parse_theme_selector(".foo ") == expected);
    REQUIRE(hi::parse_theme_selector(" .foo ") == expected);
}

TEST_CASE(parse_single_element_with_psuedo)
{
    auto expected = hi::theme_selector{};
    expected.emplace_back();
    expected.back().add_pseudo("foo");

    REQUIRE(hi::parse_theme_selector(":foo") == expected);
    REQUIRE(hi::parse_theme_selector(" :foo") == expected);
    REQUIRE(hi::parse_theme_selector(":foo ") == expected);
    REQUIRE(hi::parse_theme_selector(" :foo ") == expected);
}

TEST_CASE(parse_two_elements)
{
    auto expected = hi::theme_selector{};
    expected.emplace_back();
    expected.back().name = "foo";
    expected.emplace_back();
    expected.back().name = "bar";

    REQUIRE(hi::parse_theme_selector("foo bar") == expected);
    REQUIRE(hi::parse_theme_selector("foo  bar") == expected);
    REQUIRE(hi::parse_theme_selector(" foo  bar") == expected);
    REQUIRE(hi::parse_theme_selector(" foo  bar ") == expected);
}

TEST_CASE(parse_two_direct_elements)
{
    auto expected = hi::theme_selector{};
    expected.emplace_back();
    expected.back().name = "foo";
    expected.back().is_direct_parent = true;
    expected.emplace_back();
    expected.back().name = "bar";

    REQUIRE(hi::parse_theme_selector("foo>bar") == expected);
    REQUIRE(hi::parse_theme_selector("foo> bar") == expected);
    REQUIRE(hi::parse_theme_selector("foo >bar") == expected);
    REQUIRE(hi::parse_theme_selector("foo > bar") == expected);
    REQUIRE(hi::parse_theme_selector("foo >> bar") == expected);
    REQUIRE(hi::parse_theme_selector("foo > > bar") == expected);
}

TEST_CASE(parse_two_classes)
{
    auto expected = hi::theme_selector{};
    expected.emplace_back();
    expected.back().add_class("foo");
    expected.back().add_class("bar");

    REQUIRE(hi::parse_theme_selector(".bar.foo") == expected);
    REQUIRE(hi::parse_theme_selector(".foo.bar") == expected);
    REQUIRE(hi::parse_theme_selector(" .foo.bar") == expected);
    REQUIRE(hi::parse_theme_selector(" .foo.bar") == expected);
    REQUIRE(hi::parse_theme_selector(" .foo.bar ") == expected);
    REQUIRE(hi::parse_theme_selector(" .foo.foo.bar ") == expected);
}

TEST_CASE(parse_two_elements_with_class)
{
    auto expected = hi::theme_selector{};
    expected.emplace_back();
    expected.back().add_class("foo");
    expected.emplace_back();
    expected.back().add_class("bar");

    REQUIRE(hi::parse_theme_selector(".foo .bar") == expected);
    REQUIRE(hi::parse_theme_selector(" .foo .bar") == expected);
    REQUIRE(hi::parse_theme_selector(" .foo .bar") == expected);
    REQUIRE(hi::parse_theme_selector(" .foo  .bar ") == expected);
    REQUIRE(hi::parse_theme_selector(" .foo.foo  .bar ") == expected);
}

TEST_CASE(parse_two_elements_with_id)
{
    auto expected = hi::theme_selector{};
    expected.emplace_back();
    expected.back().id = "foo";
    expected.emplace_back();
    expected.back().id = "bar";

    REQUIRE(hi::parse_theme_selector("#foo #bar") == expected);
    REQUIRE(hi::parse_theme_selector(" #foo #bar") == expected);
    REQUIRE(hi::parse_theme_selector(" #foo #bar") == expected);
    REQUIRE(hi::parse_theme_selector(" #foo  #bar ") == expected);
}

TEST_CASE(parse_two_psuedos)
{
    auto expected = hi::theme_selector{};
    expected.emplace_back();
    expected.back().add_pseudo("foo");
    expected.back().add_pseudo("bar");

    REQUIRE(hi::parse_theme_selector(":bar:foo") == expected);
    REQUIRE(hi::parse_theme_selector(":foo:bar") == expected);
    REQUIRE(hi::parse_theme_selector(" :foo:bar") == expected);
    REQUIRE(hi::parse_theme_selector(" :foo:bar") == expected);
    REQUIRE(hi::parse_theme_selector(" :foo:bar ") == expected);
    REQUIRE(hi::parse_theme_selector(" :foo:foo:bar ") == expected);
}

TEST_CASE(parse_two_elements_with_psuedo)
{
    auto expected = hi::theme_selector{};
    expected.emplace_back();
    expected.back().add_pseudo("foo");
    expected.emplace_back();
    expected.back().add_pseudo("bar");

    REQUIRE(hi::parse_theme_selector(":foo :bar") == expected);
    REQUIRE(hi::parse_theme_selector(" :foo :bar") == expected);
    REQUIRE(hi::parse_theme_selector(" :foo :bar") == expected);
    REQUIRE(hi::parse_theme_selector(" :foo  :bar ") == expected);
}

TEST_CASE(parse_complex1)
{
    auto expected = hi::theme_selector{};
    expected.emplace_back();
    expected.back().name = "foo";
    expected.back().id = "my";
    expected.back().add_class("kls1");
    expected.back().add_class("kls2");
    expected.back().add_pseudo("ps1");
    expected.back().add_pseudo("ps2");
    expected.emplace_back();
    expected.back().name = "bar";
    expected.back().id = "your";
    expected.back().add_class("kls3");
    expected.back().add_class("kls4");
    expected.back().add_pseudo("ps5");
    expected.back().add_pseudo("ps6");
    expected.back().is_direct_parent = true;

    REQUIRE(hi::parse_theme_selector("foo#my.kls1.kls2:ps1:ps2 bar#your.kls3.kls4:ps5:ps6>") == expected);
    REQUIRE(hi::parse_theme_selector("foo#my.kls1.kls2:ps1:ps2  bar#your.kls3.kls4:ps5:ps6 >") == expected);
    REQUIRE(hi::parse_theme_selector("foo#my.kls1.kls2:ps1:ps2  bar#your.kls4.kls3:ps5:ps6 >") == expected);
    REQUIRE(hi::parse_theme_selector(" foo#my.kls1.kls2:ps1:ps2  bar#your.kls4.kls3:ps5:ps6 >") == expected);
}


};
