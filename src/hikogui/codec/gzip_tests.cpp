// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gzip.hpp"
#include "../file/file.hpp"
#include "../container/container.hpp"
#include "../path/path.hpp"
#include "../utility/utility.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(gzip_suite) {

TEST_CASE(unzip_empty)
{
    auto decompressed = hi::gzip_decompress(hi::library_source_dir() / "tests" / "data" / "gzip_test1.bin.gz");

    auto const original = hi::file_view{hi::library_source_dir() / "tests" / "data" / "gzip_test1.bin"};
    auto const original_bytes = as_bstring_view(original);

    REQUIRE(decompressed.size() == original_bytes.size());

    for (size_t i = 0; i != decompressed.size(); ++i) {
        REQUIRE(decompressed[i] == original_bytes[i]);
    }
}

TEST_CASE(unzip_single_a)
{
    auto decompressed = hi::gzip_decompress(hi::library_source_dir() / "tests" / "data" / "gzip_test2.bin.gz");

    auto const original = hi::file_view{hi::library_source_dir() / "tests" / "data" / "gzip_test2.bin"};
    auto const original_bytes = as_bstring_view(original);

    REQUIRE(decompressed.size() == original_bytes.size());

    for (size_t i = 0; i != decompressed.size(); ++i) {
        REQUIRE(decompressed[i] == original_bytes[i]);
    }
}

TEST_CASE(unzip_text)
{
    auto decompressed = hi::gzip_decompress(hi::library_source_dir() / "tests" / "data" / "gzip_test3.bin.gz");

    auto const original = hi::file_view{hi::library_source_dir() / "tests" / "data" / "gzip_test3.bin"};
    auto const original_bytes = as_bstring_view(original);

    REQUIRE(decompressed.size() == original_bytes.size());

    for (size_t i = 0; i != decompressed.size(); ++i) {
        REQUIRE(decompressed[i] == original_bytes[i]);
    }
}

// The rest of the tests are from the caterbury corpus

TEST_CASE(unzip_caterbury_html)
{
    auto decompressed = hi::gzip_decompress(hi::library_source_dir() / "tests" / "data" / "gzip_test4.bin.gz");

    auto const original = hi::file_view{hi::library_source_dir() / "tests" / "data" / "gzip_test4.bin"};
    auto const original_bytes = as_bstring_view(original);

    REQUIRE(decompressed.size() == original_bytes.size());

    for (size_t i = 0; i != decompressed.size(); ++i) {
        REQUIRE(decompressed[i] == original_bytes[i]);
    }
}

TEST_CASE(unzip_fields_c)
{
    auto decompressed = hi::gzip_decompress(hi::library_source_dir() / "tests" / "data" / "gzip_test5.bin.gz");

    auto const original = hi::file_view{hi::library_source_dir() / "tests" / "data" / "gzip_test5.bin"};
    auto const original_bytes = as_bstring_view(original);

    REQUIRE(decompressed.size() == original_bytes.size());

    for (size_t i = 0; i != decompressed.size(); ++i) {
        REQUIRE(decompressed[i] == original_bytes[i]);
    }
}

TEST_CASE(unzip_grammar_lsp)
{
    auto decompressed = hi::gzip_decompress(hi::library_source_dir() / "tests" / "data" / "gzip_test6.bin.gz");

    auto const original = hi::file_view{hi::library_source_dir() / "tests" / "data" / "gzip_test6.bin"};
    auto const original_bytes = as_bstring_view(original);

    REQUIRE(decompressed.size() == original_bytes.size());

    for (size_t i = 0; i != decompressed.size(); ++i) {
        REQUIRE(decompressed[i] == original_bytes[i]);
    }
}

TEST_CASE(unzip_sum)
{
    auto decompressed = hi::gzip_decompress(hi::library_source_dir() / "tests" / "data" / "gzip_test7.bin.gz");

    auto const original = hi::file_view{hi::library_source_dir() / "tests" / "data" / "gzip_test7.bin"};
    auto const original_bytes = as_bstring_view(original);

    REQUIRE(decompressed.size() == original_bytes.size());

    for (size_t i = 0; i != decompressed.size(); ++i) {
        REQUIRE(decompressed[i] == original_bytes[i]);
    }
}

TEST_CASE(unzip_xargs1)
{
    auto decompressed = hi::gzip_decompress(hi::library_source_dir() / "tests" / "data" / "gzip_test8.bin.gz");

    auto const original = hi::file_view{hi::library_source_dir() / "tests" / "data" / "gzip_test8.bin"};
    auto const original_bytes = as_bstring_view(original);

    REQUIRE(decompressed.size() == original_bytes.size());

    for (size_t i = 0; i != decompressed.size(); ++i) {
        REQUIRE(decompressed[i] == original_bytes[i]);
    }
}

};
