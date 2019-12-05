// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/tokenizer.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <array>

using namespace std;
using namespace TTauri;


#define ASSERT_TOKEN_EQ(nextToken, name, value, offset)\
    do {\
        let expectedToken = token_t{tokenizer_name_t::name, value, offset};\
        ASSERT_EQ(nextToken, expectedToken);\
    } while (false)

TEST(Tokenizer, ParseInteger1) {
    auto str = "++12345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "12345", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseInteger2) {
    auto str = "+++2345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "+2345", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseInteger3) {
    auto str = "++-2345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "-2345", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseInteger4) {
    auto str = "++02345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "02345", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseInteger5) {
    auto str = "++0x345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "0x345", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseInteger6) {
    auto str = "+++0345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "+0345", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseInteger7) {
    auto str = "++-0345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "-0345", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseInteger8) {
    auto str = "+++0x45++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "+0x45", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseInteger9) {
    auto str = "++-0x45++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "-0x45", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseDashedNumber) {
    auto str = "++2019-12-22++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], DateLiteral, "2019-12-22", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 12);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 14);
}

TEST(Tokenizer, ParseFloat1) {
    auto str = "++12.45++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, "12.45", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFloat2) {
    auto str = "+++2.45++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, "+2.45", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFloat3) {
    auto str = "++-2.45++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, "-2.45", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFloat4) {
    auto str = "++.2345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, ".2345", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFloat5) {
    auto str = "+++.345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, "+.345", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFloat6) {
    auto str = "++-.345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, "-.345", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFloat7) {
    auto str = "++1234.++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, "1234.", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFloat8) {
    auto str = "++1.3e5++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, "1.3e5", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFloat9) {
    auto str = "++1.e45++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, "1.e45", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseString1) {
    auto str = "++\"2\\\"4\"++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], StringLiteral, "2\"4", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 8);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 10);
}

TEST(Tokenizer, ParseString2) {
    auto str = "++\"2\\\n4\"++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], StringLiteral, "2\n4", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 8);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 10);
}

TEST(Tokenizer, ParseString3) {
    auto str = "++\"2\n4\"++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], ErrorLFInString, "\n", v.begin() + 4);
}

TEST(Tokenizer, ParseString4) {
    auto str = "++\"234";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], ErrorEOTInString, "234", v.begin() + 2);
}


TEST(Tokenizer, ParseName) {
    auto str = "++_Foo_++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], Name, "_Foo_", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseLiteral) {
    auto str = "++.++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], Literal, ".", v.begin() + 2);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 3);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 5);
}

TEST(Tokenizer, ParseSlash) {
    auto str = "++ / ++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], Literal, "/", v.begin() + 3);
    ASSERT_TOKEN_EQ(tokens[2], Literal, "++", v.begin() + 5);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 7);
}


TEST(Tokenizer, ParseWhitespace) {
    auto str = "++     ++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[2], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseLineComment1) {
    auto str = "++//45\n++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[2], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseLineComment2) {
    auto str = "++#345\n++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[2], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseBlockComment2) {
    auto str = "++/*3*/++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Literal, "++", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], Literal, "++", v.begin() + 7);
    ASSERT_TOKEN_EQ(tokens[2], End, "", v.begin() + 9);
}

TEST(Tokenizer, ParseFQName) {
    auto str = "creditor.mc-clown";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Name, "creditor", v.begin() + 0);
    ASSERT_TOKEN_EQ(tokens[1], Literal, ".", v.begin() + 8);
    ASSERT_TOKEN_EQ(tokens[2], Name, "mc-clown", v.begin() + 9);
    ASSERT_TOKEN_EQ(tokens[3], End, "", v.begin() + 17);
}

