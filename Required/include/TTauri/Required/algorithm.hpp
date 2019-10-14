// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/required.hpp"
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

template<typename It, typename T>
constexpr It rfind(It const begin, It const end, T const &value)
{
    auto i = end;
    do {
        i--;
        if (*i == value) {
            return i;
        }
    } while (i != begin);
    return end;
}

/*! For each cluster.
 * func() is executed for each cluster that is found between first-last.
 * A cluster is found between two seperators, a seperator is detected with IsClusterSeperator().
 * A cluster does not include the seperator itself.
 */
template<typename It, typename It, typename S, typename F>
void for_each_cluster(It first, It last, S IsClusterSeperator, F Function)
{
    for (size_t i = text.begin(); i != text.end();) {
        auto j = std::find_if(i + 1, last, IsClusterSeperator);
        Function(i, j);

        auto skipOverSeperator = (j == text.end()) ? 0 : 1;
        i = j + skipOverSeperator;
    }
}

}

