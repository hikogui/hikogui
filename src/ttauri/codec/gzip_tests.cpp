// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
#include "ttauri/codec/gzip.hpp"
#include "ttauri/file_view.hpp"
#include "ttauri/required.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace tt;


TEST(GZip, UnzipEmpty) {
    auto decompressed = gzip_decompress(URL("file:gzip_test1.bin.gz"));

    ttlet original = file_view(URL("file:gzip_test1.bin"));
    ttlet original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipSingleA) {
    auto decompressed = gzip_decompress(URL("file:gzip_test2.bin.gz"));

    ttlet original = file_view(URL("file:gzip_test2.bin"));
    ttlet original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipText) {
    auto decompressed = gzip_decompress(URL("file:gzip_test3.bin.gz"));

    ttlet original = file_view(URL("file:gzip_test3.bin"));
    ttlet original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

// The rest of the tests are from the caterbury corpus

TEST(GZip, UnzipCpHTML) {
    auto decompressed = gzip_decompress(URL("file:gzip_test4.bin.gz"));

    ttlet original = file_view(URL("file:gzip_test4.bin"));
    ttlet original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipFieldsC) {
    auto decompressed = gzip_decompress(URL("file:gzip_test5.bin.gz"));

    ttlet original = file_view(URL("file:gzip_test5.bin"));
    ttlet original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipGrammarLSP) {
    auto decompressed = gzip_decompress(URL("file:gzip_test6.bin.gz"));

    ttlet original = file_view(URL("file:gzip_test6.bin"));
    ttlet original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipSum) {
    auto decompressed = gzip_decompress(URL("file:gzip_test7.bin.gz"));

    ttlet original = file_view(URL("file:gzip_test7.bin"));
    ttlet original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipXargs1) {
    auto decompressed = gzip_decompress(URL("file:gzip_test8.bin.gz"));

    ttlet original = file_view(URL("file:gzip_test8.bin"));
    ttlet original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}
