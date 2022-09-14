// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "glob.hpp"
#include "utility.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace hi;

#define ASSERT_TOKEN_EQ2(token, name) \
    do { \
        hilet expected = glob_token_t{glob_callback_token_t::name}; \
        ASSERT_EQ(token, expected); \
    } while (false)

#define ASSERT_TOKEN_EQ3(token, name, value) \
    do { \
        hilet expected = glob_token_t{glob_callback_token_t::name, value}; \
        ASSERT_EQ(token, expected); \
    } while (false)

#define ASSERT_TOKEN_EQ5(token, name, value1, value2, value3) \
    do { \
        hilet expected = glob_token_t{glob_callback_token_t::name, std::vector<std::string>{value1, value2, value3}}; \
        ASSERT_EQ(token, expected); \
    } while (false)

TEST(Glob, ParseNoPattern)
{
    auto tokens = parseGlob("world");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], String, "world");
}

TEST(Glob, ParseSlashPattern)
{
    auto tokens = parseGlob("w/rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ2(tokens[1], Separator);
    ASSERT_TOKEN_EQ3(tokens[2], String, "rld");
}

TEST(Glob, ParseSlashPatternAtEnd)
{
    auto tokens = parseGlob("w/");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ2(tokens[1], Separator);
}

TEST(Glob, ParseSlashPatternAtBegin)
{
    auto tokens = parseGlob("/world");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ2(tokens[0], Separator);
    ASSERT_TOKEN_EQ3(tokens[1], String, "world");
}

TEST(Glob, ParseStarPattern)
{
    auto tokens = parseGlob("w*rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyString);
    ASSERT_TOKEN_EQ3(tokens[2], String, "rld");
}

TEST(Glob, ParseStarPatternAtEnd)
{
    auto tokens = parseGlob("w*");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyString);
}

TEST(Glob, ParseStarPatternAtBegin)
{
    auto tokens = parseGlob("*world");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ2(tokens[0], AnyString);
    ASSERT_TOKEN_EQ3(tokens[1], String, "world");
}

TEST(Glob, ParseDoubleStarPattern)
{
    auto tokens = parseGlob("w**rld");
    ASSERT_EQ(tokens.size(), 4);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyString);
    ASSERT_TOKEN_EQ2(tokens[2], AnyString);
    ASSERT_TOKEN_EQ3(tokens[3], String, "rld");
}

TEST(Glob, ParseDoubleStarPatternAtEnd)
{
    auto tokens = parseGlob("w**");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyString);
    ASSERT_TOKEN_EQ2(tokens[2], AnyString);
}

TEST(Glob, ParseDoubleStarPatternAtBegin)
{
    auto tokens = parseGlob("**world");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ2(tokens[0], AnyString);
    ASSERT_TOKEN_EQ2(tokens[1], AnyString);
    ASSERT_TOKEN_EQ3(tokens[2], String, "world");
}

TEST(Glob, ParseSlashDoubleStarPattern)
{
    auto tokens = parseGlob("w/**/rld");
    ASSERT_EQ(tokens.size(), 4);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyDirectory);
    ASSERT_TOKEN_EQ2(tokens[2], Separator);
    ASSERT_TOKEN_EQ3(tokens[3], String, "rld");
}

TEST(Glob, ParseSlashDoubleStarPatternAtEnd)
{
    auto tokens = parseGlob("w/**/");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyDirectory);
    ASSERT_TOKEN_EQ2(tokens[2], Separator);
}

TEST(Glob, ParseSlashDoubleStarPatternAtBegin)
{
    auto tokens = parseGlob("/**/world");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ2(tokens[0], AnyDirectory);
    ASSERT_TOKEN_EQ2(tokens[1], Separator);
    ASSERT_TOKEN_EQ3(tokens[2], String, "world");
}

TEST(Glob, ParseQuestionPattern)
{
    auto tokens = parseGlob("w?rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyCharacter);
    ASSERT_TOKEN_EQ3(tokens[2], String, "rld");
}

TEST(Glob, ParseQuestionPatternAtEnd)
{
    auto tokens = parseGlob("w?");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyCharacter);
}

TEST(Glob, ParseQuestionPatternAtBegin)
{
    auto tokens = parseGlob("?world");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ2(tokens[0], AnyCharacter);
    ASSERT_TOKEN_EQ3(tokens[1], String, "world");
}

TEST(Glob, ParseCharacterListPattern)
{
    auto tokens = parseGlob("w[abc]rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ3(tokens[1], CharacterList, "abc");
    ASSERT_TOKEN_EQ3(tokens[2], String, "rld");
}

TEST(Glob, ParseCharacterListPatternAtBegin)
{
    auto tokens = parseGlob("[abc]world");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], CharacterList, "abc");
    ASSERT_TOKEN_EQ3(tokens[1], String, "world");
}

TEST(Glob, ParseCharacterListPatternAtEnd)
{
    auto tokens = parseGlob("world[abc]");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], String, "world");
    ASSERT_TOKEN_EQ3(tokens[1], CharacterList, "abc");
}

TEST(Glob, ParseCharacterListPatternAtEndUnfinished)
{
    auto tokens = parseGlob("world[abc");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], String, "world");
    ASSERT_TOKEN_EQ3(tokens[1], CharacterList, "abc");
}

TEST(Glob, ParseCharacterRangeList1Pattern)
{
    auto tokens = parseGlob("w[ad-g]rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ3(tokens[1], CharacterList, "adefg");
    ASSERT_TOKEN_EQ3(tokens[2], String, "rld");
}

TEST(Glob, ParseCharacterRangeList2Pattern)
{
    auto tokens = parseGlob("w[-gad]rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ3(tokens[1], CharacterList, "-gad");
    ASSERT_TOKEN_EQ3(tokens[2], String, "rld");
}

TEST(Glob, ParseCharacterRangeList3Pattern)
{
    auto tokens = parseGlob("w[gad-]rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ3(tokens[1], CharacterList, "gad-");
    ASSERT_TOKEN_EQ3(tokens[2], String, "rld");
}

TEST(Glob, ParseCharacterRangeList4Pattern)
{
    auto tokens = parseGlob("w[]gad]rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ3(tokens[1], CharacterList, "]gad");
    ASSERT_TOKEN_EQ3(tokens[2], String, "rld");
}

TEST(Glob, ParseCharacterRangeList5Pattern)
{
    auto tokens = parseGlob("w[ga]d]rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ3(tokens[1], CharacterList, "ga");
    ASSERT_TOKEN_EQ3(tokens[2], String, "d]rld");
}

TEST(Glob, ParseCharacterInverseRangeList1Pattern)
{
    auto tokens = parseGlob("w[^ad-g]rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ3(tokens[1], InverseCharacterList, "/adefg");
    ASSERT_TOKEN_EQ3(tokens[2], String, "rld");
}

TEST(Glob, ParseCharacterInverseRangeList2Pattern)
{
    auto tokens = parseGlob("w[^-adg]rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ3(tokens[1], InverseCharacterList, "/-adg");
    ASSERT_TOKEN_EQ3(tokens[2], String, "rld");
}

TEST(Glob, ParseCharacterInverseRangeList3Pattern)
{
    auto tokens = parseGlob("w[^]adg]rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ3(tokens[1], InverseCharacterList, "/]adg");
    ASSERT_TOKEN_EQ3(tokens[2], String, "rld");
}

TEST(Glob, ParseStringPattern)
{
    auto tokens = parseGlob("w{foo,bar,baz}rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w");
    ASSERT_TOKEN_EQ5(tokens[1], StringList, "foo", "bar", "baz");
    ASSERT_TOKEN_EQ3(tokens[2], String, "rld");
}

TEST(Glob, ParseStringPatternAtBegin)
{
    auto tokens = parseGlob("{foo,bar,baz}world");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ5(tokens[0], StringList, "foo", "bar", "baz");
    ASSERT_TOKEN_EQ3(tokens[1], String, "world");
}

TEST(Glob, ParseStringPatternAtEnd)
{
    auto tokens = parseGlob("world{foo,bar,baz}");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], String, "world");
    ASSERT_TOKEN_EQ5(tokens[1], StringList, "foo", "bar", "baz");
}

TEST(Glob, ParseStringPatternAtEndUnfinished1)
{
    auto tokens = parseGlob("world{foo,bar,baz");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], String, "world");
    ASSERT_TOKEN_EQ5(tokens[1], StringList, "foo", "bar", "baz");
}

TEST(Glob, ParseStringPatternAtEndUnfinished2)
{
    auto tokens = parseGlob("world{foo,bar,");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], String, "world");
    ASSERT_TOKEN_EQ5(tokens[1], StringList, "foo", "bar", "");
}

TEST(Glob, ParseEscapePatternStar)
{
    auto tokens = parseGlob("w\\*rld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w*rld");
}

TEST(Glob, ParseEscapePatternQuestion)
{
    auto tokens = parseGlob("w\\?rld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w?rld");
}

TEST(Glob, ParseEscapePatternBracket)
{
    auto tokens = parseGlob("w\\[rld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w[rld");
}

TEST(Glob, ParseEscapePatternBrace)
{
    auto tokens = parseGlob("w\\{rld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w{rld");
}

TEST(Glob, ParseEscapePatternBackSlash)
{
    auto tokens = parseGlob("w\\\\rld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], String, "w\\rld");
}

TEST(Glob, ParseEscapePatternO)
{
    auto tokens = parseGlob("w\\orld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], String, "world");
}

TEST(Glob, ParseEscapePatternAtEnd)
{
    auto tokens = parseGlob("world\\");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], String, "world");
}

TEST(Glob, MatchStar)
{
    ASSERT_EQ(matchGlob("*bar", "foobar"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("*bar", "foobarbaz"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("*bar", "bar"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("*bar/baz", "foobar"), glob_match_result_t::Partial);

    ASSERT_EQ(matchGlob("foo*", "foobar"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("foo*", "foo"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("foo*", "fo"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("foo*/baz", "foobar"), glob_match_result_t::Partial);

    ASSERT_EQ(matchGlob("foo*baz", "foobarbaz"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("foo*baz", "foobaz"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("foo*baz", "fobaz"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("foo*baz", "foobarbaz"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("foo*baz/tree", "foobarbaz"), glob_match_result_t::Partial);
    ASSERT_EQ(matchGlob("foo*baz/tree", "foobaz"), glob_match_result_t::Partial);

    ASSERT_EQ(matchGlob("foo/*/baz", "foo/bar/baz"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("foo/*/baz", "foo/bar1/bar2/baz"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("foo/*/baz", "foo/bar1"), glob_match_result_t::Partial);
}

TEST(Glob, MatchDoubleStar)
{
    ASSERT_EQ(matchGlob("**bar", "foobar"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("**bar", "foobarbaz"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("**bar", "bar"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("**bar/baz", "foobar"), glob_match_result_t::Partial);

    ASSERT_EQ(matchGlob("foo**", "foobar"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("foo**", "foo"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("foo**", "fo"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("foo**/baz", "foobar"), glob_match_result_t::Partial);

    ASSERT_EQ(matchGlob("foo**baz", "foobarbaz"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("foo**baz", "foobaz"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("foo**baz", "fobaz"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("foo**baz", "foobarbaz"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("foo**baz/tree", "foobarbaz"), glob_match_result_t::Partial);
    ASSERT_EQ(matchGlob("foo**baz/tree", "foobaz"), glob_match_result_t::Partial);

    ASSERT_EQ(matchGlob("foo/**/baz", "foo/bar/baz"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("foo/**/baz", "foo/bar1/bar2/baz"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("foo/**/baz", "foo/baz"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("foo/**/baz", "foo/bar1"), glob_match_result_t::Partial);
    ASSERT_EQ(matchGlob("foo/**/baz", "foo"), glob_match_result_t::Partial);
}

TEST(Glob, MatchQuestion)
{
    ASSERT_EQ(matchGlob("?ar", "bar"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("?ar", "ar"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("?ar", "obar"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("?/baz", "f"), glob_match_result_t::Partial);

    ASSERT_EQ(matchGlob("fo?", "foo"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("fo?", "fo"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("fo?", "foop"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("fo?/baz", "foo"), glob_match_result_t::Partial);

    ASSERT_EQ(matchGlob("f?o", "foo"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("f?o", "fo"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("f?o", "fooo"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("f?o/tree", "foo"), glob_match_result_t::Partial);

    ASSERT_EQ(matchGlob("foo/??baz", "foo/b/baz"), glob_match_result_t::No);
}

TEST(Glob, MatchBrackets)
{
    ASSERT_EQ(matchGlob("[abc]ar", "bar"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("[abc]ar", "ar"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("[abc]ar", "obar"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("[abc]ar/baz", "bar"), glob_match_result_t::Partial);

    ASSERT_EQ(matchGlob("fo[abc]", "fob"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("fo[abc]", "foo"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("fo[abc]", "fo"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("fo[abc]", "fobp"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("fo[abc]", "foop"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("fo[abc]/baz", "fob"), glob_match_result_t::Partial);

    ASSERT_EQ(matchGlob("f[abc]o", "fbo"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("f[abc]o", "fb"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("f[abc]o", "fboo"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("f[abc]o/tree", "fbo"), glob_match_result_t::Partial);
}

TEST(Glob, MatchRange)
{
    ASSERT_EQ(matchGlob("[a-c]ar", "bar"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("[a-c]ar", "ar"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("[a-c]ar", "obar"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("[a-c]ar/baz", "bar"), glob_match_result_t::Partial);

    ASSERT_EQ(matchGlob("fo[a-c]", "fob"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("fo[a-c]", "foo"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("fo[a-c]", "fo"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("fo[a-c]", "fobp"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("fo[a-c]", "foop"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("fo[a-c]/baz", "fob"), glob_match_result_t::Partial);

    ASSERT_EQ(matchGlob("f[a-c]o", "fbo"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("f[a-c]o", "fb"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("f[a-c]o", "fboo"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("f[a-c]o/tree", "fbo"), glob_match_result_t::Partial);
}

TEST(Glob, MatchBraces)
{
    ASSERT_EQ(matchGlob("{12,23,1256}ar", "12ar"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("{12,23,1256}ar", "125ar"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("{12,23,1256}ar", "1256ar"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("{12,23,1256}ar", "ar"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("{12,23,1256}ar", "o12ar"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("{12,23,1256}ar/baz", "12ar"), glob_match_result_t::Partial);

    ASSERT_EQ(matchGlob("fo{12,23,1256}", "fo12"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("fo{12,23,1256}", "foo"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("fo{12,23,1256}", "fo"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("fo{12,23,1256}", "fo12p"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("fo{12,23,1256}", "foop"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("fo{12,23,1256}/baz", "fo12"), glob_match_result_t::Partial);

    ASSERT_EQ(matchGlob("f{12,23,1256}o", "f12o"), glob_match_result_t::Match);
    ASSERT_EQ(matchGlob("f{12,23,1256}o", "f23"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("f{12,23,1256}o", "f23oo"), glob_match_result_t::No);
    ASSERT_EQ(matchGlob("f{12,23,1256}o/tree", "f23o"), glob_match_result_t::Partial);
}

TEST(Glob, BasePath)
{
    ASSERT_EQ(basePathOfGlob("foo/bar/baz*"), "foo/bar");
    ASSERT_EQ(basePathOfGlob("foo/bar/*"), "foo/bar");
    ASSERT_EQ(basePathOfGlob("/foo/bar/baz*"), "/foo/bar");
    ASSERT_EQ(basePathOfGlob("/foo/bar/*"), "/foo/bar");
    ASSERT_EQ(basePathOfGlob("/foo*"), "/");
    ASSERT_EQ(basePathOfGlob("/*"), "/");
}
