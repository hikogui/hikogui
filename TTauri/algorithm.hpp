// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

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

template<typename T>
inline std::enable_if_t<!std::is_pointer_v<T>, T> middle(T begin, T end) noexcept
{
    return begin + std::distance(begin, end) / 2;
}

template<typename T>
inline std::enable_if_t<std::is_pointer_v<T>, T> middle(T begin, T end) noexcept
{
    return reinterpret_cast<T>((reinterpret_cast<intptr_t>(begin) + reinterpret_cast<intptr_t>(end)) / 2);;
}

template<typename T, typename U>
inline T binary_nearest_find(T begin, T end, U value) noexcept
{
    while (begin < end) {
        let m = middle(begin, end);

        if (value > *m) {
            begin = m + 1;
        } else if (value < *m) {
            end = m;
        } else {
            return m;
        }
    }
    return begin;
}
}

