// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pixmap.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(pixmap) {

TEST_CASE(construct_empty)
{
    auto a = hi::pixmap<uint8_t>{};
    REQUIRE(a.empty());
    REQUIRE(a.width() == 0);
    REQUIRE(a.height() == 0);
    REQUIRE(a.size() == 0);
    REQUIRE(a.capacity() == 0);
}

TEST_CASE(construct_zero_fill)
{
    auto a = hi::pixmap<uint8_t>{4, 3};
    REQUIRE(not a.empty());
    REQUIRE(a.width() == 4);
    REQUIRE(a.height() == 3);
    REQUIRE(a.size() == 12);
    REQUIRE(a.capacity() == 12);
    for (auto const& pixel : a) {
        REQUIRE(pixel == 0);
    }
}

static auto make_test_pixmap()
{
    auto r = hi::pixmap<uint8_t>{4, 3};
    auto i = uint8_t{0};
    for (auto& pixel : r) {
        pixel = i++;
    }
    return r;
}

TEST_CASE(copy_construct)
{
    auto a = make_test_pixmap();
    auto b = a;

    REQUIRE(not b.empty());
    REQUIRE(b.width() == 4);
    REQUIRE(b.height() == 3);
    REQUIRE(b.size() == 12);
    REQUIRE(b.capacity() == 12);

    auto i = uint8_t{0};
    for (auto const& pixel : b) {
        REQUIRE(pixel == i);
        ++i;
    }
}

TEST_CASE(construct_from_data)
{
    auto a = make_test_pixmap();

    // Create a smaller image, last argument is the stride.
    auto b = hi::pixmap<uint8_t>{a.data(), 3, 3, 4};

    REQUIRE(not b.empty());
    REQUIRE(b.width() == 3);
    REQUIRE(b.height() == 3);
    REQUIRE(b.size() == 9);
    REQUIRE(b.capacity() ==9);

    REQUIRE(b(0, 0) == 0);
    REQUIRE(b(1, 0) == 1);
    REQUIRE(b(2, 0) == 2);
    REQUIRE(b(0, 1) == 4);
    REQUIRE(b(1, 1) == 5);
    REQUIRE(b(2, 1) == 6);
    REQUIRE(b(0, 2) == 8);
    REQUIRE(b(1, 2) == 9);
    REQUIRE(b(2, 2) == 10);
}

TEST_CASE(move_construct)
{
    auto a = make_test_pixmap();
    auto b = std::move(a);

    REQUIRE(not b.empty());
    REQUIRE(b.width() == 4);
    REQUIRE(b.height() == 3);
    REQUIRE(b.size() == 12);
    REQUIRE(b.capacity() == 12);

    REQUIRE(a.empty());
    REQUIRE(a.capacity() == 0);

    auto i = uint8_t{0};
    for (auto const& pixel : b) {
        REQUIRE(pixel == i);
        ++i;
    }
}

TEST_CASE(copy_assign)
{
    auto a = make_test_pixmap();
    auto b = hi::pixmap<uint8_t>{10, 8};

    REQUIRE(b.size() == 80);
    REQUIRE(b.capacity() == 80);

    b = a;

    REQUIRE(not b.empty());
    REQUIRE(b.width() == 4);
    REQUIRE(b.height() == 3);
    REQUIRE(b.size() == 12);
    REQUIRE(b.capacity() == 80);

    auto i = uint8_t{0};
    for (auto const& pixel : b) {
        REQUIRE(pixel == i);
        ++i;
    }
}

TEST_CASE(move_assign)
{
    auto a = make_test_pixmap();
    auto b = hi::pixmap<uint8_t>{10, 8};

    REQUIRE(b.size() == 80);
    REQUIRE(b.capacity() == 80);

    b = std::move(a);

    REQUIRE(not b.empty());
    REQUIRE(b.width() == 4);
    REQUIRE(b.height() == 3);
    REQUIRE(b.size() == 12);
    REQUIRE(b.capacity() == 12);

    auto i = uint8_t{0};
    for (auto const& pixel : b) {
        REQUIRE(pixel == i);
        ++i;
    }
}

TEST_CASE(shrink_to_fit)
{
    auto a = make_test_pixmap();
    auto b = hi::pixmap<uint8_t>{10, 8};
    b = a;

    REQUIRE(b.capacity() == 80);

    b.shrink_to_fit();

    REQUIRE(b.width() == 4);
    REQUIRE(b.height() == 3);
    REQUIRE(b.size() == 12);
    REQUIRE(b.capacity() == 12);
}

TEST_CASE(clear)
{
    auto a = make_test_pixmap();

    REQUIRE(a.width() == 4);
    REQUIRE(a.height() == 3);
    REQUIRE(a.capacity() == 12);

    a.clear();

    REQUIRE(a.width() == 0);
    REQUIRE(a.height() == 0);
    REQUIRE(a.capacity() == 12);

    a.shrink_to_fit();

    REQUIRE(a.width() == 0);
    REQUIRE(a.height() == 0);
    REQUIRE(a.capacity() == 0);
    REQUIRE(a.data() == nullptr);
}

TEST_CASE(subimage)
{
    auto a = make_test_pixmap();

    {
        auto b = a.subimage(0, 0, 4, 3);

        REQUIRE(not b.empty());
        REQUIRE(b.width() == 4);
        REQUIRE(b.height() == 3);
        REQUIRE(b.capacity() == 12);
        auto i = uint8_t{0};
        for (auto& pixel : b) {
            REQUIRE(pixel == i);
            ++i;
        }
    }

    {
        auto b = a.subimage(0, 0, 2, 2);

        REQUIRE(not b.empty());
        REQUIRE(b.width() == 2);
        REQUIRE(b.height() == 2);
        REQUIRE(b.capacity() == 4);
        REQUIRE(b(0, 0) == 0);
        REQUIRE(b(1, 0) == 1);
        REQUIRE(b(0, 1) == 4);
        REQUIRE(b(1, 1) == 5);
    }

    {
        auto b = a.subimage(1, 0, 2, 2);

        REQUIRE(not b.empty());
        REQUIRE(b.width() == 2);
        REQUIRE(b.height() == 2);
        REQUIRE(b.capacity() == 4);
        REQUIRE(b(0, 0) == 1);
        REQUIRE(b(1, 0) == 2);
        REQUIRE(b(0, 1) == 5);
        REQUIRE(b(1, 1) == 6);
    }

    {
        auto b = a.subimage(0, 1, 2, 2);

        REQUIRE(not b.empty());
        REQUIRE(b.width() == 2);
        REQUIRE(b.height() == 2);
        REQUIRE(b.capacity() == 4);
        REQUIRE(b(0, 0) == 4);
        REQUIRE(b(1, 0) == 5);
        REQUIRE(b(0, 1) == 8);
        REQUIRE(b(1, 1) == 9);
    }

    {
        auto b = a.subimage(1, 1, 2, 2);

        REQUIRE(not b.empty());
        REQUIRE(b.width() == 2);
        REQUIRE(b.height() == 2);
        REQUIRE(b.capacity() == 4);
        REQUIRE(b(0, 0) == 5);
        REQUIRE(b(1, 0) == 6);
        REQUIRE(b(0, 1) == 9);
        REQUIRE(b(1, 1) == 10);
    }
}

};
