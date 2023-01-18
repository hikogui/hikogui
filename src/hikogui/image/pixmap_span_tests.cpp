// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pixmap_span.hpp"
#include "pixmap.hpp"
#include <gtest/gtest.h>

TEST(pixmap_span, construct_empty)
{
    auto a = hi::pixmap_span<uint8_t>{};
    ASSERT_TRUE(a.empty());
    ASSERT_EQ(a.width(), 0);
    ASSERT_EQ(a.height(), 0);
    ASSERT_EQ(a.stride(), 0);
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

TEST(pixmap_span, convert_from_pixmap)
{
    auto a = make_test_pixmap();
    auto b = hi::pixmap_span{a};

    ASSERT_FALSE(b.empty());
    ASSERT_EQ(b.width(), 4);
    ASSERT_EQ(b.height(), 3);
    ASSERT_EQ(b.stride(), 4);

    ASSERT_EQ(b(0, 0), 0);
    ASSERT_EQ(b(1, 0), 1);
    ASSERT_EQ(b(2, 0), 2);
    ASSERT_EQ(b(3, 0), 3);
    ASSERT_EQ(b(0, 1), 4);
    ASSERT_EQ(b(1, 1), 5);
    ASSERT_EQ(b(2, 1), 6);
    ASSERT_EQ(b(3, 1), 7);
    ASSERT_EQ(b(0, 2), 8);
    ASSERT_EQ(b(1, 2), 9);
    ASSERT_EQ(b(2, 2), 10);
    ASSERT_EQ(b(3, 2), 11);
}

TEST(pixmap_span, convert_to_pixmap)
{
    auto a = make_test_pixmap();
    auto b = hi::pixmap_span<uint8_t>{a};
    auto c = b.subimage(1, 0, 2, 2);

    ASSERT_FALSE(c.empty());
    ASSERT_EQ(c.width(), 2);
    ASSERT_EQ(c.height(), 2);
    ASSERT_EQ(c.stride(), 4);
    ASSERT_EQ(c(0, 0), 1);
    ASSERT_EQ(c(1, 0), 2);
    ASSERT_EQ(c(0, 1), 5);
    ASSERT_EQ(c(1, 1), 6);

    auto d = hi::pixmap{c};

    ASSERT_FALSE(d.empty());
    ASSERT_EQ(d.width(), 2);
    ASSERT_EQ(d.height(), 2);
    ASSERT_EQ(d.capacity(), 4);
    ASSERT_EQ(d(0, 0), 1);
    ASSERT_EQ(d(1, 0), 2);
    ASSERT_EQ(d(0, 1), 5);
    ASSERT_EQ(d(1, 1), 6);
}

TEST(pixmap_span, construct_from_data)
{
    auto a = make_test_pixmap();

    // Create a smaller image_view, last argument is the stride.
    auto b = hi::pixmap_span<uint8_t>{a.data(), 3, 3, 4};

    ASSERT_FALSE(b.empty());
    ASSERT_EQ(b.width(), 3);
    ASSERT_EQ(b.height(), 3);
    ASSERT_EQ(b.stride(), 4);

    ASSERT_EQ(b(0, 0), 0);
    ASSERT_EQ(b(1, 0), 1);
    ASSERT_EQ(b(2, 0), 2);
    ASSERT_EQ(b(0, 1), 4);
    ASSERT_EQ(b(1, 1), 5);
    ASSERT_EQ(b(2, 1), 6);
    ASSERT_EQ(b(0, 2), 8);
    ASSERT_EQ(b(1, 2), 9);
    ASSERT_EQ(b(2, 2), 10);
}

TEST(pixmap_span, copy_assign)
{
    auto a = make_test_pixmap();
    auto b = hi::pixmap_span<uint8_t>{};

    ASSERT_EQ(b.width(), 0);
    ASSERT_EQ(b.height(), 0);
    ASSERT_EQ(b.stride(), 0);
    ASSERT_EQ(b.data(), nullptr);

    b = hi::pixmap_span<uint8_t>{a};

    ASSERT_EQ(b.width(), 4);
    ASSERT_EQ(b.height(), 3);
    ASSERT_EQ(b.stride(), 4);
    ASSERT_EQ(b.data(), a.data());

    ASSERT_EQ(b(0, 0), 0);
    ASSERT_EQ(b(1, 0), 1);
    ASSERT_EQ(b(2, 0), 2);
    ASSERT_EQ(b(3, 0), 3);
    ASSERT_EQ(b(0, 1), 4);
    ASSERT_EQ(b(1, 1), 5);
    ASSERT_EQ(b(2, 1), 6);
    ASSERT_EQ(b(3, 1), 7);
    ASSERT_EQ(b(0, 2), 8);
    ASSERT_EQ(b(1, 2), 9);
    ASSERT_EQ(b(2, 2), 10);
    ASSERT_EQ(b(3, 2), 11);
}

TEST(pixmap_span, subimage)
{
    auto a_ = make_test_pixmap();
    auto a = hi::pixmap_span<uint8_t>{a_};

    {
        auto b = a.subimage(0, 0, 4, 3);

        ASSERT_FALSE(b.empty());
        ASSERT_EQ(b.width(), 4);
        ASSERT_EQ(b.height(), 3);
        ASSERT_EQ(b.stride(), 4);
        ASSERT_EQ(b(0, 0), 0);
        ASSERT_EQ(b(1, 0), 1);
        ASSERT_EQ(b(2, 0), 2);
        ASSERT_EQ(b(3, 0), 3);
        ASSERT_EQ(b(0, 1), 4);
        ASSERT_EQ(b(1, 1), 5);
        ASSERT_EQ(b(2, 1), 6);
        ASSERT_EQ(b(3, 1), 7);
        ASSERT_EQ(b(0, 2), 8);
        ASSERT_EQ(b(1, 2), 9);
        ASSERT_EQ(b(2, 2), 10);
        ASSERT_EQ(b(3, 2), 11);
    }

    {
        auto b = a.subimage(0, 0, 2, 2);

        ASSERT_FALSE(b.empty());
        ASSERT_EQ(b.width(), 2);
        ASSERT_EQ(b.height(), 2);
        ASSERT_EQ(b.stride(), 4);
        ASSERT_EQ(b(0, 0), 0);
        ASSERT_EQ(b(1, 0), 1);
        ASSERT_EQ(b(0, 1), 4);
        ASSERT_EQ(b(1, 1), 5);
    }

    {
        auto b = a.subimage(1, 0, 2, 2);

        ASSERT_FALSE(b.empty());
        ASSERT_EQ(b.width(), 2);
        ASSERT_EQ(b.height(), 2);
        ASSERT_EQ(b.stride(), 4);
        ASSERT_EQ(b(0, 0), 1);
        ASSERT_EQ(b(1, 0), 2);
        ASSERT_EQ(b(0, 1), 5);
        ASSERT_EQ(b(1, 1), 6);
    }

    {
        auto b = a.subimage(0, 1, 2, 2);

        ASSERT_FALSE(b.empty());
        ASSERT_EQ(b.width(), 2);
        ASSERT_EQ(b.height(), 2);
        ASSERT_EQ(b.stride(), 4);
        ASSERT_EQ(b(0, 0), 4);
        ASSERT_EQ(b(1, 0), 5);
        ASSERT_EQ(b(0, 1), 8);
        ASSERT_EQ(b(1, 1), 9);
    }

    {
        auto b = a.subimage(1, 1, 2, 2);

        ASSERT_FALSE(b.empty());
        ASSERT_EQ(b.width(), 2);
        ASSERT_EQ(b.height(), 2);
        ASSERT_EQ(b.stride(), 4);
        ASSERT_EQ(b(0, 0), 5);
        ASSERT_EQ(b(1, 0), 6);
        ASSERT_EQ(b(0, 1), 9);
        ASSERT_EQ(b(1, 1), 10);

        auto c = b.subimage(0, 1, 2, 1);
        ASSERT_FALSE(c.empty());
        ASSERT_EQ(c.width(), 2);
        ASSERT_EQ(c.height(), 1);
        ASSERT_EQ(c.stride(), 4);
        ASSERT_EQ(c(0, 0), 9);
        ASSERT_EQ(c(1, 0), 10);
    }
}
