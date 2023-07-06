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

hi_warning_push();
// C26439: This kind of function should not throw. Declare it 'noexcept' (f.6)
// These test are required to test for throwing move constructor/operators.
hi_warning_ignore_msvc(26439)

namespace lean_vector_detail {

class Copyable {
public:
};

class MoveOnly {
    MoveOnly(const MoveOnly&);
    MoveOnly& operator=(const MoveOnly&);

    int data_;

public:
    MoveOnly(int data = 1) : data_(data) {}
    MoveOnly(MoveOnly&& x) noexcept : data_(x.data_)
    {
        x.data_ = 0;
    }

    MoveOnly& operator=(MoveOnly&& x) noexcept
    {
        data_ = x.data_;
        x.data_ = 0;
        return *this;
    }

    int get() const
    {
        return data_;
    }

    bool operator==(const MoveOnly& x) const
    {
        return data_ == x.data_;
    }
    bool operator<(const MoveOnly& x) const
    {
        return data_ < x.data_;
    }
    MoveOnly operator+(const MoveOnly& x) const
    {
        return MoveOnly{data_ + x.data_};
    }
    MoveOnly operator*(const MoveOnly& x) const
    {
        return MoveOnly{data_ * x.data_};
    }
};

class DefaultOnly {
    int data_;

    DefaultOnly(const DefaultOnly&);
    DefaultOnly& operator=(const DefaultOnly&);

public:
    // static int count;

    DefaultOnly() : data_(-1)
    {
        //++count;
    }
    ~DefaultOnly()
    {
        data_ = 0;
        //--count;
    }

    friend bool operator==(const DefaultOnly& x, const DefaultOnly& y)
    {
        return x.data_ == y.data_;
    }
    friend bool operator<(const DefaultOnly& x, const DefaultOnly& y)
    {
        return x.data_ < y.data_;
    }
};

// int DefaultOnly::count = 0;

template<class T>
struct EmplaceConstructibleMoveableAndAssignable {
    int copied = 0;
    int assigned = 0;
    T value;
    explicit EmplaceConstructibleMoveableAndAssignable(T xvalue) noexcept : value(xvalue) {}

    EmplaceConstructibleMoveableAndAssignable(EmplaceConstructibleMoveableAndAssignable&& Other) noexcept :
        copied(Other.copied + 1), value(std::move(Other.value))
    {
    }

    EmplaceConstructibleMoveableAndAssignable& operator=(EmplaceConstructibleMoveableAndAssignable&& Other) noexcept
    {
        copied = Other.copied;
        assigned = Other.assigned + 1;
        value = std::move(Other.value);
        return *this;
    }

    EmplaceConstructibleMoveableAndAssignable& operator=(T xvalue)
    {
        value = std::move(xvalue);
        ++assigned;
        return *this;
    }
};

struct Throws {
    Throws() : v_(0) {}
    Throws(int v) : v_(v) {}
    Throws(const Throws& rhs) : v_(rhs.v_)
    {
        if (sThrows)
            throw 1;
    }
    Throws(Throws&& rhs) : v_(rhs.v_)
    {
        if (sThrows)
            throw 1;
    }
    Throws& operator=(const Throws& rhs)
    {
        v_ = rhs.v_;
        return *this;
    }
    Throws& operator=(Throws&& rhs)
    {
        v_ = rhs.v_;
        return *this;
    }
    int v_;
    inline static bool sThrows = false;

    friend bool operator==(Throws const &, Throws const &) noexcept = default;
};

template<class It, class ItTraits = It>
class input_iterator {
    typedef std::iterator_traits<ItTraits> Traits;
    It it_;

    template<class U, class T>
    friend class input_iterator;

public:
    typedef std::input_iterator_tag iterator_category;
    typedef typename Traits::value_type value_type;
    typedef typename Traits::difference_type difference_type;
    typedef It pointer;
    typedef typename Traits::reference reference;

    It base() const
    {
        return it_;
    }

    input_iterator() : it_() {}
    explicit input_iterator(It it) : it_(it) {}
    template<class U, class T>
    input_iterator(const input_iterator<U, T>& u) : it_(u.it_)
    {
    }

    reference operator*() const
    {
        return *it_;
    }
    pointer operator->() const
    {
        return it_;
    }

    input_iterator& operator++()
    {
        ++it_;
        return *this;
    }
    input_iterator operator++(int)
    {
        input_iterator tmp(*this);
        ++(*this);
        return tmp;
    }

    friend bool operator==(const input_iterator& x, const input_iterator& y)
    {
        return x.it_ == y.it_;
    }
    friend bool operator!=(const input_iterator& x, const input_iterator& y)
    {
        return !(x == y);
    }

    template<class T>
    void operator,(T const&) = delete;
};

template<class T, class TV, class U, class UV>
inline bool operator==(const input_iterator<T, TV>& x, const input_iterator<U, UV>& y)
{
    return x.base() == y.base();
}

template<class T, class TV, class U, class UV>
inline bool operator!=(const input_iterator<T, TV>& x, const input_iterator<U, UV>& y)
{
    return !(x == y);
}

template<class It>
class forward_iterator {
    It it_;

    template<class U>
    friend class forward_iterator;

public:
    typedef std::forward_iterator_tag iterator_category;
    typedef typename std::iterator_traits<It>::value_type value_type;
    typedef typename std::iterator_traits<It>::difference_type difference_type;
    typedef It pointer;
    typedef typename std::iterator_traits<It>::reference reference;

    It base() const
    {
        return it_;
    }

    forward_iterator() : it_() {}
    explicit forward_iterator(It it) : it_(it) {}
    template<class U>
    forward_iterator(const forward_iterator<U>& u) : it_(u.it_)
    {
    }

    reference operator*() const
    {
        return *it_;
    }
    pointer operator->() const
    {
        return it_;
    }

    forward_iterator& operator++()
    {
        ++it_;
        return *this;
    }
    forward_iterator operator++(int)
    {
        forward_iterator tmp(*this);
        ++(*this);
        return tmp;
    }

    friend bool operator==(const forward_iterator& x, const forward_iterator& y)
    {
        return x.it_ == y.it_;
    }
    friend bool operator!=(const forward_iterator& x, const forward_iterator& y)
    {
        return !(x == y);
    }

    template<class T>
    void operator,(T const&) = delete;
};

template<class T, class U>
inline bool operator==(const forward_iterator<T>& x, const forward_iterator<U>& y)
{
    return x.base() == y.base();
}

template<class T, class U>
inline bool operator!=(const forward_iterator<T>& x, const forward_iterator<U>& y)
{
    return !(x == y);
}

} // namespace lean_vector_detail

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
    hi_assert_not_null(i);

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
    types_test<int>();
    types_test<int *>();
    types_test<lean_vector_detail::Copyable>();
    static_assert((std::is_same<lean_vector<char>::allocator_type, std::allocator<char>>::value), "");
}

TEST(lean_vector, capacity_empty)
{
    lean_vector<int> v;
    ASSERT_EQ(v.capacity(), v.short_capacity());
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
    ASSERT_EQ(v1.capacity(), v1.short_capacity());
    ASSERT_EQ(v2.size(), 3);
    ASSERT_EQ(v2.capacity(), v2.short_capacity());
}

TEST(lean_vector, swap_short_long)
{
    lean_vector<int> v1(3);
    lean_vector<int> v2(200);
    v1.swap(v2);
    ASSERT_EQ(v1.size(), 200);
    ASSERT_EQ(v1.capacity(), 200);
    ASSERT_EQ(v2.size(), 3);
    ASSERT_EQ(v2.capacity(), v2.short_capacity());
}

TEST(lean_vector, swap_long_short)
{
    lean_vector<int> v1(100);
    lean_vector<int> v2(5);
    v1.swap(v2);
    ASSERT_EQ(v1.size(), 5);
    ASSERT_EQ(v1.capacity(), v1.short_capacity());
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

template<typename Vec>
static void assign_initializer_list_test(Vec& v)
{
    v.assign({3, 4, 5, 6});
    ASSERT_EQ(v.size(), 4);
    ASSERT_EQ(v[0], 3);
    ASSERT_EQ(v[1], 4);
    ASSERT_EQ(v[2], 5);
    ASSERT_EQ(v[3], 6);
}

TEST(lean_vector, assign_initializer_list)
{
    typedef lean_vector<int> V;
    V d1;
    V d2;
    d2.reserve(10); // no reallocation during assign.
    assign_initializer_list_test(d1);
    assign_initializer_list_test(d2);
}

TEST(lean_vector, assign_forward_iter_iter)
{
    int arr1[] = {42};
    int arr2[] = {1, 101, 42};
    using T = lean_vector_detail::EmplaceConstructibleMoveableAndAssignable<int>;
    using It = lean_vector_detail::forward_iterator<int *>;
    {
        lean_vector<T> v;
        v.assign(It(arr1), It(std::end(arr1)));
        ASSERT_EQ(v[0].value, 42);
    }
    {
        lean_vector<T> v;
        v.assign(It(arr2), It(std::end(arr2)));
        ASSERT_EQ(v[0].value, 1);
        ASSERT_EQ(v[1].value, 101);
        ASSERT_EQ(v[2].value, 42);
    }
}

TEST(lean_vector, assign_input_iter_iter)
{
    int arr1[] = {42};
    int arr2[] = {1, 101, 42};
    using T = lean_vector_detail::EmplaceConstructibleMoveableAndAssignable<int>;
    using It = lean_vector_detail::input_iterator<int *>;
    {
        lean_vector<T> v;
        v.assign(It(arr1), It(std::end(arr1)));
        ASSERT_EQ(v[0].copied, 0);
        ASSERT_EQ(v[0].value, 42);
    }
    {
        lean_vector<T> v;
        v.assign(It(arr2), It(std::end(arr2)));
        ASSERT_EQ(v[0].value, 1);
        ASSERT_EQ(v[1].value, 101);
        ASSERT_EQ(v[2].copied, 0);
        ASSERT_EQ(v[2].value, 42);
    }
}

static void assign_size_value_test(auto& v)
{
    v.assign(5, 6);
    ASSERT_EQ(v.size(), 5);
    for (auto i = 0; i != 5; ++i) {
        ASSERT_EQ(v[i], 6);
    }
}

TEST(lean_vector, assign_size_value)
{
    typedef lean_vector<int> V;
    V d1;
    V d2;
    d2.reserve(10); // no reallocation during assign.
    assign_size_value_test(d1);
    assign_size_value_test(d2);
}

TEST(lean_vector, construct_size)
{
    typedef lean_vector<int> V;

    V v(50);
    ASSERT_EQ(v.size(), 50);
    for (auto it = v.cbegin(); it != v.cend(); ++it) {
        ASSERT_EQ(*it, V::value_type{});
    }
}

TEST(lean_vector, construct_size_value)
{
    typedef lean_vector<int> V;

    V v(50, 3);
    ASSERT_EQ(v.size(), 50);
    for (auto it = v.cbegin(); it != v.cend(); ++it) {
        ASSERT_EQ(*it, 3);
    }
}

TEST(lean_vector, construct_copy)
{
    typedef lean_vector<int> V;

    int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 7, 6, 5, 4, 3, 1, 0};
    int *an = a + sizeof(a) / sizeof(a[0]);
    V x(a, an);
    V c(x);
    ASSERT_EQ(x.size(), c.size());
    ASSERT_EQ(x, c);
}

TEST(lean_vector, construct_deduction)
{
    //  Test the explicit deduction guides
    {
        const int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        lean_vector vec(std::begin(arr), std::end(arr));

        static_assert(std::is_same_v<decltype(vec), lean_vector<int>>);
        ASSERT_TRUE(std::equal(vec.begin(), vec.end(), std::begin(arr), std::end(arr)));
    }

    //  Test the implicit deduction guides

    {
        //  We don't expect this one to work.
        //  std::vector vec(std::allocator<int>()); // vector (allocator &)
    }

    {
        lean_vector vec(1, lean_vector_detail::Copyable{}); // vector (size_type, T)
        static_assert(std::is_same_v<decltype(vec)::value_type, lean_vector_detail::Copyable>);
        static_assert(std::is_same_v<decltype(vec)::allocator_type, std::allocator<lean_vector_detail::Copyable>>);
        ASSERT_EQ(vec.size(), 1);
    }

    {
        lean_vector vec{1U, 2U, 3U, 4U, 5U}; // vector(initializer-list)
        static_assert(std::is_same_v<decltype(vec)::value_type, unsigned>);
        ASSERT_EQ(vec.size(), 5);
        ASSERT_EQ(vec[2], 3U);
    }

    {
        lean_vector<long double> source;
        lean_vector vec(source); // vector(vector &)
        static_assert(std::is_same_v<decltype(vec)::value_type, long double>);
        static_assert(std::is_same_v<decltype(vec)::allocator_type, std::allocator<long double>>);
        ASSERT_EQ(vec.size(), 0);
    }

    //  A couple of vector<bool> tests, too!
    {
        lean_vector vec(3, true); // vector(initializer-list)
        static_assert(std::is_same_v<decltype(vec)::value_type, bool>);
        static_assert(std::is_same_v<decltype(vec)::allocator_type, std::allocator<bool>>);
        ASSERT_EQ(vec.size(), 3);
        ASSERT_TRUE(vec[0] && vec[1] && vec[2]);
    }

    {
        lean_vector<bool> source;
        lean_vector vec(source); // vector(vector &)
        static_assert(std::is_same_v<decltype(vec)::value_type, bool>);
        static_assert(std::is_same_v<decltype(vec)::allocator_type, std::allocator<bool>>);
        ASSERT_EQ(vec.size(), 0);
    }
}

//TEST(lean_vector, construct_default_recursive)
//{
//    struct X {
//        lean_vector<X> q;
//    };
//
//    X x;
//    ASSERT_EQ(x.q.size(), 0);
//}

TEST(lean_vector, construct_default_noexcept)
{
    typedef lean_vector<lean_vector_detail::MoveOnly> C;
    static_assert(std::is_nothrow_default_constructible<C>::value);
}

TEST(lean_vector, destruct_noexcept)
{
    typedef lean_vector<lean_vector_detail::MoveOnly> C;
    static_assert(std::is_nothrow_destructible<C>::value);
}

TEST(lean_vector, construct_initializer_list)
{
    lean_vector<int> d = {3, 4, 5, 6};
    ASSERT_EQ(d.size(), 4);
    ASSERT_EQ(d[0], 3);
    ASSERT_EQ(d[1], 4);
    ASSERT_EQ(d[2], 5);
    ASSERT_EQ(d[3], 6);
}

TEST(lean_vector, construct_move)
{
    int a1[] = {1, 3, 7, 9, 10};
    lean_vector<int> c1(a1, a1 + sizeof(a1) / sizeof(a1[0]));
    lean_vector<int> c2 = std::move(c1);
    c1.clear();
    ASSERT_EQ(c1.size(), 0);
    ASSERT_EQ(c2.size(), 5);
    ASSERT_EQ(c2[0], 1);
    ASSERT_EQ(c2[1], 3);
    ASSERT_EQ(c2[2], 7);
    ASSERT_EQ(c2[3], 9);
    ASSERT_EQ(c2[4], 10);
}

TEST(lean_vector, construct_move_assign_noexcept)
{
    typedef lean_vector<lean_vector_detail::MoveOnly> C;
    static_assert(std::is_nothrow_move_assignable<C>::value);
}

TEST(lean_vector, construct_move_noexcept)
{
    typedef lean_vector<lean_vector_detail::MoveOnly> C;
    static_assert(std::is_nothrow_move_constructible<C>::value);
}

TEST(lean_vector, construct_op_equal_initializer_list)
{
    lean_vector<int> d;
    d = {3, 4, 5, 6};
    ASSERT_EQ(d.size(), 4);
    ASSERT_EQ(d[0], 3);
    ASSERT_EQ(d[1], 4);
    ASSERT_EQ(d[2], 5);
    ASSERT_EQ(d[3], 6);
}

TEST(lean_vector, data)
{
    struct Nasty {
        Nasty() : i_(0) {}
        Nasty(int i) : i_(i) {}
        ~Nasty() {}

        Nasty *operator&() const
        {
            assert(false);
            return nullptr;
        }
        int i_;
    };

    {
        lean_vector<int> v;
        ASSERT_EQ(v.data(), nullptr);
    }
    {
        lean_vector<int> v(100);
        ASSERT_EQ(v.data(), std::addressof(v.front()));
    }
    {
        lean_vector<Nasty> v(100);
        ASSERT_EQ(v.data(), std::addressof(v.front()));
    }
}

TEST(lean_vector, data_const)
{
    struct Nasty {
        Nasty() : i_(0) {}
        Nasty(int i) : i_(i) {}
        ~Nasty() {}

        Nasty *operator&() const
        {
            assert(false);
            return nullptr;
        }
        int i_;
    };

    {
        const std::vector<int> v;
        ASSERT_EQ(v.data(), nullptr);
    }
    {
        const std::vector<int> v(100);
        ASSERT_EQ(v.data(), std::addressof(v.front()));
    }
    {
        std::vector<Nasty> v(100);
        ASSERT_EQ(v.data(), std::addressof(v.front()));
    }
}

template<typename S, typename U>
void erase_test0(S s, U val, S expected)
{
    static_assert(std::is_same_v<typename S::size_type, decltype(erase(s, val))>);
    erase(s, val);
    ASSERT_EQ(s, expected);
}

template<typename S>
static void erase_test()
{
    erase_test0(S(), 1, S());

    erase_test0(S({1}), 1, S());
    erase_test0(S({1}), 2, S({1}));

    erase_test0(S({1, 2}), 1, S({2}));
    erase_test0(S({1, 2}), 2, S({1}));
    erase_test0(S({1, 2}), 3, S({1, 2}));
    erase_test0(S({1, 1}), 1, S());
    erase_test0(S({1, 1}), 3, S({1, 1}));

    erase_test0(S({1, 2, 3}), 1, S({2, 3}));
    erase_test0(S({1, 2, 3}), 2, S({1, 3}));
    erase_test0(S({1, 2, 3}), 3, S({1, 2}));
    erase_test0(S({1, 2, 3}), 4, S({1, 2, 3}));

    erase_test0(S({1, 1, 1}), 1, S());
    erase_test0(S({1, 1, 1}), 2, S({1, 1, 1}));
    erase_test0(S({1, 1, 2}), 1, S({2}));
    erase_test0(S({1, 1, 2}), 2, S({1, 1}));
    erase_test0(S({1, 1, 2}), 3, S({1, 1, 2}));
    erase_test0(S({1, 2, 2}), 1, S({2, 2}));
    erase_test0(S({1, 2, 2}), 2, S({1}));
    erase_test0(S({1, 2, 2}), 3, S({1, 2, 2}));

    //  Test cross-type erasure
    using opt = std::optional<typename S::value_type>;
    erase_test0(S({1, 2, 1}), opt(), S({1, 2, 1}));
    erase_test0(S({1, 2, 1}), opt(1), S({2}));
    erase_test0(S({1, 2, 1}), opt(2), S({1, 1}));
    erase_test0(S({1, 2, 1}), opt(3), S({1, 2, 1}));
}

TEST(lean_vector, erase)
{
    erase_test<lean_vector<int>>();
    erase_test<lean_vector<long>>();
    erase_test<lean_vector<double>>();
}

template<class S, class Pred>
void erase_if_test0(S s, Pred p, S expected)
{
    static_assert(std::is_same_v<typename S::size_type, decltype(erase_if(s, p))>);
    erase_if(s, p);
    ASSERT_EQ(s, expected);
}

template<typename S>
void erase_if_test()
{
    auto is1 = [](auto v) {
        return v == 1;
    };
    auto is2 = [](auto v) {
        return v == 2;
    };
    auto is3 = [](auto v) {
        return v == 3;
    };
    auto is4 = [](auto v) {
        return v == 4;
    };
    auto True = [](auto) {
        return true;
    };
    auto False = [](auto) {
        return false;
    };

    erase_if_test0(S(), is1, S());

    erase_if_test0(S({1}), is1, S());
    erase_if_test0(S({1}), is2, S({1}));

    erase_if_test0(S({1, 2}), is1, S({2}));
    erase_if_test0(S({1, 2}), is2, S({1}));
    erase_if_test0(S({1, 2}), is3, S({1, 2}));
    erase_if_test0(S({1, 1}), is1, S());
    erase_if_test0(S({1, 1}), is3, S({1, 1}));

    erase_if_test0(S({1, 2, 3}), is1, S({2, 3}));
    erase_if_test0(S({1, 2, 3}), is2, S({1, 3}));
    erase_if_test0(S({1, 2, 3}), is3, S({1, 2}));
    erase_if_test0(S({1, 2, 3}), is4, S({1, 2, 3}));

    erase_if_test0(S({1, 1, 1}), is1, S());
    erase_if_test0(S({1, 1, 1}), is2, S({1, 1, 1}));
    erase_if_test0(S({1, 1, 2}), is1, S({2}));
    erase_if_test0(S({1, 1, 2}), is2, S({1, 1}));
    erase_if_test0(S({1, 1, 2}), is3, S({1, 1, 2}));
    erase_if_test0(S({1, 2, 2}), is1, S({2, 2}));
    erase_if_test0(S({1, 2, 2}), is2, S({1}));
    erase_if_test0(S({1, 2, 2}), is3, S({1, 2, 2}));

    erase_if_test0(S({1, 2, 3}), True, S());
    erase_if_test0(S({1, 2, 3}), False, S({1, 2, 3}));
}

TEST(lean_vector, erase_if)
{
    erase_if_test<std::vector<int>>();
    erase_if_test<std::vector<long>>();
    erase_if_test<std::vector<double>>();
}

TEST(lean_vector, clear)
{
    int a[] = {1, 2, 3};
    lean_vector<int> c(a, a + 3);
    static_assert(noexcept(c.clear()));
    c.clear();
    ASSERT_TRUE(c.empty());
}

TEST(lean_vector, emplace)
{
    class A {
        int i_;
        double d_;

        A(const A&) = delete;
        A& operator=(const A&) = delete;

    public:
        A(int i, double d) : i_(i), d_(d) {}

        A(A&& a) : i_(a.i_), d_(a.d_)
        {
            a.i_ = 0;
            a.d_ = 0;
        }

        A& operator=(A&& a)
        {
            i_ = a.i_;
            d_ = a.d_;
            a.i_ = 0;
            a.d_ = 0;
            return *this;
        }

        int geti() const
        {
            return i_;
        }
        double getd() const
        {
            return d_;
        }
    };

    lean_vector<A> c;
    lean_vector<A>::iterator i = c.emplace(c.cbegin(), 2, 3.5);
    ASSERT_EQ(i, c.begin());
    ASSERT_EQ(c.size(), 1);
    ASSERT_EQ(c.front().geti(), 2);
    ASSERT_EQ(c.front().getd(), 3.5);
    i = c.emplace(c.cend(), 3, 4.5);
    ASSERT_EQ(i, c.end() - 1);
    ASSERT_EQ(c.size(), 2);
    ASSERT_EQ(c.front().geti(), 2);
    ASSERT_EQ(c.front().getd(), 3.5);
    ASSERT_EQ(c.back().geti(), 3);
    ASSERT_EQ(c.back().getd(), 4.5);
    i = c.emplace(c.cbegin() + 1, 4, 6.5);
    ASSERT_EQ(i, c.begin() + 1);
    ASSERT_EQ(c.size(), 3);
    ASSERT_EQ(c.front().geti(), 2);
    ASSERT_EQ(c.front().getd(), 3.5);
    ASSERT_EQ(c[1].geti(), 4);
    ASSERT_EQ(c[1].getd(), 6.5);
    ASSERT_EQ(c.back().geti(), 3);
    ASSERT_EQ(c.back().getd(), 4.5);
}

TEST(lean_vector, emplace_back)
{
    class A {
        int i_;
        double d_;

        A(const A&) = delete;
        A& operator=(const A&) = delete;

    public:
        A(int i, double d) : i_(i), d_(d) {}

        A(A&& a) : i_(a.i_), d_(a.d_)
        {
            a.i_ = 0;
            a.d_ = 0;
        }

        A& operator=(A&& a)
        {
            i_ = a.i_;
            d_ = a.d_;
            a.i_ = 0;
            a.d_ = 0;
            return *this;
        }

        int geti() const
        {
            return i_;
        }
        double getd() const
        {
            return d_;
        }
    };

    lean_vector<A> c;
    lean_vector<A>::iterator i = c.emplace(c.cbegin(), 2, 3.5);
    ASSERT_EQ(i, c.begin());
    ASSERT_EQ(c.size(), 1);
    ASSERT_EQ(c.front().geti(), 2);
    ASSERT_EQ(c.front().getd(), 3.5);
    i = c.emplace(c.cend(), 3, 4.5);
    ASSERT_EQ(i, c.end() - 1);
    ASSERT_EQ(c.size(), 2);
    ASSERT_EQ(c.front().geti(), 2);
    ASSERT_EQ(c.front().getd(), 3.5);
    ASSERT_EQ(c.back().geti(), 3);
    ASSERT_EQ(c.back().getd(), 4.5);
    i = c.emplace(c.cbegin() + 1, 4, 6.5);
    ASSERT_EQ(i, c.begin() + 1);
    ASSERT_EQ(c.size(), 3);
    ASSERT_EQ(c.front().geti(), 2);
    ASSERT_EQ(c.front().getd(), 3.5);
    ASSERT_EQ(c[1].geti(), 4);
    ASSERT_EQ(c[1].getd(), 6.5);
    ASSERT_EQ(c.back().geti(), 3);
    ASSERT_EQ(c.back().getd(), 4.5);
}

TEST(lean_vector, emplace_extra1)
{
    {
        lean_vector<int> v;
        v.reserve(3);
        v = {1, 2, 3};
        v.emplace(v.begin(), v.back());
        ASSERT_EQ(v[0], 3);
    }
    {
        lean_vector<int> v;
        v.reserve(4);
        v = {1, 2, 3};
        v.emplace(v.begin(), v.back());
        ASSERT_EQ(v[0], 3);
    }

    {
        lean_vector<int> v;
        v.reserve(5);
        v = {1, 2, 3};
        v.emplace(v.begin(), v.back());
        ASSERT_EQ(v[0], 3);
    }
    {
        lean_vector<int> v;
        v.reserve(6);
        v = {1, 2, 3};
        v.emplace(v.begin(), v.back());
        ASSERT_EQ(v[0], 3);
    }

    {
        lean_vector<int> v;
        v.reserve(5);
        v = {1, 2, 3, 4, 5};
        v.emplace(v.begin(), v.back());
        ASSERT_EQ(v[0], 5);
    }
    {
        lean_vector<int> v;
        v.reserve(6);
        v = {1, 2, 3, 4, 5};
        v.emplace(v.begin(), v.back());
        ASSERT_EQ(v[0], 5);
    }
}

TEST(lean_vector, erase_iter)
{
    int a1[] = {1, 2, 3};
    lean_vector<int> l1(a1, a1 + 3);
    lean_vector<int>::const_iterator i = l1.begin();
    ++i;
    lean_vector<int>::iterator j = l1.erase(i);
    ASSERT_EQ(l1.size(), 2);
    ASSERT_EQ(std::distance(l1.begin(), l1.end()), 2);
    ASSERT_EQ(*j, 3);
    ASSERT_EQ(*l1.begin(), 1);
    ASSERT_EQ(*std::next(l1.begin()), 3);
    j = l1.erase(j);
    ASSERT_EQ(j, l1.end());
    ASSERT_EQ(l1.size(), 1);
    ASSERT_EQ(std::distance(l1.begin(), l1.end()), 1);
    ASSERT_EQ(*l1.begin(), 1);
    j = l1.erase(l1.begin());
    ASSERT_EQ(j, l1.end());
    ASSERT_EQ(l1.size(), 0);
    ASSERT_EQ(std::distance(l1.begin(), l1.end()), 0);
}

TEST(lean_vector, erase_iter_LWG2853)
{
    lean_vector_detail::Throws arr[] = {1, 2, 3};
    lean_vector<lean_vector_detail::Throws> v(arr, arr + 3);
    lean_vector_detail::Throws::sThrows = true;
    v.erase(v.begin());
    auto tmp = v.end();
    v.erase(--tmp);
    v.erase(v.begin());
    ASSERT_EQ(v.size(), 0);
    lean_vector_detail::Throws::sThrows = false;
}

TEST(lean_vector, erase_iter_iter)
{
    int a1[] = {1, 2, 3};
    {
        lean_vector<int> l1(a1, a1 + 3);
        lean_vector<int>::iterator i = l1.erase(l1.cbegin(), l1.cbegin());
        ASSERT_EQ(l1.size(), 3);
        ASSERT_EQ(std::distance(l1.cbegin(), l1.cend()), 3);
        ASSERT_EQ(i, l1.begin());
    }
    {
        lean_vector<int> l1(a1, a1 + 3);
        lean_vector<int>::iterator i = l1.erase(l1.cbegin(), std::next(l1.cbegin()));
        ASSERT_EQ(l1.size(), 2);
        ASSERT_EQ(std::distance(l1.cbegin(), l1.cend()), 2);
        ASSERT_EQ(i, l1.begin());
        ASSERT_EQ(l1, lean_vector<int>(a1 + 1, a1 + 3));
    }
    {
        lean_vector<int> l1(a1, a1 + 3);
        lean_vector<int>::iterator i = l1.erase(l1.cbegin(), std::next(l1.cbegin(), 2));
        ASSERT_EQ(l1.size(), 1);
        ASSERT_EQ(std::distance(l1.cbegin(), l1.cend()), 1);
        ASSERT_EQ(i, l1.begin());
        ASSERT_EQ(l1, lean_vector<int>(a1 + 2, a1 + 3));
    }
    {
        lean_vector<int> l1(a1, a1 + 3);
        lean_vector<int>::iterator i = l1.erase(l1.cbegin(), std::next(l1.cbegin(), 3));
        ASSERT_EQ(l1.size(), 0);
        ASSERT_EQ(std::distance(l1.cbegin(), l1.cend()), 0);
        ASSERT_EQ(i, l1.begin());
    }
    {
        lean_vector<lean_vector<int>> outer(2, lean_vector<int>(1));
        outer.erase(outer.begin(), outer.begin());
        ASSERT_EQ(outer.size(), 2);
        ASSERT_EQ(outer[0].size(), 1);
        ASSERT_EQ(outer[1].size(), 1);
    }
}

TEST(lean_vector, erase_iter_iter_LWG2863)
{
    lean_vector_detail::Throws arr[] = {1, 2, 3};
    lean_vector<lean_vector_detail::Throws> v(arr, arr + 3);
    lean_vector_detail::Throws::sThrows = true;
    auto tmp = v.end();
    v.erase(v.begin(), --tmp);
    ASSERT_EQ(v.size(), 1);
    v.erase(v.begin(), v.end());
    ASSERT_EQ(v.size(), 0);
    lean_vector_detail::Throws::sThrows = false;
}

TEST(lean_vector, insert_iter_initializer_list)
{
    lean_vector<int> d(10, 1);
    lean_vector<int>::iterator i = d.insert(d.cbegin() + 2, {3, 4, 5, 6});
    ASSERT_EQ(d.size(), 14);
    ASSERT_EQ(i, d.begin() + 2);
    ASSERT_EQ(d[0], 1);
    ASSERT_EQ(d[1], 1);
    ASSERT_EQ(d[2], 3);
    ASSERT_EQ(d[3], 4);
    ASSERT_EQ(d[4], 5);
    ASSERT_EQ(d[5], 6);
    ASSERT_EQ(d[6], 1);
    ASSERT_EQ(d[7], 1);
    ASSERT_EQ(d[8], 1);
    ASSERT_EQ(d[9], 1);
    ASSERT_EQ(d[10], 1);
    ASSERT_EQ(d[11], 1);
    ASSERT_EQ(d[12], 1);
    ASSERT_EQ(d[13], 1);
}

TEST(lean_vector, insert_iter_iter_iter)
{
    {
        typedef lean_vector<int> V;
        V v(100);
        int a[] = {1, 2, 3, 4, 5};
        const int N = sizeof(a) / sizeof(a[0]);
        V::iterator i = v.insert(
            v.cbegin() + 10,
            lean_vector_detail::input_iterator<const int *>(a),
            lean_vector_detail::input_iterator<const int *>(a + N));
        ASSERT_EQ(v.size(), 100 + N);
        ASSERT_EQ(i, v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            ASSERT_EQ(v[j], 0);
        for (std::size_t k = 0; k < N; ++j, ++k)
            ASSERT_EQ(v[j], a[k]);
        for (; j < 105; ++j)
            ASSERT_EQ(v[j], 0);
    }
    {
        typedef lean_vector<int> V;
        V v(100);
        int a[] = {1, 2, 3, 4, 5};
        const int N = sizeof(a) / sizeof(a[0]);
        V::iterator i = v.insert(
            v.cbegin() + 10,
            lean_vector_detail::forward_iterator<const int *>(a),
            lean_vector_detail::forward_iterator<const int *>(a + N));
        ASSERT_EQ(v.size(), 100 + N);
        ASSERT_EQ(i, v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            ASSERT_EQ(v[j], 0);
        for (std::size_t k = 0; k < N; ++j, ++k)
            ASSERT_EQ(v[j], a[k]);
        for (; j < 105; ++j)
            ASSERT_EQ(v[j], 0);
    }
    {
        typedef lean_vector<int> V;
        V v(100);
        while (v.size() < v.capacity())
            v.push_back(0); // force reallocation
        size_t sz = v.size();
        int a[] = {1, 2, 3, 4, 5};
        const unsigned N = sizeof(a) / sizeof(a[0]);
        V::iterator i = v.insert(
            v.cbegin() + 10,
            lean_vector_detail::forward_iterator<const int *>(a),
            lean_vector_detail::forward_iterator<const int *>(a + N));
        ASSERT_EQ(v.size(), sz + N);
        ASSERT_EQ(i, v.begin() + 10);
        std::size_t j;
        for (j = 0; j < 10; ++j)
            ASSERT_EQ(v[j], 0);
        for (std::size_t k = 0; k < N; ++j, ++k)
            ASSERT_EQ(v[j], a[k]);
        for (; j < v.size(); ++j)
            ASSERT_EQ(v[j], 0);
    }
    {
        typedef lean_vector<int> V;
        V v(100);
        v.reserve(128); // force no reallocation
        size_t sz = v.size();
        int a[] = {1, 2, 3, 4, 5};
        const unsigned N = sizeof(a) / sizeof(a[0]);
        V::iterator i = v.insert(
            v.cbegin() + 10,
            lean_vector_detail::forward_iterator<const int *>(a),
            lean_vector_detail::forward_iterator<const int *>(a + N));
        ASSERT_EQ(v.size(), sz + N);
        ASSERT_EQ(i, v.begin() + 10);
        std::size_t j;
        for (j = 0; j < 10; ++j)
            ASSERT_EQ(v[j], 0);
        for (std::size_t k = 0; k < N; ++j, ++k)
            ASSERT_EQ(v[j], a[k]);
        for (; j < v.size(); ++j)
            ASSERT_EQ(v[j], 0);
    }
}

TEST(lean_vector, insert_iter_rvalue)
{
    lean_vector<lean_vector_detail::MoveOnly> v(100);
    lean_vector<lean_vector_detail::MoveOnly>::iterator i = v.insert(v.cbegin() + 10, lean_vector_detail::MoveOnly(3));
    ASSERT_EQ(v.size(), 101);
    ASSERT_EQ(i, v.begin() + 10);
    int j;
    for (j = 0; j < 10; ++j)
        ASSERT_EQ(v[j], lean_vector_detail::MoveOnly());
    ASSERT_EQ(v[j], lean_vector_detail::MoveOnly(3));
    for (++j; j < 101; ++j)
        ASSERT_EQ(v[j], lean_vector_detail::MoveOnly());
}

TEST(lean_vector, insert_iter_size_value)
{
    {
        lean_vector<int> v(100);
        lean_vector<int>::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        ASSERT_EQ(v.size(), 105);
        ASSERT_EQ(i, v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            ASSERT_EQ(v[j], 0);
        for (; j < 15; ++j)
            ASSERT_EQ(v[j], 1);
        for (++j; j < 105; ++j)
            ASSERT_EQ(v[j], 0);
    }
    {
        lean_vector<int> v(100);
        while (v.size() < v.capacity())
            v.push_back(0); // force reallocation
        size_t sz = v.size();
        lean_vector<int>::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        ASSERT_EQ(v.size(), sz + 5);
        ASSERT_EQ(i, v.begin() + 10);
        std::size_t j;
        for (j = 0; j < 10; ++j)
            ASSERT_EQ(v[j], 0);
        for (; j < 15; ++j)
            ASSERT_EQ(v[j], 1);
        for (++j; j < v.size(); ++j)
            ASSERT_EQ(v[j], 0);
    }
    {
        lean_vector<int> v(100);
        v.reserve(128); // force no reallocation
        size_t sz = v.size();
        lean_vector<int>::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        ASSERT_EQ(v.size(), sz + 5);
        ASSERT_EQ(i, v.begin() + 10);
        std::size_t j;
        for (j = 0; j < 10; ++j)
            ASSERT_EQ(v[j], 0);
        for (; j < 15; ++j)
            ASSERT_EQ(v[j], 1);
        for (++j; j < v.size(); ++j)
            ASSERT_EQ(v[j], 0);
    }
}

TEST(lean_vector, insert_iter_value)
{
    {
        lean_vector<int> v(100);
        lean_vector<int>::iterator i = v.insert(v.cbegin() + 10, 1);
        ASSERT_EQ(v.size(), 101);
        ASSERT_EQ(i, v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            ASSERT_EQ(v[j], 0);
        ASSERT_EQ(v[j], 1);
        for (++j; j < 101; ++j)
            ASSERT_EQ(v[j], 0);
    }
    {
        lean_vector<int> v(100);
        while (v.size() < v.capacity())
            v.push_back(0); // force reallocation
        size_t sz = v.size();
        lean_vector<int>::iterator i = v.insert(v.cbegin() + 10, 1);
        ASSERT_EQ(v.size(), sz + 1);
        ASSERT_EQ(i, v.begin() + 10);
        std::size_t j;
        for (j = 0; j < 10; ++j)
            ASSERT_EQ(v[j], 0);
        ASSERT_EQ(v[j], 1);
        for (++j; j < v.size(); ++j)
            ASSERT_EQ(v[j], 0);
    }
    {
        lean_vector<int> v(100);
        while (v.size() < v.capacity())
            v.push_back(0);
        v.pop_back();
        v.pop_back(); // force no reallocation
        size_t sz = v.size();
        lean_vector<int>::iterator i = v.insert(v.cbegin() + 10, 1);
        ASSERT_EQ(v.size(), sz + 1);
        ASSERT_EQ(i, v.begin() + 10);
        std::size_t j;
        for (j = 0; j < 10; ++j)
            ASSERT_EQ(v[j], 0);
        ASSERT_EQ(v[j], 1);
        for (++j; j < v.size(); ++j)
            ASSERT_EQ(v[j], 0);
    }
}

TEST(lean_vector, pop_back)
{
    lean_vector<int> c;
    c.push_back(1);
    ASSERT_EQ(c.size(), 1);
    c.pop_back();
    ASSERT_EQ(c.size(), 0);
}

TEST(lean_vector, pop_back_LWG526)
{
    int arr[] = {0, 1, 2, 3, 4};
    int sz = 5;
    lean_vector<int> c(arr, arr + sz);
    while (c.size() < c.capacity())
        c.push_back(sz++);
    c.push_back(c.front());
    ASSERT_EQ(c.back(), 0);
    for (int i = 0; i < sz; ++i)
        ASSERT_EQ(c[i], i);
}

TEST(lean_vector, push_back)
{
    lean_vector<int> c;
    c.push_back(0);
    ASSERT_EQ(c.size(), 1);
    for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
        ASSERT_EQ(c[j], j);
    c.push_back(1);
    ASSERT_EQ(c.size(), 2);
    for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
        ASSERT_EQ(c[j], j);
    c.push_back(2);
    ASSERT_EQ(c.size(), 3);
    for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
        ASSERT_EQ(c[j], j);
    c.push_back(3);
    ASSERT_EQ(c.size(), 4);
    for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
        ASSERT_EQ(c[j], j);
    c.push_back(4);
    ASSERT_EQ(c.size(), 5);
    for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
        ASSERT_EQ(c[j], j);
}

TEST(lean_vector, push_back_exception_safety)
{
    lean_vector_detail::Throws instance(42);
    lean_vector<lean_vector_detail::Throws> vec;

    vec.push_back(instance);
    lean_vector<lean_vector_detail::Throws> vec2(vec);

    lean_vector_detail::Throws::sThrows = true;
    ASSERT_ANY_THROW(vec.push_back(instance));
    ASSERT_EQ(vec, vec2);
    lean_vector_detail::Throws::sThrows = false;
}

TEST(lean_vector, swap)
{
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        lean_vector<int> c1(a1, a1 + sizeof(a1) / sizeof(a1[0]));
        lean_vector<int> c2(a2, a2 + sizeof(a2) / sizeof(a2[0]));
        swap(c1, c2);
        ASSERT_EQ(c1 , lean_vector<int>(a2, a2 + sizeof(a2) / sizeof(a2[0])));
        ASSERT_EQ(c2 , lean_vector<int>(a1, a1 + sizeof(a1) / sizeof(a1[0])));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        lean_vector<int> c1(a1, a1);
        lean_vector<int> c2(a2, a2 + sizeof(a2) / sizeof(a2[0]));
        swap(c1, c2);
        ASSERT_EQ(c1 , lean_vector<int>(a2, a2 + sizeof(a2) / sizeof(a2[0])));
        ASSERT_TRUE(c2.empty());
        ASSERT_EQ(std::distance(c2.begin(), c2.end()) , 0);
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        lean_vector<int> c1(a1, a1 + sizeof(a1) / sizeof(a1[0]));
        lean_vector<int> c2(a2, a2);
        swap(c1, c2);
        ASSERT_TRUE(c1.empty());
        ASSERT_EQ(std::distance(c1.begin(), c1.end()), 0);
        ASSERT_EQ(c2 , lean_vector<int>(a1, a1 + sizeof(a1) / sizeof(a1[0])));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        lean_vector<int> c1(a1, a1);
        lean_vector<int> c2(a2, a2);
        swap(c1, c2);
        ASSERT_TRUE(c1.empty());
        ASSERT_EQ(std::distance(c1.begin(), c1.end()), 0);
        ASSERT_TRUE(c2.empty());
        ASSERT_EQ(std::distance(c2.begin(), c2.end()), 0);
    }
}

TEST(lean_vector, swap_noexcept)
{
    typedef lean_vector<lean_vector_detail::MoveOnly> C;
    static_assert(noexcept(swap(std::declval<C&>(), std::declval<C&>())));
}

hi_warning_pop();
