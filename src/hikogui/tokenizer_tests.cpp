// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "tokenizer.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <array>

using namespace std;
using namespace hi;

#define ASSERT_TOKEN_EQ(nextToken, name, value) \
    do { \
        hilet expectedToken = token_t{tokenizer_name_t::name, value}; \
        ASSERT_EQ(nextToken, expectedToken); \
    } while (false)

TEST(Tokenizer, ParseInteger1)
{
    auto str = "++12345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "12345");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseInteger2)
{
    auto str = "+++2345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "+2345");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseInteger3)
{
    auto str = "++-2345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "-2345");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseInteger4)
{
    auto str = "++02345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "02345");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseInteger5)
{
    auto str = "++0x345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "0x345");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseInteger6)
{
    auto str = "+++0345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "+0345");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseInteger7)
{
    auto str = "++-0345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "-0345");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseInteger8)
{
    auto str = "+++0x45++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "+0x45");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseInteger9)
{
    auto str = "++-0x45++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], IntegerLiteral, "-0x45");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseDashedNumber)
{
    auto str = "++2019-12-22++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], DateLiteral, "2019-12-22");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseFloat1)
{
    auto str = "++12.45++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, "12.45");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseFloat2)
{
    auto str = "+++2.45++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, "+2.45");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseFloat3)
{
    auto str = "++-2.45++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, "-2.45");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseFloat4)
{
    auto str = "++.2345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, ".2345");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseFloat5)
{
    auto str = "+++.345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, "+.345");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseFloat6)
{
    auto str = "++-.345++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, "-.345");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseFloat7)
{
    auto str = "++1234.++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, "1234.");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseFloat8)
{
    auto str = "++1.3e5++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, "1.3e5");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseFloat9)
{
    auto str = "++1.e45++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], FloatLiteral, "1.e45");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseString1)
{
    auto str = "++\"2\\\"4\"++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], StringLiteral, "2\"4");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseString2)
{
    auto str = "++\"2\\\n4\"++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], StringLiteral, "2\n4");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseString3)
{
    auto str = "++\"2\n4\"++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], ErrorLFInString, "\n");
}

TEST(Tokenizer, ParseString4)
{
    auto str = "++\"234";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], ErrorEOTInString, "234");
}

TEST(Tokenizer, ParseEmptyString)
{
    auto str = "++\"\"++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], StringLiteral, "");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseBlockString1)
{
    auto str = "++\"\"\"foo\nbar\"\"\"++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], StringLiteral, "foo\nbar");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseBlockString2)
{
    auto str = "++\"\"\"foo\"bar\"\"\"++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], StringLiteral, "foo\"bar");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseBlockString3)
{
    auto str = "++\"\"\"foo\"\"bar\"\"\"++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], StringLiteral, "foo\"\"bar");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseBlockString4)
{
    auto str = "++\"\"\"foo\\\nbar\"\"\"++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], StringLiteral, "foo\nbar");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseName)
{
    auto str = "++_Foo_++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], Name, "_Foo_");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseLiteral)
{
    auto str = "++.++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], Operator, ".");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseSlash)
{
    auto str = "++ / ++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], Operator, "/");
    ASSERT_TOKEN_EQ(tokens[2], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}

TEST(Tokenizer, ParseWhitespace)
{
    auto str = "++     ++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[2], End, "");
}

TEST(Tokenizer, ParseLineComment1a)
{
    auto str = "++//45\n++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[2], End, "");
}

TEST(Tokenizer, ParseLineComment1b)
{
    auto str = "{\n    foo;\n     //bar;\n}";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "{");
    ASSERT_TOKEN_EQ(tokens[1], Name, "foo");
    ASSERT_TOKEN_EQ(tokens[2], Operator, ";");
    ASSERT_TOKEN_EQ(tokens[3], Operator, "}");
    ASSERT_TOKEN_EQ(tokens[4], End, "");
}

TEST(Tokenizer, ParseLineComment2)
{
    auto str = "++#345\n++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[2], End, "");
}

TEST(Tokenizer, ParseBlockComment2)
{
    auto str = "++/*3*/++";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[1], Operator, "++");
    ASSERT_TOKEN_EQ(tokens[2], End, "");
}

TEST(Tokenizer, ParseFQName)
{
    auto str = "creditor.mc-clown";
    auto v = std::string_view(str);
    auto tokens = parseTokens(v);
    ASSERT_TOKEN_EQ(tokens[0], Name, "creditor");
    ASSERT_TOKEN_EQ(tokens[1], Operator, ".");
    ASSERT_TOKEN_EQ(tokens[2], Name, "mc-clown");
    ASSERT_TOKEN_EQ(tokens[3], End, "");
}
