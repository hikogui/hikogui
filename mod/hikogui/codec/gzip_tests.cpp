// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gzip.hpp"
#include "../file/file.hpp"
#include "../path/path.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace hi;

TEST(GZip, UnzipEmpty)
{
    auto decompressed = gzip_decompress(library_source_dir() / "tests" / "data" / "gzip_test1.bin.gz");

    hilet original = file_view{library_source_dir() / "tests" / "data" / "gzip_test1.bin"};
    hilet original_bytes = as_bstring_view(original);

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipSingleA)
{
    auto decompressed = gzip_decompress(library_source_dir() / "tests" / "data" / "gzip_test2.bin.gz");

    hilet original = file_view{library_source_dir() / "tests" / "data" / "gzip_test2.bin"};
    hilet original_bytes = as_bstring_view(original);

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipText)
{
    auto decompressed = gzip_decompress(library_source_dir() / "tests" / "data" / "gzip_test3.bin.gz");

    hilet original = file_view{library_source_dir() / "tests" / "data" / "gzip_test3.bin"};
    hilet original_bytes = as_bstring_view(original);

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

// The rest of the tests are from the caterbury corpus

TEST(GZip, UnzipCpHTML)
{
    auto decompressed = gzip_decompress(library_source_dir() / "tests" / "data" / "gzip_test4.bin.gz");

    hilet original = file_view{library_source_dir() / "tests" / "data" / "gzip_test4.bin"};
    hilet original_bytes = as_bstring_view(original);

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipFieldsC)
{
    auto decompressed = gzip_decompress(library_source_dir() / "tests" / "data" / "gzip_test5.bin.gz");

    hilet original = file_view{library_source_dir() / "tests" / "data" / "gzip_test5.bin"};
    hilet original_bytes = as_bstring_view(original);

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipGrammarLSP)
{
    auto decompressed = gzip_decompress(library_source_dir() / "tests" / "data" / "gzip_test6.bin.gz");

    hilet original = file_view{library_source_dir() / "tests" / "data" / "gzip_test6.bin"};
    hilet original_bytes = as_bstring_view(original);

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipSum)
{
    auto decompressed = gzip_decompress(library_source_dir() / "tests" / "data" / "gzip_test7.bin.gz");

    hilet original = file_view{library_source_dir() / "tests" / "data" / "gzip_test7.bin"};
    hilet original_bytes = as_bstring_view(original);

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}

TEST(GZip, UnzipXargs1)
{
    auto decompressed = gzip_decompress(library_source_dir() / "tests" / "data" / "gzip_test8.bin.gz");

    hilet original = file_view{library_source_dir() / "tests" / "data" / "gzip_test8.bin"};
    hilet original_bytes = as_bstring_view(original);

    ASSERT_EQ(ssize(decompressed), ssize(original_bytes));

    for (ssize_t i = 0; i != ssize(decompressed); ++i) {
        ASSERT_EQ(decompressed[i], original_bytes[i]);
    }
}
