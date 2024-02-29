// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "font_weight.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(font_weight) {

TEST_CASE(regular_alternatives)
{
    auto range = hi::alternatives(hi::font_weight::regular);
    auto it = range.begin();
    REQUIRE(it != range.end());
    REQUIRE(*it++ == hi::font_weight::regular);
    REQUIRE(*it++ == hi::font_weight::light);
    REQUIRE(*it++ == hi::font_weight::medium);
    REQUIRE(*it++ == hi::font_weight::extra_light);
    REQUIRE(*it++ == hi::font_weight::semi_bold);
    REQUIRE(*it++ == hi::font_weight::thin);
    REQUIRE(*it++ == hi::font_weight::bold);
    REQUIRE(*it++ == hi::font_weight::extra_bold);
    REQUIRE(*it++ == hi::font_weight::black);
    REQUIRE(*it++ == hi::font_weight::extra_black);
    REQUIRE(it == range.end());
}

TEST_CASE(bold_alternatives)
{
    auto range = hi::alternatives(hi::font_weight::bold);
    auto it = range.begin();
    REQUIRE(it != range.end());
    REQUIRE(*it++ == hi::font_weight::bold);
    REQUIRE(*it++ == hi::font_weight::semi_bold);
    REQUIRE(*it++ == hi::font_weight::extra_bold);
    REQUIRE(*it++ == hi::font_weight::medium);
    REQUIRE(*it++ == hi::font_weight::black);
    REQUIRE(*it++ == hi::font_weight::regular);
    REQUIRE(*it++ == hi::font_weight::extra_black);
    REQUIRE(*it++ == hi::font_weight::light);
    REQUIRE(*it++ == hi::font_weight::extra_light);
    REQUIRE(*it++ == hi::font_weight::thin);
    REQUIRE(it == range.end());
}

};
