// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "glob.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(glob_suite) {

TEST_CASE(parse)
{
    REQUIRE(hi::glob_pattern{"world"}.debug_string() == "'world'");
    REQUIRE(hi::glob_pattern{"w*rld"}.debug_string() == "'w'*'rld'");
    REQUIRE(hi::glob_pattern{"worl*"}.debug_string() == "'worl'*");
    REQUIRE(hi::glob_pattern{"*orld"}.debug_string() == "*'orld'");
    REQUIRE(hi::glob_pattern{"w?rld"}.debug_string() == "'w'?'rld'");
    REQUIRE(hi::glob_pattern{"worl?"}.debug_string() == "'worl'?");
    REQUIRE(hi::glob_pattern{"?orld"}.debug_string() == "?'orld'");
    REQUIRE(hi::glob_pattern{"w[abc]rld"}.debug_string() == "'w'[abc]'rld'");
    REQUIRE(hi::glob_pattern{"worl[abc]"}.debug_string() == "'worl'[abc]");
    REQUIRE(hi::glob_pattern{"[abc]orld"}.debug_string() == "[abc]'orld'");
    REQUIRE_THROWS(hi::glob_pattern{"worl[abc"}, hi::parse_error);
    REQUIRE(hi::glob_pattern{"w{ab,c}rld"}.debug_string() == "'w'{ab,c}'rld'");
    REQUIRE(hi::glob_pattern{"worl{ab,c}"}.debug_string() == "'worl'{ab,c}");
    REQUIRE(hi::glob_pattern{"{ab,c}orld"}.debug_string() == "{ab,c}'orld'");
    REQUIRE_THROWS(hi::glob_pattern{"worl{ab,c"}, hi::parse_error);

    REQUIRE(hi::glob_pattern{"w/orld"}.debug_string() == "'w/orld'");
    REQUIRE(hi::glob_pattern{"w/"}.debug_string() == "'w/'");
    REQUIRE(hi::glob_pattern{"/world"}.debug_string() == "'/world'");
    REQUIRE_THROWS(hi::glob_pattern{"w**rld"}, hi::parse_error);
    REQUIRE(hi::glob_pattern{"world/**"}.debug_string() == "'world'/**/");
    REQUIRE(hi::glob_pattern{"world/**/"}.debug_string() == "'world'/**/");
    REQUIRE(hi::glob_pattern{"hello/**/world"}.debug_string() == "'hello'/**/'world'");
    REQUIRE(hi::glob_pattern{"/**/world"}.debug_string() == "/**/'world'");
}

};
