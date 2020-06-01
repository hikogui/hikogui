#include "TTauri/Foundation/gzip.hpp"
#include "TTauri/Foundation/FileView.hpp"
#include "TTauri/Foundation/required.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace TTauri;


TEST(GZip, Empty) {
    auto decompressed = gzip_decompress(URL("file:gzip_test1.bin.gz"));

    let original = FileView(URL("file:gzip_test1.bin"));
    let original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, SingleA) {
    auto decompressed = gzip_decompress(URL("file:gzip_test2.bin.gz"));

    let original = FileView(URL("file:gzip_test2.bin"));
    let original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UncompressText) {
    auto decompressed = gzip_decompress(URL("file:gzip_test3.bin.gz"));

    let original = FileView(URL("file:gzip_test3.bin"));
    let original_bytes = original.bytes();

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}
