// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "os_detect.hpp"
#include "assert.hpp"
#include <type_traits>
#include <cstdint>


namespace tt {

using namespace std::literals;

/*! Invariant should be the default for variables.
* C++ does have an invariant but it requires you to enter the 'const' keyword which
* is easy to forget. Using a single keyword 'ttlet' for an invariant makes it easier to notice
* when you have defined a variant.
*/
#ifndef ttlet
#define ttlet auto const
#endif

/*! Signed size/index into an array.
*/
using ssize_t = std::ptrdiff_t;

template<typename T>
std::remove_reference_t<T> rvalue_cast(T value)
{
    return value;
}


template<typename C>
struct nr_items {
    constexpr static ssize_t value = static_cast<ssize_t>(C::max) + 1;
};

template<typename C>
constexpr auto nr_items_v = nr_items<C>::value;


template <class C>
constexpr auto usize(const C& c) -> std::common_type_t<size_t, std::make_unsigned_t<decltype(c.size())>> 
{
    using R = std::common_type_t<size_t, std::make_unsigned_t<decltype(c.size())>>;
    return static_cast<R>(c.size());
}


#define ssizeof(x) (static_cast<ssize_t>(sizeof (x)))

}