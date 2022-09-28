// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "glob.hpp"
#include "utility.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace hi;


TEST(glob, parsing)
{
    ASSERT_EQ(to_regex_string(glob_pattern{"world"}), "world");
    ASSERT_EQ(to_regex_string(glob_pattern{"w*rld"}), "w[^/]*rld");
    ASSERT_EQ(to_regex_string(glob_pattern{"worl*"}), "worl[^/]*");
    ASSERT_EQ(to_regex_string(glob_pattern{"*orld"}), "[^/]*orld");
    ASSERT_EQ(to_regex_string(glob_pattern{"w?rld"}), "w[^/]rld");
    ASSERT_EQ(to_regex_string(glob_pattern{"worl?"}), "worl[^/]");
    ASSERT_EQ(to_regex_string(glob_pattern{"?orld"}), "[^/]orld");
    ASSERT_EQ(to_regex_string(glob_pattern{"w[abc]rld"}), "w(a|b|c)rld");
    ASSERT_EQ(to_regex_string(glob_pattern{"worl[abc]"}), "worl(a|b|c)");
    ASSERT_EQ(to_regex_string(glob_pattern{"[abc]orld"}), "(a|b|c)orld");
    ASSERT_THROW(glob_pattern{"worl[abc"}, parse_error);
    ASSERT_EQ(to_regex_string(glob_pattern{"w{ab,c}rld"}), "w(ab|c)rld");
    ASSERT_EQ(to_regex_string(glob_pattern{"worl{ab,c}"}), "worl(ab|c)");
    ASSERT_EQ(to_regex_string(glob_pattern{"{ab,c}orld"}), "(ab|c)orld");
    ASSERT_THROW(glob_pattern{"worl{ab,c"}, parse_error);

    ASSERT_EQ(to_regex_string(glob_pattern{"w/orld"}), "w/orld");
    ASSERT_EQ(to_regex_string(glob_pattern{"w/"}), "w/");
    ASSERT_EQ(to_regex_string(glob_pattern{"/world"}), "/world");
    ASSERT_THROW(glob_pattern{"w**rld"}, parse_error);
    ASSERT_EQ(to_regex_string(glob_pattern{"world/**"}), "world(/.*)?/");
    ASSERT_EQ(to_regex_string(glob_pattern{"world/**/"}), "world(/.*)?/");
    ASSERT_EQ(to_regex_string(glob_pattern{"hello/**/world"}), "hello(/.*)?/world");
    ASSERT_EQ(to_regex_string(glob_pattern{"/**/world"}), "(/.*)?/world");
}

//TEST(Glob, MatchStar)
//{
//    ASSERT_EQ(matchGlob("*bar", "foobar"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("*bar", "foobarbaz"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("*bar", "bar"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("*bar/baz", "foobar"), glob_match_result_t::Partial);
//
//    ASSERT_EQ(matchGlob("foo*", "foobar"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("foo*", "foo"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("foo*", "fo"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("foo*/baz", "foobar"), glob_match_result_t::Partial);
//
//    ASSERT_EQ(matchGlob("foo*baz", "foobarbaz"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("foo*baz", "foobaz"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("foo*baz", "fobaz"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("foo*baz", "foobarbaz"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("foo*baz/tree", "foobarbaz"), glob_match_result_t::Partial);
//    ASSERT_EQ(matchGlob("foo*baz/tree", "foobaz"), glob_match_result_t::Partial);
//
//    ASSERT_EQ(matchGlob("foo/*/baz", "foo/bar/baz"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("foo/*/baz", "foo/bar1/bar2/baz"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("foo/*/baz", "foo/bar1"), glob_match_result_t::Partial);
//}
//
//TEST(Glob, MatchDoubleStar)
//{
//    ASSERT_EQ(matchGlob("**bar", "foobar"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("**bar", "foobarbaz"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("**bar", "bar"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("**bar/baz", "foobar"), glob_match_result_t::Partial);
//
//    ASSERT_EQ(matchGlob("foo**", "foobar"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("foo**", "foo"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("foo**", "fo"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("foo**/baz", "foobar"), glob_match_result_t::Partial);
//
//    ASSERT_EQ(matchGlob("foo**baz", "foobarbaz"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("foo**baz", "foobaz"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("foo**baz", "fobaz"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("foo**baz", "foobarbaz"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("foo**baz/tree", "foobarbaz"), glob_match_result_t::Partial);
//    ASSERT_EQ(matchGlob("foo**baz/tree", "foobaz"), glob_match_result_t::Partial);
//
//    ASSERT_EQ(matchGlob("foo/**/baz", "foo/bar/baz"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("foo/**/baz", "foo/bar1/bar2/baz"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("foo/**/baz", "foo/baz"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("foo/**/baz", "foo/bar1"), glob_match_result_t::Partial);
//    ASSERT_EQ(matchGlob("foo/**/baz", "foo"), glob_match_result_t::Partial);
//}
//
//TEST(Glob, MatchQuestion)
//{
//    ASSERT_EQ(matchGlob("?ar", "bar"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("?ar", "ar"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("?ar", "obar"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("?/baz", "f"), glob_match_result_t::Partial);
//
//    ASSERT_EQ(matchGlob("fo?", "foo"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("fo?", "fo"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("fo?", "foop"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("fo?/baz", "foo"), glob_match_result_t::Partial);
//
//    ASSERT_EQ(matchGlob("f?o", "foo"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("f?o", "fo"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("f?o", "fooo"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("f?o/tree", "foo"), glob_match_result_t::Partial);
//
//    ASSERT_EQ(matchGlob("foo/??baz", "foo/b/baz"), glob_match_result_t::No);
//}
//
//TEST(Glob, MatchBrackets)
//{
//    ASSERT_EQ(matchGlob("[abc]ar", "bar"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("[abc]ar", "ar"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("[abc]ar", "obar"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("[abc]ar/baz", "bar"), glob_match_result_t::Partial);
//
//    ASSERT_EQ(matchGlob("fo[abc]", "fob"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("fo[abc]", "foo"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("fo[abc]", "fo"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("fo[abc]", "fobp"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("fo[abc]", "foop"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("fo[abc]/baz", "fob"), glob_match_result_t::Partial);
//
//    ASSERT_EQ(matchGlob("f[abc]o", "fbo"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("f[abc]o", "fb"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("f[abc]o", "fboo"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("f[abc]o/tree", "fbo"), glob_match_result_t::Partial);
//}
//
//TEST(Glob, MatchRange)
//{
//    ASSERT_EQ(matchGlob("[a-c]ar", "bar"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("[a-c]ar", "ar"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("[a-c]ar", "obar"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("[a-c]ar/baz", "bar"), glob_match_result_t::Partial);
//
//    ASSERT_EQ(matchGlob("fo[a-c]", "fob"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("fo[a-c]", "foo"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("fo[a-c]", "fo"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("fo[a-c]", "fobp"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("fo[a-c]", "foop"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("fo[a-c]/baz", "fob"), glob_match_result_t::Partial);
//
//    ASSERT_EQ(matchGlob("f[a-c]o", "fbo"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("f[a-c]o", "fb"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("f[a-c]o", "fboo"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("f[a-c]o/tree", "fbo"), glob_match_result_t::Partial);
//}
//
//TEST(Glob, MatchBraces)
//{
//    ASSERT_EQ(matchGlob("{12,23,1256}ar", "12ar"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("{12,23,1256}ar", "125ar"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("{12,23,1256}ar", "1256ar"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("{12,23,1256}ar", "ar"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("{12,23,1256}ar", "o12ar"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("{12,23,1256}ar/baz", "12ar"), glob_match_result_t::Partial);
//
//    ASSERT_EQ(matchGlob("fo{12,23,1256}", "fo12"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("fo{12,23,1256}", "foo"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("fo{12,23,1256}", "fo"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("fo{12,23,1256}", "fo12p"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("fo{12,23,1256}", "foop"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("fo{12,23,1256}/baz", "fo12"), glob_match_result_t::Partial);
//
//    ASSERT_EQ(matchGlob("f{12,23,1256}o", "f12o"), glob_match_result_t::Match);
//    ASSERT_EQ(matchGlob("f{12,23,1256}o", "f23"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("f{12,23,1256}o", "f23oo"), glob_match_result_t::No);
//    ASSERT_EQ(matchGlob("f{12,23,1256}o/tree", "f23o"), glob_match_result_t::Partial);
//}
//
//TEST(Glob, BasePath)
//{
//    ASSERT_EQ(basePathOfGlob("foo/bar/baz*"), "foo/bar");
//    ASSERT_EQ(basePathOfGlob("foo/bar/*"), "foo/bar");
//    ASSERT_EQ(basePathOfGlob("/foo/bar/baz*"), "/foo/bar");
//    ASSERT_EQ(basePathOfGlob("/foo/bar/*"), "/foo/bar");
//    ASSERT_EQ(basePathOfGlob("/foo*"), "/");
//    ASSERT_EQ(basePathOfGlob("/*"), "/");
//}
