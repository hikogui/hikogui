// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include <algorithm>

namespace TTauri {

template<typename T, typename U, typename F>
inline T transform(const U &input, F operation)
{
    T result = {};
    result.reserve(input.size());
    std::transform(input.begin(), input.end(), std::back_inserter(result), operation);
    return result;
}

template<typename T, size_t N, typename F>
constexpr std::array<T, N> generate_array(F operation)
{
    std::array<T, N> a{};

    for (size_t i = 0; i < N; i++) {
        a.at(i) = operation(i);
    }

    return a;
}

template<typename T, typename F>
inline void erase_if(T &v, F operation)
{
    while (true) {
        let i = std::find_if(v.begin(), v.end(), operation);
        if (i == v.end()) {
            return;
        }
        v.erase(i);
    }
}


template<typename It, typename UnaryPredicate>
constexpr It rfind_if(It const begin, It const end, UnaryPredicate predicate)
{
    auto i = end;
    do {
        i--;
        if (predicate(*i)) {
            return i;
        }
    } while (i != begin);
    return end;
}

template<typename It, typename UnaryPredicate>
constexpr It rfind_if_not(It const begin, It const end, UnaryPredicate predicate)
{
    return rfind_if(begin, end, [](auto const &x) { return !predicate(x); });
}

template<typename It, typename T>
constexpr It rfind(It const begin, It const end, T const &value)
{
    return rfind_if(begin, end, [](auto const &x) { return x == value; });
}

/*! For each cluster.
 * func() is executed for each cluster that is found between first-last.
 * A cluster is found between two seperators, a seperator is detected with IsClusterSeperator().
 * A cluster does not include the seperator itself.
 */
template<typename It, typename S, typename F>
void for_each_cluster(It first, It last, S IsClusterSeperator, F Function)
{
    if (first == last) {
        return;
    }

    // If the first item is a cluster seperator skip over it.
    if (IsClusterSeperator(*first)) {
        first++;
    }

    for (auto i = first; i != last;) {
        auto j = std::find_if(i, last, IsClusterSeperator);
        Function(i, j);

        auto skipOverSeperator = (j == last) ? 0 : 1;
        i = j + skipOverSeperator;
    }
}

/*! Check if the 
 */
template<typename InputIt1, typename InputIt2>
bool starts_with(InputIt1 haystack_first, InputIt1 haystack_last, InputIt2 needle_first, InputIt2 needle_last) noexcept
{
    let [haystack_result, needle_result] = std::mismatch(haystack_first, haystack_last, needle_first, needle_last);
    return needle_result == needle_last;
}

template<typename Container1, typename Container2>
bool starts_with(Container1 haystack, Container2 needle) noexcept
{
    return starts_with(haystack.begin(), haystack.end(), needle.begin(), needle.end());
}

}

