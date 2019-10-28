// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/tokenizer.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <array>

using namespace std;
using namespace TTauri;


#define ASSERT_NEXT_TOKEN_EQ(t, name, value, offset)\
    do {\
        let nextToken = t();\
        let expectedToken = tokenizer_token_t{tokenizer_name_t::name, value, offset};\
        ASSERT_EQ(nextToken, expectedToken);\
    } while (false)

TEST(Tokenizer, ParseInteger1) {
    auto t = tokenizer("++12345++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "12345", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseInteger2) {
    auto t = tokenizer("+++2345++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "+2345", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseInteger3) {
    auto t = tokenizer("++-2345++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "-2345", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseInteger4) {
    auto t = tokenizer("++02345++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "02345", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseInteger5) {
    auto t = tokenizer("++0x345++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "0x345", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseInteger6) {
    auto t = tokenizer("+++0345++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "+0345", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseInteger7) {
    auto t = tokenizer("++-0345++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "-0345", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseInteger8) {
    auto t = tokenizer("+++0x45++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "+0x45", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseInteger9) {
    auto t = tokenizer("++-0x45++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "-0x45", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseFloat1) {
    auto t = tokenizer("++12.45++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, "12.45", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseFloat2) {
    auto t = tokenizer("+++2.45++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, "+2.45", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseFloat3) {
    auto t = tokenizer("++-2.45++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, "-2.45", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseFloat4) {
    auto t = tokenizer("++.2345++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, ".2345", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseFloat5) {
    auto t = tokenizer("+++.345++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, "+.345", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseFloat6) {
    auto t = tokenizer("++-.345++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, "-.345", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseFloat7) {
    auto t = tokenizer("++1234.++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, "1234.", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseFloat8) {
    auto t = tokenizer("++1.3e5++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, "1.3e5", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseFloat9) {
    auto t = tokenizer("++1.e45++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, "1.e45", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseString1) {
    auto t = tokenizer("++\"2\\\"4\"++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, StringLiteral, "2\"4", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 8);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 10);
}

TEST(Tokenizer, ParseString2) {
    auto t = tokenizer("++\"2\\\n4\"++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, StringLiteral, "2\n4", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 8);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 10);
}

TEST(Tokenizer, ParseString3) {
    auto t = tokenizer("++\"2\n4\"++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, ErrorLFInString, "\n", 4);
}

TEST(Tokenizer, ParseString4) {
    auto t = tokenizer("++\"234");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, ErrorEOTInString, "234", 2);
}


TEST(Tokenizer, ParseName) {
    auto t = tokenizer("++_Foo_++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, Name, "_Foo_", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseLiteral) {
    auto t = tokenizer("++.++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, ".", 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 3);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 5);
}

TEST(Tokenizer, ParseWhitespace) {
    auto t = tokenizer("++     ++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseLineComment1) {
    auto t = tokenizer("++//45\n++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseLineComment2) {
    auto t = tokenizer("++#345\n++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}

TEST(Tokenizer, ParseBlockComment2) {
    auto t = tokenizer("++/*3*/++");
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 0);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", 9);
}
