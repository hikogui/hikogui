// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unicode_normalization.hpp"
#include "../file/file_view.hpp"
#include "../strings.hpp"
#include "../generator.hpp"
#include "../charconv.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <span>
#include <format>

using namespace std;
using namespace hi;

/*! A test defined in NormalizationTests.txt.
 *
 * CONFORMANCE:
 * 1. The following invariants must be true for all conformant implementations
 *
 *    NFC
 *      c2 ==  toNFC(c1) ==  toNFC(c2) ==  toNFC(c3)
 *      c4 ==  toNFC(c4) ==  toNFC(c5)
 *
 *    NFD
 *      c3 ==  toNFD(c1) ==  toNFD(c2) ==  toNFD(c3)
 *      c5 ==  toNFD(c4) ==  toNFD(c5)
 *
 *    NFKC
 *      c4 == toNFKC(c1) == toNFKC(c2) == toNFKC(c3) == toNFKC(c4) == toNFKC(c5)
 *
 *    NFKD
 *      c5 == toNFKD(c1) == toNFKD(c2) == toNFKD(c3) == toNFKD(c4) == toNFKD(c5)
 *
 * 2. For every code point X assigned in this version of Unicode that is not specifically
 *    listed in Part 1, the following invariants must be true for all conformant
 *    implementations:
 *
 *      X == toNFC(X) == toNFD(X) == toNFKC(X) == toNFKD(X)
 */
struct NormalizationTest {
    std::u32string c1;
    std::u32string c2;
    std::u32string c3;
    std::u32string c4;
    std::u32string c5;
    size_t line_nr;
    std::string comment;
};

std::u32string parseNormalizationTest_column(std::string_view column)
{
    std::u32string r;

    auto codePointStrings = split_view(column);
    for (hilet codePointString : codePointStrings) {
        r += static_cast<char32_t>(from_string<uint32_t>(codePointString, 16));
    }
    return r;
}

std::optional<NormalizationTest> parseNormalizationTest_line(std::string_view line, size_t line_nr)
{
    auto r = std::optional<NormalizationTest>{};

    hilet split_line = split_view(line, '#');
    if (split_line.size() < 2) {
        return r;
    }
    hilet columns = split_view(split_line[0], ';');
    if (columns.size() < 6) {
        return r;
    }

    r.emplace(
        parseNormalizationTest_column(columns[0]),
        parseNormalizationTest_column(columns[1]),
        parseNormalizationTest_column(columns[2]),
        parseNormalizationTest_column(columns[3]),
        parseNormalizationTest_column(columns[4]),
        line_nr,
        std::format("{}: {}", line_nr, split_line[1]));

    return r;
}

generator<NormalizationTest> parseNormalizationTests()
{
    hilet view = file_view(std::filesystem::path{"NormalizationTest.txt"});
    hilet test_data = as_string_view(view);

    size_t line_nr = 0;
    for (hilet line : split_view(test_data, '\n')) {
        if (hilet optionalTest = parseNormalizationTest_line(line, ++line_nr)) {
            co_yield *optionalTest;
        }
    }
}

// CONFORMANCE:
// 1. The following invariants must be true for all conformant implementations
//
//    NFC
//      c2 ==  toNFC(c1) ==  toNFC(c2) ==  toNFC(c3)
//      c4 ==  toNFC(c4) ==  toNFC(c5)
//
//    NFD
//      c3 ==  toNFD(c1) ==  toNFD(c2) ==  toNFD(c3)
//      c5 ==  toNFD(c4) ==  toNFD(c5)
//
//    NFKC
//      c4 == toNFKC(c1) == toNFKC(c2) == toNFKC(c3) == toNFKC(c4) == toNFKC(c5)
//
//    NFKD
//      c5 == toNFKD(c1) == toNFKD(c2) == toNFKD(c3) == toNFKD(c4) == toNFKD(c5)
//
// 2. For every code point X assigned in this version of Unicode that is not specifically
//    listed in Part 1, the following invariants must be true for all conformant
//    implementations:
//
//      X == toNFC(X) == toNFD(X) == toNFKC(X) == toNFKD(X)

TEST(unicode_normalization, unicode_NFC_colon)
{
    ASSERT_TRUE(unicode_NFC(hi::to_u32string("Audio device:")) == hi::to_u32string("Audio device:"));
    ASSERT_TRUE(unicode_NFD(hi::to_u32string("Audio device:")) == hi::to_u32string("Audio device:"));
}

TEST(unicode_normalization, NFC)
{
    for (hilet& test : parseNormalizationTests()) {
        ASSERT_TRUE(unicode_NFC(test.c1) == test.c2) << test.comment;
        ASSERT_TRUE(unicode_NFC(test.c2) == test.c2) << test.comment;
        ASSERT_TRUE(unicode_NFC(test.c3) == test.c2) << test.comment;
        ASSERT_TRUE(unicode_NFC(test.c4) == test.c4) << test.comment;
        ASSERT_TRUE(unicode_NFC(test.c5) == test.c4) << test.comment;
    }
}

TEST(unicode_normalization, NFKC)
{
    for (hilet& test : parseNormalizationTests()) {
        ASSERT_TRUE(unicode_NFKC(test.c1) == test.c4) << test.comment;
        ASSERT_TRUE(unicode_NFKC(test.c2) == test.c4) << test.comment;
        ASSERT_TRUE(unicode_NFKC(test.c3) == test.c4) << test.comment;
        ASSERT_TRUE(unicode_NFKC(test.c4) == test.c4) << test.comment;
        ASSERT_TRUE(unicode_NFKC(test.c5) == test.c4) << test.comment;
    }
}

TEST(unicode_normalization, NFD)
{
    for (hilet& test : parseNormalizationTests()) {
        ASSERT_TRUE(unicode_NFD(test.c1) == test.c3) << test.comment;
        ASSERT_TRUE(unicode_NFD(test.c2) == test.c3) << test.comment;
        ASSERT_TRUE(unicode_NFD(test.c3) == test.c3) << test.comment;
        ASSERT_TRUE(unicode_NFD(test.c4) == test.c5) << test.comment;
        ASSERT_TRUE(unicode_NFD(test.c5) == test.c5) << test.comment;
    }
}

TEST(unicode_normalization, NFKD)
{
    for (hilet& test : parseNormalizationTests()) {
        ASSERT_TRUE(unicode_NFKD(test.c1) == test.c5) << test.comment;
        ASSERT_TRUE(unicode_NFKD(test.c2) == test.c5) << test.comment;
        ASSERT_TRUE(unicode_NFKD(test.c3) == test.c5) << test.comment;
        ASSERT_TRUE(unicode_NFKD(test.c4) == test.c5) << test.comment;
        ASSERT_TRUE(unicode_NFKD(test.c5) == test.c5) << test.comment;
    }
}

TEST(unicode_normalization, Invariant)
{
    auto previouslyTestedCodePoints = std::vector<bool>(0x11'0000, false);
    for (hilet& test : parseNormalizationTests()) {
        for (hilet& c : test.c1) {
            previouslyTestedCodePoints[c] = true;
        }
        for (hilet& c : test.c2) {
            previouslyTestedCodePoints[c] = true;
        }
        for (hilet& c : test.c3) {
            previouslyTestedCodePoints[c] = true;
        }
        for (hilet& c : test.c4) {
            previouslyTestedCodePoints[c] = true;
        }
        for (hilet& c : test.c5) {
            previouslyTestedCodePoints[c] = true;
        }
    }

    for (char32_t i = 0; i < previouslyTestedCodePoints.size(); i++) {
        if (!previouslyTestedCodePoints[i]) {
            hilet str = std::u32string(1, i);

            ASSERT_TRUE(unicode_NFD(str) == str) << "NFD code-point: " << static_cast<int>(i);
            ASSERT_TRUE(unicode_NFC(str) == str) << "NFC code-point: " << static_cast<int>(i);
            ASSERT_TRUE(unicode_NFKD(str) == str) << "NFKD code-point: " << static_cast<int>(i);
            ASSERT_TRUE(unicode_NFKC(str) == str) << "NFKC code-point: " << static_cast<int>(i);
        }
    }
}
