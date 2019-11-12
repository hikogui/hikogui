// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/glob.hpp"
#include "TTauri/Foundation/required.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace TTauri;

#define ASSERT_TOKEN_EQ2(token, name)\
    do {\
        let expected = glob_token_t{glob_token_type_t::name};\
        ASSERT_EQ(token, expected);\
    } while (false)

#define ASSERT_TOKEN_EQ3(token, name, value)\
    do {\
        let expected = glob_token_t{glob_token_type_t::name, value};\
        ASSERT_EQ(token, expected);\
    } while (false)

#define ASSERT_TOKEN_EQ5(token, name, value1, value2, value3)\
    do {\
        let expected = glob_token_t{glob_token_type_t::name, std::vector<std::string>{value1, value2, value3}};\
        ASSERT_EQ(token, expected);\
    } while (false)

TEST(Glob, ParseNoPattern) {
    auto tokens = parseGlob("world");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "world");
}

TEST(Glob, ParseSlashPattern) {
    auto tokens = parseGlob("w/rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "w");
    ASSERT_TOKEN_EQ2(tokens[1], Seperator);
    ASSERT_TOKEN_EQ3(tokens[2], Choice, "rld");
}

TEST(Glob, ParseSlashPatternAtEnd) {
    auto tokens = parseGlob("w/");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "w");
    ASSERT_TOKEN_EQ2(tokens[1], Seperator);
}

TEST(Glob, ParseSlashPatternAtBegin) {
    auto tokens = parseGlob("/world");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ2(tokens[0], Seperator);
    ASSERT_TOKEN_EQ3(tokens[1], Choice, "world");
}

TEST(Glob, ParseStarPattern) {
    auto tokens = parseGlob("w*rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyString);
    ASSERT_TOKEN_EQ3(tokens[2], Choice, "rld");
}

TEST(Glob, ParseStarPatternAtEnd) {
    auto tokens = parseGlob("w*");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyString);
}

TEST(Glob, ParseStarPatternAtBegin) {
    auto tokens = parseGlob("*world");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ2(tokens[0], AnyString);
    ASSERT_TOKEN_EQ3(tokens[1], Choice, "world");
}

TEST(Glob, ParseDoubleStarPattern) {
    auto tokens = parseGlob("w**rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyDirectory);
    ASSERT_TOKEN_EQ3(tokens[2], Choice, "rld");
}

TEST(Glob, ParseDoubleStarPatternAtEnd) {
    auto tokens = parseGlob("w**");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyDirectory);
}

TEST(Glob, ParseDoubleStarPatternAtBegin) {
    auto tokens = parseGlob("**world");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ2(tokens[0], AnyDirectory);
    ASSERT_TOKEN_EQ3(tokens[1], Choice, "world");
}

TEST(Glob, ParseQuestionPattern) {
    auto tokens = parseGlob("w?rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyCharacter);
    ASSERT_TOKEN_EQ3(tokens[2], Choice, "rld");
}

TEST(Glob, ParseQuestionPatternAtEnd) {
    auto tokens = parseGlob("w?");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyCharacter);
}

TEST(Glob, ParseQuestionPatternAtBegin) {
    auto tokens = parseGlob("?world");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ2(tokens[0], AnyCharacter);
    ASSERT_TOKEN_EQ3(tokens[1], Choice, "world");
}

TEST(Glob, ParseCharacterListPattern) {
    auto tokens = parseGlob("w[abc]rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "w");
    ASSERT_TOKEN_EQ5(tokens[1], Choice, "a", "b", "c");
    ASSERT_TOKEN_EQ3(tokens[2], Choice, "rld");
}

TEST(Glob, ParseCharacterListPatternAtBegin) {
    auto tokens = parseGlob("[abc]world");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ5(tokens[0], Choice, "a", "b", "c");
    ASSERT_TOKEN_EQ3(tokens[1], Choice, "world");
}

TEST(Glob, ParseCharacterListPatternAtEnd) {
    auto tokens = parseGlob("world[abc]");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "world");
    ASSERT_TOKEN_EQ5(tokens[1], Choice, "a", "b", "c");
}

TEST(Glob, ParseCharacterListPatternAtEndUnfinished) {
    auto tokens = parseGlob("world[abc");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "world");
    ASSERT_TOKEN_EQ5(tokens[1], Choice, "a", "b", "c");
}

TEST(Glob, ParseStringListPattern) {
    auto tokens = parseGlob("w{foo,bar,baz}rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "w");
    ASSERT_TOKEN_EQ5(tokens[1], Choice, "foo", "bar", "baz");
    ASSERT_TOKEN_EQ3(tokens[2], Choice, "rld");
}

TEST(Glob, ParseStringListPatternAtBegin) {
    auto tokens = parseGlob("{foo,bar,baz}world");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ5(tokens[0], Choice, "foo", "bar", "baz");
    ASSERT_TOKEN_EQ3(tokens[1], Choice, "world");
}

TEST(Glob, ParseStringListPatternAtEnd) {
    auto tokens = parseGlob("world{foo,bar,baz}");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "world");
    ASSERT_TOKEN_EQ5(tokens[1], Choice, "foo", "bar", "baz");
}

TEST(Glob, ParseStringListPatternAtEndUnfinished1) {
    auto tokens = parseGlob("world{foo,bar,baz");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "world");
    ASSERT_TOKEN_EQ5(tokens[1], Choice, "foo", "bar", "baz");
}

TEST(Glob, ParseStringListPatternAtEndUnfinished2) {
    auto tokens = parseGlob("world{foo,bar,");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "world");
    ASSERT_TOKEN_EQ5(tokens[1], Choice, "foo", "bar", "");
}

TEST(Glob, ParseEscapePatternStar) {
    auto tokens = parseGlob("w\\*rld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "w*rld");
}

TEST(Glob, ParseEscapePatternQuestion) {
    auto tokens = parseGlob("w\\?rld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "w?rld");
}

TEST(Glob, ParseEscapePatternBracket) {
    auto tokens = parseGlob("w\\[rld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "w[rld");
}

TEST(Glob, ParseEscapePatternBrace) {
    auto tokens = parseGlob("w\\{rld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "w{rld");
}

TEST(Glob, ParseEscapePatternBackSlash) {
    auto tokens = parseGlob("w\\\\rld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "w\\rld");
}

TEST(Glob, ParseEscapePatternO) {
    auto tokens = parseGlob("w\\orld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "world");
}

TEST(Glob, ParseEscapePatternAtEnd) {
    auto tokens = parseGlob("world\\");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "world");
}

TEST(Glob, MatchStar) {
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

TEST(Glob, MatchDoubleStar) {
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
    ASSERT_EQ(matchGlob("foo/**/baz", "foo/bar1"), glob_match_result_t::Partial);
}

TEST(Glob, MatchQuestion) {
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
