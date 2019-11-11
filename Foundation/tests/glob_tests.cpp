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
        let expected = glob_token_t{glob_token_name_t::name};\
        ASSERT_EQ(token, expected);\
    } while (false)

#define ASSERT_TOKEN_EQ3(token, name, value)\
    do {\
        let expected = glob_token_t{glob_token_name_t::name, value};\
        ASSERT_EQ(token, expected);\
    } while (false)

#define ASSERT_TOKEN_EQ5(token, name, value1, value2, value3)\
    do {\
        let expected = glob_token_t{glob_token_name_t::name, std::vector<std::string>{value1, value2, value3}};\
        ASSERT_EQ(token, expected);\
    } while (false)

TEST(Glob, ParseNoPattern) {
    auto tokens = parseGlobPattern("/hello/world");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/world");
}

TEST(Glob, ParseStarPattern) {
    auto tokens = parseGlobPattern("/hello/w*rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyString);
    ASSERT_TOKEN_EQ3(tokens[2], Choice, "rld");
}

TEST(Glob, ParseStarPatternAtEnd) {
    auto tokens = parseGlobPattern("/hello/w*");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyString);
}

TEST(Glob, ParseStarPatternAtBegin) {
    auto tokens = parseGlobPattern("*/hello/world");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ2(tokens[0], AnyString);
    ASSERT_TOKEN_EQ3(tokens[1], Choice, "/hello/world");
}

TEST(Glob, ParseQuestionPattern) {
    auto tokens = parseGlobPattern("/hello/w?rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyCharacter);
    ASSERT_TOKEN_EQ3(tokens[2], Choice, "rld");
}

TEST(Glob, ParseQuestionPatternAtEnd) {
    auto tokens = parseGlobPattern("/hello/w?");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/w");
    ASSERT_TOKEN_EQ2(tokens[1], AnyCharacter);
}

TEST(Glob, ParseQuestionPatternAtBegin) {
    auto tokens = parseGlobPattern("?/hello/world");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ2(tokens[0], AnyCharacter);
    ASSERT_TOKEN_EQ3(tokens[1], Choice, "/hello/world");
}

TEST(Glob, ParseCharacterListPattern) {
    auto tokens = parseGlobPattern("/hello/w[abc]rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/w");
    ASSERT_TOKEN_EQ5(tokens[1], Choice, "a", "b", "c");
    ASSERT_TOKEN_EQ3(tokens[2], Choice, "rld");
}

TEST(Glob, ParseCharacterListPatternAtBegin) {
    auto tokens = parseGlobPattern("[abc]/hello/world");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ5(tokens[0], Choice, "a", "b", "c");
    ASSERT_TOKEN_EQ3(tokens[1], Choice, "/hello/world");
}

TEST(Glob, ParseCharacterListPatternAtEnd) {
    auto tokens = parseGlobPattern("/hello/world[abc]");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/world");
    ASSERT_TOKEN_EQ5(tokens[1], Choice, "a", "b", "c");
}

TEST(Glob, ParseCharacterListPatternAtEndUnfinished) {
    auto tokens = parseGlobPattern("/hello/world[abc");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/world");
    ASSERT_TOKEN_EQ5(tokens[1], Choice, "a", "b", "c");
}

TEST(Glob, ParseStringListPattern) {
    auto tokens = parseGlobPattern("/hello/w{foo,bar,baz}rld");
    ASSERT_EQ(tokens.size(), 3);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/w");
    ASSERT_TOKEN_EQ5(tokens[1], Choice, "foo", "bar", "baz");
    ASSERT_TOKEN_EQ3(tokens[2], Choice, "rld");
}

TEST(Glob, ParseStringListPatternAtBegin) {
    auto tokens = parseGlobPattern("{foo,bar,baz}/hello/world");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ5(tokens[0], Choice, "foo", "bar", "baz");
    ASSERT_TOKEN_EQ3(tokens[1], Choice, "/hello/world");
}

TEST(Glob, ParseStringListPatternAtEnd) {
    auto tokens = parseGlobPattern("/hello/world{foo,bar,baz}");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/world");
    ASSERT_TOKEN_EQ5(tokens[1], Choice, "foo", "bar", "baz");
}

TEST(Glob, ParseStringListPatternAtEndUnfinished1) {
    auto tokens = parseGlobPattern("/hello/world{foo,bar,baz");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/world");
    ASSERT_TOKEN_EQ5(tokens[1], Choice, "foo", "bar", "baz");
}

TEST(Glob, ParseStringListPatternAtEndUnfinished2) {
    auto tokens = parseGlobPattern("/hello/world{foo,bar,");
    ASSERT_EQ(tokens.size(), 2);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/world");
    ASSERT_TOKEN_EQ5(tokens[1], Choice, "foo", "bar", "");
}

TEST(Glob, ParseEscapePatternStar) {
    auto tokens = parseGlobPattern("/hello/w\\*rld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/w*rld");
}

TEST(Glob, ParseEscapePatternQuestion) {
    auto tokens = parseGlobPattern("/hello/w\\?rld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/w?rld");
}

TEST(Glob, ParseEscapePatternBracket) {
    auto tokens = parseGlobPattern("/hello/w\\[rld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/w[rld");
}

TEST(Glob, ParseEscapePatternBrace) {
    auto tokens = parseGlobPattern("/hello/w\\{rld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/w{rld");
}

TEST(Glob, ParseEscapePatternBackSlash) {
    auto tokens = parseGlobPattern("/hello/w\\\\rld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/w\\rld");
}

TEST(Glob, ParseEscapePatternO) {
    auto tokens = parseGlobPattern("/hello/w\\orld");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/world");
}

TEST(Glob, ParseEscapePatternAtEnd) {
    auto tokens = parseGlobPattern("/hello/world\\");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_TOKEN_EQ3(tokens[0], Choice, "/hello/world");
}