// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/BasicTokenizer.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <array>

using namespace std;
using namespace TTauri;

TEST(BasicTokenizer, ParseInteger) {
    size_t offset = 2;
    auto token = parseToken("++12345++", offset);
    auto expected = Token{TokenName::IntegerLiteral, datum{12345}, 2};
    ASSERT_EQ(offset, 7);
    ASSERT_EQ(token, expected);
}

TEST(BasicTokenizer, ParseFloat) {
    size_t offset = 2;
    auto token = parseToken("++12.45++", offset);
    auto expected = Token{TokenName::FloatLiteral, datum{12.45}, 2};
    ASSERT_EQ(offset, 7);
    ASSERT_EQ(token, expected);
}

TEST(BasicTokenizer, ParseString) {
    {size_t offset = 2;
    auto token = parseToken("++\"2\\\"4\"++", offset);
    auto expected = Token{TokenName::StringLiteral, datum{"\"2\"4"}, 2};
    ASSERT_EQ(offset, 8);
    ASSERT_EQ(token, expected);}

    {size_t offset = 2;
    auto token = parseToken("++'2\"4'++", offset);
    auto expected = Token{TokenName::StringLiteral, datum{"'2\"4"}, 2};
    ASSERT_EQ(offset, 7);
    ASSERT_EQ(token, expected);}

    {size_t offset = 2;
    auto token = parseToken("++'2\\'4'++", offset);
    auto expected = Token{TokenName::StringLiteral, datum{"'2'4"}, 2};
    ASSERT_EQ(offset, 8);
    ASSERT_EQ(token, expected);}

    {size_t offset = 2;
    auto token = parseToken("++`2\"4`++", offset);
    auto expected = Token{TokenName::StringLiteral, datum{"`2\"4"}, 2};
    ASSERT_EQ(offset, 7);
    ASSERT_EQ(token, expected);}
}

TEST(BasicTokenizer, ParseName) {
    size_t offset = 2;
    auto token = parseToken("++_Foo_++", offset);
    auto expected = Token{TokenName::Name, datum{"_Foo_"}, 2};
    ASSERT_EQ(offset, 7);
    ASSERT_EQ(token, expected);
}

TEST(BasicTokenizer, ParseOperator) {
    size_t offset = 2;
    auto token = parseToken("ab+-*-+ab", offset);
    auto expected = Token{TokenName::Operator, datum{"+-*-+"}, 2};
    ASSERT_EQ(offset, 7);
    ASSERT_EQ(token, expected);
}

TEST(BasicTokenizer, ParseOpen) {
    {size_t offset = 2;
    auto token = parseToken("ab((}))ab", offset);
    auto expected = Token{TokenName::Open, datum{"("}, 2};
    ASSERT_EQ(offset, 3);
    ASSERT_EQ(token, expected);}

    {size_t offset = 2;
    auto token = parseToken("ab{(}))ab", offset);
    auto expected = Token{TokenName::Open, datum{"{"}, 2};
    ASSERT_EQ(offset, 3);
    ASSERT_EQ(token, expected);}

    {size_t offset = 2;
    auto token = parseToken("ab[(}))ab", offset);
    auto expected = Token{TokenName::Open, datum{"["}, 2};
    ASSERT_EQ(offset, 3);
    ASSERT_EQ(token, expected);}
}

TEST(BasicTokenizer, ParseClose) {
    {size_t offset = 2;
    auto token = parseToken("ab)(}))ab", offset);
    auto expected = Token{TokenName::Close, datum{")"}, 2};
    ASSERT_EQ(offset, 3);
    ASSERT_EQ(token, expected);}

    {size_t offset = 2;
    auto token = parseToken("ab}(}))ab", offset);
    auto expected = Token{TokenName::Close, datum{"}"}, 2};
    ASSERT_EQ(offset, 3);
    ASSERT_EQ(token, expected);}

    {size_t offset = 2;
    auto token = parseToken("ab](}))ab", offset);
    auto expected = Token{TokenName::Close, datum{"]"}, 2};
    ASSERT_EQ(offset, 3);
    ASSERT_EQ(token, expected);}
}

TEST(BasicTokenizer, ParseWhitespace) {
    size_t offset = 2;
    auto token = parseToken("++     ++", offset);
    auto expected = Token{TokenName::Whitespace, datum{}, 2};
    ASSERT_EQ(offset, 7);
    ASSERT_EQ(token, expected);
}

TEST(BasicTokenizer, ParseComment) {
    {size_t offset = 2;
    auto token = parseToken("++/* */++", offset);
    auto expected = Token{TokenName::Comment, datum{}, 2};
    ASSERT_EQ(offset, 7);
    ASSERT_EQ(token, expected);}

    {size_t offset = 2;
    auto token = parseToken("++// foo\n++", offset);
    auto expected = Token{TokenName::Comment, datum{}, 2};
    ASSERT_EQ(offset, 8);
    ASSERT_EQ(token, expected);}

    {size_t offset = 2;
    auto token = parseToken("++# foo\n++", offset);
    auto expected = Token{TokenName::Comment, datum{}, 2};
    ASSERT_EQ(offset, 7);
    ASSERT_EQ(token, expected);}

    {size_t offset = 2;
    auto token = parseToken("++# foo\r++", offset);
    auto expected = Token{TokenName::Comment, datum{}, 2};
    ASSERT_EQ(offset, 7);
    ASSERT_EQ(token, expected);}

    {size_t offset = 2;
    auto token = parseToken("++# foo\r\n++", offset);
    auto expected = Token{TokenName::Comment, datum{}, 2};
    ASSERT_EQ(offset, 7);
    ASSERT_EQ(token, expected);}

    {size_t offset = 2;
    auto token = parseToken("++# foo\n\n++", offset);
    auto expected = Token{TokenName::Comment, datum{}, 2};
    ASSERT_EQ(offset, 7);
    ASSERT_EQ(token, expected);}

    {size_t offset = 2;
    auto token = parseToken("++# foo\n\r++", offset);
    auto expected = Token{TokenName::Comment, datum{}, 2};
    ASSERT_EQ(offset, 7);
    ASSERT_EQ(token, expected);}
}
