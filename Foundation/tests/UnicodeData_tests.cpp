// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/UnicodeData.hpp"
#include "TTauri/Foundation/FileView.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "data/UnicodeData.bin.inl"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <gsl/gsl>

using namespace std;
using namespace TTauri;


struct NormalizationTest {
    std::u32string source;
    std::u32string NFC;
    std::u32string NFD;
    std::u32string NFKC;
    std::u32string NFKD;
    std::string comment;
};

std::u32string parseNormalizationTest_column(std::string_view column) {
    std::u32string r;

    auto codePointStrings = split(column, " ");
    for (let codePointString: codePointStrings) {
        auto codePoint = static_cast<char32_t>(std::stoi(std::string(codePointString), nullptr, 16));
        r += codePoint;
    }
    return r;
}

std::optional<NormalizationTest> parseNormalizationTest_line(std::string_view line) {
    let split_line = split(line, "#");
    if (split_line.size() < 2) {
        return {};
    }
    let columns = split(split_line[0], ";");
    if (columns.size() < 6) {
        return {};
    }

    NormalizationTest r;
    r.source = parseNormalizationTest_column(columns[0]);
    r.NFC = parseNormalizationTest_column(columns[1]);
    r.NFD = parseNormalizationTest_column(columns[2]);
    r.NFKC = parseNormalizationTest_column(columns[3]);
    r.NFKD = parseNormalizationTest_column(columns[4]);
    r.comment = split_line[1];

    return r;
}

std::vector<NormalizationTest> parseNormalizationTests() {
    let view = FileView(URL("file:NormalizationTest.txt"));
    let test_data = view.string_view();

    std::vector<NormalizationTest> r;
    for (let line: split(test_data, "\n")) {
        if (let optionalTest = parseNormalizationTest_line(line)) {   
            r.push_back(*optionalTest);
        }
    }
    return r;
}

class UnicodeDataTests : public ::testing::Test {
protected:
    void SetUp() override {
        normalizationTests = parseNormalizationTests();
    }

    std::vector<NormalizationTest> normalizationTests;
    UnicodeData unicodeData = UnicodeData(UnicodeData_bin_bytes);
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

TEST_F(UnicodeDataTests, toNFC_source) {
    for (let test: normalizationTests) {
        auto tmp = test.NFD;
        unicodeData.compose(tmp);
        ASSERT_EQ(tmp, test.NFC) << test.comment;
    }
}

TEST_F(UnicodeDataTests, toNFD_source) {
    for (let test: normalizationTests) {
        ASSERT_EQ(unicodeData.canonicalDecompose(test.source), test.NFD) << test.comment;
    }
}

TEST_F(UnicodeDataTests, toNFD_NFC) {
    for (let test: normalizationTests) {
        ASSERT_EQ(unicodeData.canonicalDecompose(test.NFC), test.NFD) << test.comment;
    }
}

TEST_F(UnicodeDataTests, toNFD_NFD) {
    for (let test: normalizationTests) {
        ASSERT_EQ(unicodeData.canonicalDecompose(test.NFD), test.NFD) << test.comment;
    }
}

TEST_F(UnicodeDataTests, toNFD_NFKC) {
    for (let test: normalizationTests) {
        ASSERT_EQ(unicodeData.canonicalDecompose(test.NFKC), test.NFKD) << test.comment;
    }
}

TEST_F(UnicodeDataTests, toNFD_NFKD) {
    for (let test: normalizationTests) {
        ASSERT_EQ(unicodeData.canonicalDecompose(test.NFKD), test.NFKD) << test.comment;
    }
}


TEST_F(UnicodeDataTests, toNFKD_source) {
    for (let test: normalizationTests) {
        ASSERT_EQ(unicodeData.compatibleDecompose(test.source), test.NFKD) << test.comment;
    }
}

TEST_F(UnicodeDataTests, toNFKD_NFC) {
    for (let test: normalizationTests) {
        ASSERT_EQ(unicodeData.compatibleDecompose(test.NFC), test.NFKD) << test.comment;
    }
}

TEST_F(UnicodeDataTests, toNFKD_NFD) {
    for (let test: normalizationTests) {
        ASSERT_EQ(unicodeData.compatibleDecompose(test.NFD), test.NFKD) << test.comment;
    }
}

TEST_F(UnicodeDataTests, toNFKD_NFKC) {
    for (let test: normalizationTests) {
        ASSERT_EQ(unicodeData.compatibleDecompose(test.NFKC), test.NFKD) << test.comment;
    }
}

TEST_F(UnicodeDataTests, toNFKD_NFKD) {
    for (let test: normalizationTests) {
        ASSERT_EQ(unicodeData.compatibleDecompose(test.NFKD), test.NFKD) << test.comment;
    }
}

/*
TEST_F(UnicodeDataTests, CompatibleDecompose) {
    for (let test: normalizationTests) {
        ASSERT_EQ(unicodeData.compatibleDecompose(test.source), test.NFKD) << test.comment;
    }
}
*/