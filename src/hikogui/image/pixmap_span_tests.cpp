// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pixmap_span.hpp"
#include "pixmap.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(pixmap_span) {

TEST_CASE(construct_empty)
{
    auto a = hi::pixmap_span<uint8_t>{};
    REQUIRE(a.empty());
    REQUIRE(a.width() == 0);
    REQUIRE(a.height() == 0);
    REQUIRE(a.stride() == 0);
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

TEST_CASE(convert_from_pixmap)
{
    auto a = make_test_pixmap();
    auto b = hi::pixmap_span{a};

    REQUIRE(not b.empty());
    REQUIRE(b.width() == 4);
    REQUIRE(b.height() == 3);
    REQUIRE(b.stride() == 4);

    REQUIRE(b(0, 0) == 0);
    REQUIRE(b(1, 0) == 1);
    REQUIRE(b(2, 0) == 2);
    REQUIRE(b(3, 0) == 3);
    REQUIRE(b(0, 1) == 4);
    REQUIRE(b(1, 1) == 5);
    REQUIRE(b(2, 1) == 6);
    REQUIRE(b(3, 1) == 7);
    REQUIRE(b(0, 2) == 8);
    REQUIRE(b(1, 2) == 9);
    REQUIRE(b(2, 2) == 10);
    REQUIRE(b(3, 2) == 11);
}

TEST_CASE(convert_to_pixmap)
{
    auto a = make_test_pixmap();
    auto b = hi::pixmap_span<uint8_t>{a};
    auto c = b.subimage(1, 0, 2, 2);

    REQUIRE(not c.empty());
    REQUIRE(c.width() == 2);
    REQUIRE(c.height() == 2);
    REQUIRE(c.stride() == 4);
    REQUIRE(c(0, 0) == 1);
    REQUIRE(c(1, 0) == 2);
    REQUIRE(c(0, 1) == 5);
    REQUIRE(c(1, 1) == 6);

    auto d = hi::pixmap{c};

    REQUIRE(not d.empty());
    REQUIRE(d.width() == 2);
    REQUIRE(d.height() == 2);
    REQUIRE(d.capacity() == 4);
    REQUIRE(d(0, 0) == 1);
    REQUIRE(d(1, 0) == 2);
    REQUIRE(d(0, 1) == 5);
    REQUIRE(d(1, 1) == 6);
}

TEST_CASE(construct_from_data)
{
    auto a = make_test_pixmap();

    // Create a smaller image_view, last argument is the stride.
    auto b = hi::pixmap_span<uint8_t>{a.data(), 3, 3, 4};

    REQUIRE(not b.empty());
    REQUIRE(b.width() == 3);
    REQUIRE(b.height() == 3);
    REQUIRE(b.stride() == 4);

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

TEST_CASE(copy_assign)
{
    auto a = make_test_pixmap();
    auto b = hi::pixmap_span<uint8_t>{};

    REQUIRE(b.width() == 0);
    REQUIRE(b.height() == 0);
    REQUIRE(b.stride() == 0);
    REQUIRE(b.data() == nullptr);

    b = hi::pixmap_span<uint8_t>{a};

    REQUIRE(b.width() == 4);
    REQUIRE(b.height() == 3);
    REQUIRE(b.stride() == 4);
    REQUIRE(b.data() == a.data());

    REQUIRE(b(0, 0) == 0);
    REQUIRE(b(1, 0) == 1);
    REQUIRE(b(2, 0) == 2);
    REQUIRE(b(3, 0) == 3);
    REQUIRE(b(0, 1) == 4);
    REQUIRE(b(1, 1) == 5);
    REQUIRE(b(2, 1) == 6);
    REQUIRE(b(3, 1) == 7);
    REQUIRE(b(0, 2) == 8);
    REQUIRE(b(1, 2) == 9);
    REQUIRE(b(2, 2) == 10);
    REQUIRE(b(3, 2) == 11);
}

TEST_CASE(subimage)
{
    auto a_ = make_test_pixmap();
    auto a = hi::pixmap_span<uint8_t>{a_};

    {
        auto b = a.subimage(0, 0, 4, 3);

        REQUIRE(not b.empty());
        REQUIRE(b.width() == 4);
        REQUIRE(b.height() == 3);
        REQUIRE(b.stride() == 4);
        REQUIRE(b(0, 0) == 0);
        REQUIRE(b(1, 0) == 1);
        REQUIRE(b(2, 0) == 2);
        REQUIRE(b(3, 0) == 3);
        REQUIRE(b(0, 1) == 4);
        REQUIRE(b(1, 1) == 5);
        REQUIRE(b(2, 1) == 6);
        REQUIRE(b(3, 1) == 7);
        REQUIRE(b(0, 2) == 8);
        REQUIRE(b(1, 2) == 9);
        REQUIRE(b(2, 2) == 10);
        REQUIRE(b(3, 2) == 11);
    }

    {
        auto b = a.subimage(0, 0, 2, 2);

        REQUIRE(not b.empty());
        REQUIRE(b.width() == 2);
        REQUIRE(b.height() == 2);
        REQUIRE(b.stride() == 4);
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
        REQUIRE(b.stride() == 4);
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
        REQUIRE(b.stride() == 4);
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
        REQUIRE(b.stride() == 4);
        REQUIRE(b(0, 0) == 5);
        REQUIRE(b(1, 0) == 6);
        REQUIRE(b(0, 1) == 9);
        REQUIRE(b(1, 1) == 10);

        auto c = b.subimage(0, 1, 2, 1);
        REQUIRE(not c.empty());
        REQUIRE(c.width() == 2);
        REQUIRE(c.height() == 1);
        REQUIRE(c.stride() == 4);
        REQUIRE(c(0, 0) == 9);
        REQUIRE(c(1, 0) == 10);
    }
}

};