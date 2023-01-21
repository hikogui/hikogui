// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pixmap.hpp"
#include <gtest/gtest.h>

TEST(pixmap, construct_empty)
{
    auto a = hi::pixmap<uint8_t>{};
    ASSERT_TRUE(a.empty());
    ASSERT_EQ(a.width(), 0);
    ASSERT_EQ(a.height(), 0);
    ASSERT_EQ(a.size(), 0);
    ASSERT_EQ(a.capacity(), 0);
}

TEST(pixmap, construct_zero_fill)
{
    auto a = hi::pixmap<uint8_t>{4, 3};
    ASSERT_FALSE(a.empty());
    ASSERT_EQ(a.width(), 4);
    ASSERT_EQ(a.height(), 3);
    ASSERT_EQ(a.size(), 12);
    ASSERT_EQ(a.capacity(), 12);
    for (auto const& pixel : a) {
        ASSERT_EQ(pixel, 0);
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

TEST(pixmap, copy_construct)
{
    auto a = make_test_pixmap();
    auto b = a;

    ASSERT_FALSE(b.empty());
    ASSERT_EQ(b.width(), 4);
    ASSERT_EQ(b.height(), 3);
    ASSERT_EQ(b.size(), 12);
    ASSERT_EQ(b.capacity(), 12);

    auto i = uint8_t{0};
    for (auto const& pixel : b) {
        ASSERT_EQ(pixel, i);
        ++i;
    }
}

TEST(pixmap, construct_from_data)
{
    auto a = make_test_pixmap();

    // Create a smaller image, last argument is the stride.
    auto b = hi::pixmap<uint8_t>{a.data(), 3, 3, 4};

    ASSERT_FALSE(b.empty());
    ASSERT_EQ(b.width(), 3);
    ASSERT_EQ(b.height(), 3);
    ASSERT_EQ(b.size(), 9);
    ASSERT_EQ(b.capacity(), 9);

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

TEST(pixmap, move_construct)
{
    auto a = make_test_pixmap();
    auto b = std::move(a);

    ASSERT_FALSE(b.empty());
    ASSERT_EQ(b.width(), 4);
    ASSERT_EQ(b.height(), 3);
    ASSERT_EQ(b.size(), 12);
    ASSERT_EQ(b.capacity(), 12);

    ASSERT_TRUE(a.empty());
    ASSERT_EQ(a.capacity(), 0);

    auto i = uint8_t{0};
    for (auto const& pixel : b) {
        ASSERT_EQ(pixel, i);
        ++i;
    }
}

TEST(pixmap, copy_assign)
{
    auto a = make_test_pixmap();
    auto b = hi::pixmap<uint8_t>{10, 8};

    ASSERT_EQ(b.size(), 80);
    ASSERT_EQ(b.capacity(), 80);

    b = a;

    ASSERT_FALSE(b.empty());
    ASSERT_EQ(b.width(), 4);
    ASSERT_EQ(b.height(), 3);
    ASSERT_EQ(b.size(), 12);
    ASSERT_EQ(b.capacity(), 80);

    auto i = uint8_t{0};
    for (auto const& pixel : b) {
        ASSERT_EQ(pixel, i);
        ++i;
    }
}

TEST(pixmap, move_assign)
{
    auto a = make_test_pixmap();
    auto b = hi::pixmap<uint8_t>{10, 8};

    ASSERT_EQ(b.size(), 80);
    ASSERT_EQ(b.capacity(), 80);

    b = std::move(a);

    ASSERT_FALSE(b.empty());
    ASSERT_EQ(b.width(), 4);
    ASSERT_EQ(b.height(), 3);
    ASSERT_EQ(b.size(), 12);
    ASSERT_EQ(b.capacity(), 12);

    auto i = uint8_t{0};
    for (auto const& pixel : b) {
        ASSERT_EQ(pixel, i);
        ++i;
    }
}

TEST(pixmap, shrink_to_fit)
{
    auto a = make_test_pixmap();
    auto b = hi::pixmap<uint8_t>{10, 8};
    b = a;

    ASSERT_EQ(b.capacity(), 80);

    b.shrink_to_fit();

    ASSERT_EQ(b.width(), 4);
    ASSERT_EQ(b.height(), 3);
    ASSERT_EQ(b.size(), 12);
    ASSERT_EQ(b.capacity(), 12);
}

TEST(pixmap, clear)
{
    auto a = make_test_pixmap();

    ASSERT_EQ(a.width(), 4);
    ASSERT_EQ(a.height(), 3);
    ASSERT_EQ(a.capacity(), 12);

    a.clear();

    ASSERT_EQ(a.width(), 0);
    ASSERT_EQ(a.height(), 0);
    ASSERT_EQ(a.capacity(), 12);

    a.shrink_to_fit();

    ASSERT_EQ(a.width(), 0);
    ASSERT_EQ(a.height(), 0);
    ASSERT_EQ(a.capacity(), 0);
    ASSERT_EQ(a.data(), nullptr);
}

TEST(pixmap, subimage)
{
    auto a = make_test_pixmap();

    {
        auto b = a.subimage(0, 0, 4, 3);

        ASSERT_FALSE(b.empty());
        ASSERT_EQ(b.width(), 4);
        ASSERT_EQ(b.height(), 3);
        ASSERT_EQ(b.capacity(), 12);
        auto i = uint8_t{0};
        for (auto& pixel : b) {
            ASSERT_EQ(pixel, i);
            ++i;
        }
    }

    {
        auto b = a.subimage(0, 0, 2, 2);

        ASSERT_FALSE(b.empty());
        ASSERT_EQ(b.width(), 2);
        ASSERT_EQ(b.height(), 2);
        ASSERT_EQ(b.capacity(), 4);
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
        ASSERT_EQ(b.capacity(), 4);
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
        ASSERT_EQ(b.capacity(), 4);
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
        ASSERT_EQ(b.capacity(), 4);
        ASSERT_EQ(b(0, 0), 5);
        ASSERT_EQ(b(1, 0), 6);
        ASSERT_EQ(b(0, 1), 9);
        ASSERT_EQ(b(1, 1), 10);
    }
}
