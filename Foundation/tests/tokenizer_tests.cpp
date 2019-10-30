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
        let nextToken = t.getNextToken();\
        let expectedToken = tokenizer::token_t{tokenizer_name_t::name, value, offset};\
        ASSERT_EQ(nextToken, expectedToken);\
    } while (false)

TEST(Tokenizer, ParseInteger1) {
    auto str = "++12345++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "12345", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseInteger2) {
    auto str = "+++2345++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "+2345", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseInteger3) {
    auto str = "++-2345++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "-2345", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseInteger4) {
    auto str = "++02345++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "02345", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseInteger5) {
    auto str = "++0x345++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "0x345", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseInteger6) {
    auto str = "+++0345++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "+0345", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseInteger7) {
    auto str = "++-0345++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "-0345", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseInteger8) {
    auto str = "+++0x45++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "+0x45", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseInteger9) {
    auto str = "++-0x45++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, IntegerLiteral, "-0x45", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFloat1) {
    auto str = "++12.45++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, "12.45", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFloat2) {
    auto str = "+++2.45++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, "+2.45", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFloat3) {
    auto str = "++-2.45++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, "-2.45", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFloat4) {
    auto str = "++.2345++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, ".2345", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFloat5) {
    auto str = "+++.345++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, "+.345", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFloat6) {
    auto str = "++-.345++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, "-.345", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFloat7) {
    auto str = "++1234.++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, "1234.", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFloat8) {
    auto str = "++1.3e5++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, "1.3e5", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFloat9) {
    auto str = "++1.e45++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, FloatLiteral, "1.e45", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseString1) {
    auto str = "++\"2\\\"4\"++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, StringLiteral, "2\"4", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 8);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 10);
}

TEST(Tokenizer, ParseString2) {
    auto str = "++\"2\\\n4\"++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, StringLiteral, "2\n4", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 8);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 10);
}

TEST(Tokenizer, ParseString3) {
    auto str = "++\"2\n4\"++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, ErrorLFInString, "\n", v.begin() + 4);
}

TEST(Tokenizer, ParseString4) {
    auto str = "++\"234";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, ErrorEOTInString, "234", v.begin() + 2);
}


TEST(Tokenizer, ParseName) {
    auto str = "++_Foo_++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, Name, "_Foo_", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseLiteral) {
    auto str = "++.++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, ".", v.begin() + 2);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 3);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 5);
}

TEST(Tokenizer, ParseWhitespace) {
    auto str = "++     ++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseLineComment1) {
    auto str = "++//45\n++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseLineComment2) {
    auto str = "++#345\n++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseBlockComment2) {
    auto str = "++/*3*/++";
    auto v = std::string_view(str);
    auto t = tokenizer(v);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 0);
    ASSERT_NEXT_TOKEN_EQ(t, Literal, "++", v.begin() + 7);
    ASSERT_NEXT_TOKEN_EQ(t, End, "", v.begin() + 9);
}
