
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

}

