// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

// The tests have come from the libcxx std::vector tests, rewritten to work on hi::lean_vector
// and use the google test frame-work.
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "lean_vector.hpp"
#include <gtest/gtest.h>

using namespace hi;

template<typename C>
[[nodiscard]] static C access_make(int size, int start = 0)
{
    C c;

    for (int i = 0; i < size; ++i) {
        c.push_back(start + i);
    }

    return c;
}

TEST(lean_vector, access)
{
    using C = lean_vector<int>;
    C c = access_make<C>(10);

    static_assert(noexcept(c[0]));
    static_assert(noexcept(c.front()));
    static_assert(noexcept(c.back()));
    // at() is NOT noexcept

    static_assert(std::is_same_v<C::reference, decltype(c[0])>);
    static_assert(std::is_same_v<C::reference, decltype(c.at(0))>);
    static_assert(std::is_same_v<C::reference, decltype(c.front())>);
    static_assert(std::is_same_v<C::reference, decltype(c.back())>);

    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(c[i], i);
    }
    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(c.at(i), i);
    }
    ASSERT_EQ(c.front(), 0);
    ASSERT_EQ(c.back(), 9);
}

TEST(lean_vector, access_const)
{
    using C = lean_vector<int>;
    const int N = 5;
    const C c = access_make<C>(10, N);

    static_assert(noexcept(c[0]));
    static_assert(noexcept(c.front()));
    static_assert(noexcept(c.back()));
    // at() is NOT noexcept

    static_assert(std::is_same_v<C::const_reference, decltype(c[0])>);
    static_assert(std::is_same_v<C::const_reference, decltype(c.at(0))>);
    static_assert(std::is_same_v<C::const_reference, decltype(c.front())>);
    static_assert(std::is_same_v<C::const_reference, decltype(c.back())>);

    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(c[i], N + i);
    }
    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(c.at(i), N + i);
    }
    ASSERT_EQ(c.front(), N);
    ASSERT_EQ(c.back(), N + 9);
}

TEST(lean_vector, contiguous)
{
    using C = lean_vector<int>;
    const C c = C(3, 5);

    for (size_t i = 0; i < c.size(); ++i) {
        ASSERT_EQ(*(c.begin() + static_cast<typename C::difference_type>(i)), *(std::addressof(*c.begin()) + i));
    }
}

TEST(lean_vector, iterators)
{
    typedef int T;
    typedef lean_vector<T> C;
    C c;
    C::iterator i = c.begin();
    C::iterator j = c.end();
    ASSERT_EQ(std::distance(i, j), 0);
    ASSERT_EQ(i, j);
}

TEST(lean_vector, const_iterators)
{
    typedef int T;
    typedef lean_vector<T> C;
    const C c;
    C::const_iterator i = c.begin();
    C::const_iterator j = c.end();
    ASSERT_EQ(std::distance(i, j), 0);
    ASSERT_EQ(i, j);
}

TEST(lean_vector, const_iterators2)
{
    typedef int T;
    typedef lean_vector<T> C;
    C c;
    C::const_iterator i = c.cbegin();
    C::const_iterator j = c.cend();
    ASSERT_EQ(std::distance(i, j), 0);
    ASSERT_EQ(i, j);
    ASSERT_EQ(i, c.end());
}

TEST(lean_vector, iterators_construction)
{
    typedef int T;
    typedef lean_vector<T> C;
    const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    C c(std::begin(t), std::end(t));
    C::iterator i = c.begin();
    ASSERT_EQ(*i, 0);
    ++i;
    ASSERT_EQ(*i, 1);
    *i = 10;
    ASSERT_EQ(*i, 10);
    ASSERT_EQ(std::distance(c.begin(), c.end()), 10);
}

TEST(lean_vector, iterators_N3644)
{
    typedef lean_vector<int> C;
    C::iterator ii1{}, ii2{};
    C::iterator ii4 = ii1;
    C::const_iterator cii{};
    ASSERT_TRUE(ii1 == ii2);
    ASSERT_TRUE(ii1 == ii4);

    ASSERT_TRUE(!(ii1 != ii2));

    ASSERT_TRUE((ii1 == cii));
    ASSERT_TRUE((cii == ii1));
    ASSERT_TRUE(!(ii1 != cii));
    ASSERT_TRUE(!(cii != ii1));
    ASSERT_TRUE(!(ii1 < cii));
    ASSERT_TRUE(!(cii < ii1));
    ASSERT_TRUE((ii1 <= cii));
    ASSERT_TRUE((cii <= ii1));
    ASSERT_TRUE(!(ii1 > cii));
    ASSERT_TRUE(!(cii > ii1));
    ASSERT_TRUE((ii1 >= cii));
    ASSERT_TRUE((cii >= ii1));
    ASSERT_TRUE(cii - ii1 == 0);
    ASSERT_TRUE(ii1 - cii == 0);
}

template<typename T>
static void types_test()
{
    typedef lean_vector<T> C;

    //  TODO: These tests should use allocator_traits to get stuff, rather than
    //  blindly pulling typedefs out of the allocator. This is why we can't call
    //  test<int, min_allocator<int>>() below.
    static_assert((std::is_same<typename C::value_type, T>::value));

    static_assert((std::is_signed<typename C::difference_type>::value));
    static_assert((std::is_unsigned<typename C::size_type>::value));

    static_assert(
        (std::is_same<typename std::iterator_traits<typename C::iterator>::iterator_category, std::random_access_iterator_tag>::
             value));
    static_assert((std::is_same<
                   typename std::iterator_traits<typename C::const_iterator>::iterator_category,
                   std::random_access_iterator_tag>::value));
    static_assert((std::is_same<typename C::reverse_iterator, std::reverse_iterator<typename C::iterator>>::value));
    static_assert((std::is_same<typename C::const_reverse_iterator, std::reverse_iterator<typename C::const_iterator>>::value));
}

TEST(lean_vector, types)
{
    class Copyable {
    public:
    };

    types_test<int>();
    types_test<int *>();
    types_test<Copyable>();
    static_assert((std::is_same<lean_vector<char>::allocator_type, std::allocator<char>>::value), "");
}

TEST(lean_vector, capacity_empty)
{
    lean_vector<int> v;
    ASSERT_EQ(v.capacity(), v.short_capacity);
}

TEST(lean_vector, capacity_100)
{
    lean_vector<int> v(100);
    ASSERT_EQ(v.capacity(), 100);
    v.push_back(0);
    ASSERT_GT(v.capacity(), 101);
}

TEST(lean_vector, empty)
{
    typedef lean_vector<int> C;
    C c;
    static_assert(noexcept(c.empty()));
    ASSERT_TRUE(c.empty());
    c.push_back(C::value_type(1));
    ASSERT_FALSE(c.empty());
    c.clear();
    ASSERT_TRUE(c.empty());
}

TEST(lean_vector, reserve_10)
{
    lean_vector<int> v;
    v.reserve(10);
    ASSERT_GE(v.capacity(), 10);
}

TEST(lean_vector, reserve_100)
{
    lean_vector<int> v(100);
    ASSERT_EQ(v.size(), 100);
    ASSERT_EQ(v.capacity(), 100);
    v.reserve(50);
    ASSERT_EQ(v.size(), 100);
    ASSERT_EQ(v.capacity(), 100);
    v.reserve(150);
    ASSERT_EQ(v.size(), 100);
    ASSERT_EQ(v.capacity(), 150);
}

TEST(lean_vector, resize_size)
{
    lean_vector<int> v(100);
    v.resize(50);
    ASSERT_EQ(v.size(), 50);
    ASSERT_EQ(v.capacity(), 100);
    v.resize(200);
    ASSERT_EQ(v.size(), 200);
    ASSERT_GE(v.capacity(), 200);
}

TEST(lean_vector, resize_size_value)
{
    lean_vector<int> v(100);
    v.resize(50, 1);
    ASSERT_EQ(v.size(), 50);
    ASSERT_EQ(v.capacity(), 100);
    ASSERT_EQ(v, lean_vector<int>(50));
    v.resize(200, 1);
    ASSERT_EQ(v.size(), 200);
    ASSERT_GE(v.capacity(), 200);
    for (unsigned i = 0; i < 50; ++i) {
        ASSERT_EQ(v[i], 0);
    }
    for (unsigned i = 50; i < 200; ++i) {
        ASSERT_EQ(v[i], 1);
    }
}

TEST(lean_vector, shrink_to_fit)
{
    lean_vector<int> v(100);
    v.push_back(1);
    v.shrink_to_fit();
    ASSERT_EQ(v.capacity(), 101);
    ASSERT_EQ(v.size(), 101);
}

TEST(lean_vector, size)
{
    typedef lean_vector<int> C;
    C c;
    static_assert(noexcept(c.size()));
    ASSERT_EQ(c.size(), 0);
    c.push_back(C::value_type(2));
    ASSERT_EQ(c.size(), 1);
    c.push_back(C::value_type(1));
    ASSERT_EQ(c.size(), 2);
    c.push_back(C::value_type(3));
    ASSERT_EQ(c.size(), 3);
    c.erase(c.begin());
    ASSERT_EQ(c.size(), 2);
    c.erase(c.begin());
    ASSERT_EQ(c.size(), 1);
    c.erase(c.begin());
    ASSERT_EQ(c.size(), 0);
}

TEST(lean_vector, swap_short_short)
{
    lean_vector<int> v1(3);
    lean_vector<int> v2(5);
    v1.swap(v2);
    ASSERT_EQ(v1.size(), 5);
    ASSERT_EQ(v1.capacity(), v1.short_capacity);
    ASSERT_EQ(v2.size(), 3);
    ASSERT_EQ(v2.capacity(), v2.short_capacity);
}

TEST(lean_vector, swap_short_long)
{
    lean_vector<int> v1(3);
    lean_vector<int> v2(200);
    v1.swap(v2);
    ASSERT_EQ(v1.size(), 200);
    ASSERT_EQ(v1.capacity(), 200);
    ASSERT_EQ(v2.size(), 3);
    ASSERT_EQ(v2.capacity(), v2.short_capacity);
}

TEST(lean_vector, swap_long_short)
{
    lean_vector<int> v1(100);
    lean_vector<int> v2(5);
    v1.swap(v2);
    ASSERT_EQ(v1.size(), 5);
    ASSERT_EQ(v1.capacity(), v1.short_capacity);
    ASSERT_EQ(v2.size(), 100);
    ASSERT_EQ(v2.capacity(), 100);
}

TEST(lean_vector, swap_long_long)
{
    lean_vector<int> v1(100);
    lean_vector<int> v2(200);
    v1.swap(v2);
    ASSERT_EQ(v1.size(), 200);
    ASSERT_EQ(v1.capacity(), 200);
    ASSERT_EQ(v2.size(), 100);
    ASSERT_EQ(v2.capacity(), 100);
}
