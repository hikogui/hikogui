// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "math.hpp"
#include "byte_string.hpp"
#include "cast.hpp"
#include <atomic>

namespace tt {

template<typename T>
inline bstring get_bytes(T &generator, ssize_t count)
{
    decltype(generator()) random_number;

    auto data = bstring(static_cast<size_t>(count), std::byte{0});

    ssize_t i = 0;
    while (i < count - ssizeof(random_number) - 1) {
        random_number = generator();
        for (int j = 0; j < sizeof(random_number); j++) {
            data[i++] = static_cast<std::byte>(random_number);
            random_number >>= 8;
        }
    }

    random_number = generator();
    while (i < count) {
        data[i++] = static_cast<std::byte>(random_number);
        random_number >>= 8;
    }

    return data;
}


class pcg32 {
    static constexpr uint64_t multiplier = 6364136223846793005u;
    static constexpr uint64_t increment = 1442695040888963407u;

    uint64_t state;

public:
    pcg32(uint64_t seed = 0x4d595df4d0f33173) :
        state(seed + increment)
    {
        (*this)();
    }

    uint32_t operator()() {
        auto x = state;

        state = x * multiplier + increment;

        ttlet count = static_cast<unsigned int>(x >> 59);

        x ^= x >> 18;
        return rotr(static_cast<uint32_t>(x >> 27), count);
    }

    bstring get_bytes(ssize_t count) {
        return tt::get_bytes(*this, count);
    }
};

class atomic_pcg32 {
    static constexpr uint64_t multiplier = 6364136223846793005u;
    static constexpr uint64_t increment = 1442695040888963407u;

    std::atomic<uint64_t> state;

public:
    atomic_pcg32(uint64_t seed = 0x4d595df4d0f33173) :
        state(seed + increment)
    {
        (*this)();
    }

    uint32_t operator()() {
        auto x = state.load(std::memory_order::relaxed);
        uint64_t new_state;
        do {
            [[unlikely]] new_state = x * multiplier + increment;
        } while (!state.compare_exchange_weak(x, new_state, std::memory_order::relaxed));

        ttlet count = static_cast<unsigned int>(x >> 59);

        x ^= x >> 18;
        return rotr(static_cast<uint32_t>(x >> 27), count);
    }

    bstring get_bytes(ssize_t count) {
        return tt::get_bytes(*this, count);
    }
};

inline atomic_pcg32 global_pcg32 = atomic_pcg32();

}