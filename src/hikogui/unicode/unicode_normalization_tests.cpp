// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/unicode/unicode_normalization.hpp"
#include "hikogui/file_view.hpp"
#include "hikogui/strings.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <span>
#include <format>

#if HI_BUILD_TYPE == HI_BT_RELEASE
#define RUN_ALL_TESTS 1
#endif

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
    std::string comment;
};

std::u32string parseNormalizationTest_column(std::string_view column)
{
    std::u32string r;

    auto codePointStrings = split(column);
    for (hilet codePointString : codePointStrings) {
        auto codePoint = static_cast<char32_t>(std::stoi(std::string(codePointString), nullptr, 16));
        r += codePoint;
    }
    return r;
}

std::optional<NormalizationTest> parseNormalizationTest_line(std::string_view line)
{
    hilet split_line = split(line, '#');
    if (split_line.size() < 2) {
        return {};
    }
    hilet columns = split(split_line[0], ';');
    if (columns.size() < 6) {
        return {};
    }

    NormalizationTest r;
    r.c1 = parseNormalizationTest_column(columns[0]);
    r.c2 = parseNormalizationTest_column(columns[1]);
    r.c3 = parseNormalizationTest_column(columns[2]);
    r.c4 = parseNormalizationTest_column(columns[3]);
    r.c5 = parseNormalizationTest_column(columns[4]);
    r.comment = split_line[1];

    return r;
}

std::vector<NormalizationTest> parseNormalizationTests()
{
    hilet view = file_view(URL("file:NormalizationTest.txt"));
    hilet test_data = view.string_view();

    std::vector<NormalizationTest> r;
    for (hilet line : split(test_data, '\n')) {
        if (hilet optionalTest = parseNormalizationTest_line(line)) {
            r.push_back(*optionalTest);
        }
    }
    return r;
}

class unicode_normalization : public ::testing::Test {
protected:
    void SetUp() override
    {
        normalizationTests = parseNormalizationTests();
    }

    std::vector<NormalizationTest> normalizationTests;
};

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

TEST_F(unicode_normalization, unicode_NFC_colon)
{
    ASSERT_TRUE(unicode_NFC(hi::to_u32string("Audio device:")) == hi::to_u32string("Audio device:"));
    ASSERT_TRUE(unicode_NFD(hi::to_u32string("Audio device:")) == hi::to_u32string("Audio device:"));
}

TEST_F(unicode_normalization, toNFC_c1)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFC(test.c1) == test.c2) << test.comment;
    }
}

#if defined(RUN_ALL_TESTS)
TEST_F(unicode_normalization, toNFC_c2)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFC(test.c2) == test.c2) << test.comment;
    }
}

TEST_F(unicode_normalization, toNFC_c3)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFC(test.c3) == test.c2) << test.comment;
    }
}

TEST_F(unicode_normalization, toNFC_c4)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFC(test.c4) == test.c4) << test.comment;
    }
}

TEST_F(unicode_normalization, toNFC_c5)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFC(test.c5) == test.c4) << test.comment;
    }
}
#endif

TEST_F(unicode_normalization, toNFKC_c1)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFKC(test.c1) == test.c4) << test.comment;
    }
}

#if defined(RUN_ALL_TESTS)
TEST_F(unicode_normalization, toNFKC_c2)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFKC(test.c2) == test.c4) << test.comment;
    }
}

TEST_F(unicode_normalization, toNFKC_c3)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFKC(test.c3) == test.c4) << test.comment;
    }
}

TEST_F(unicode_normalization, toNFKC_c4)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFKC(test.c4) == test.c4) << test.comment;
    }
}

TEST_F(unicode_normalization, toNFKC_c5)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFKC(test.c5) == test.c4) << test.comment;
    }
}
#endif

TEST_F(unicode_normalization, toNFD_c1)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFD(test.c1) == test.c3) << test.comment;
    }
}

#if defined(RUN_ALL_TESTS)
TEST_F(unicode_normalization, toNFD_c2)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFD(test.c2) == test.c3) << test.comment;
    }
}

TEST_F(unicode_normalization, toNFD_c3)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFD(test.c3) == test.c3) << test.comment;
    }
}

TEST_F(unicode_normalization, toNFD_c4)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFD(test.c4) == test.c5) << test.comment;
    }
}

TEST_F(unicode_normalization, toNFD_c5)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFD(test.c5) == test.c5) << test.comment;
    }
}
#endif

TEST_F(unicode_normalization, toNFKD_c1)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFKD(test.c1) == test.c5) << test.comment;
    }
}

#if defined(RUN_ALL_TESTS)
TEST_F(unicode_normalization, toNFKD_c2)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFKD(test.c2) == test.c5) << test.comment;
    }
}

TEST_F(unicode_normalization, toNFKD_c3)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFKD(test.c3) == test.c5) << test.comment;
    }
}

TEST_F(unicode_normalization, toNFKD_c4)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFKD(test.c4) == test.c5) << test.comment;
    }
}

TEST_F(unicode_normalization, toNFKD_c5)
{
    for (hilet &test : normalizationTests) {
        ASSERT_TRUE(unicode_NFKD(test.c5) == test.c5) << test.comment;
    }
}
#endif

#if defined(RUN_ALL_TESTS)
TEST_F(unicode_normalization, Invariant)
{
    auto previouslyTestedCodePoints = std::vector<bool>(0x11'0000, false);
    for (hilet &test : normalizationTests) {
        for (hilet &c : test.c1) {
            previouslyTestedCodePoints[c] = true;
        }
        for (hilet &c : test.c2) {
            previouslyTestedCodePoints[c] = true;
        }
        for (hilet &c : test.c3) {
            previouslyTestedCodePoints[c] = true;
        }
        for (hilet &c : test.c4) {
            previouslyTestedCodePoints[c] = true;
        }
        for (hilet &c : test.c5) {
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
#endif
