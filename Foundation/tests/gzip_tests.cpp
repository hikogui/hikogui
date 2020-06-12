#include "TTauri/Foundation/gzip.hpp"
#include "TTauri/Foundation/FileView.hpp"
#include "TTauri/Foundation/required.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace tt;


TEST(GZip, UnzipEmpty) {
    auto decompressed = gzip_decompress(URL("file:gzip_test1.bin.gz"));

    let original = FileView(URL("file:gzip_test1.bin"));
    let original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipSingleA) {
    auto decompressed = gzip_decompress(URL("file:gzip_test2.bin.gz"));

    let original = FileView(URL("file:gzip_test2.bin"));
    let original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipText) {
    auto decompressed = gzip_decompress(URL("file:gzip_test3.bin.gz"));

    let original = FileView(URL("file:gzip_test3.bin"));
    let original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

// The rest of the tests are from the caterbury corpus

TEST(GZip, UnzipCpHTML) {
    auto decompressed = gzip_decompress(URL("file:gzip_test4.bin.gz"));

    let original = FileView(URL("file:gzip_test4.bin"));
    let original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipFieldsC) {
    auto decompressed = gzip_decompress(URL("file:gzip_test5.bin.gz"));

    let original = FileView(URL("file:gzip_test5.bin"));
    let original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipGrammarLSP) {
    auto decompressed = gzip_decompress(URL("file:gzip_test6.bin.gz"));

    let original = FileView(URL("file:gzip_test6.bin"));
    let original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipSum) {
    auto decompressed = gzip_decompress(URL("file:gzip_test7.bin.gz"));

    let original = FileView(URL("file:gzip_test7.bin"));
    let original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipXargs1) {
    auto decompressed = gzip_decompress(URL("file:gzip_test8.bin.gz"));

    let original = FileView(URL("file:gzip_test8.bin"));
    let original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}
