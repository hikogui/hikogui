// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "os_detect.hpp"
#include <atomic>
#include <thread>
#include <chrono>

namespace TTauri {

inline void pause_cpu()
{
#if (COMPILER == COMPILER_MSVC)
    //_mm_pause();
#else
    asm("pause");
#endif
}

template<typename T>
void wait_for_transition(std::atomic<T> &state, T from, std::memory_order order=std::memory_order_seq_cst)
{
    using namespace std::literals::chrono_literals;

    for (auto i = 0; i < 5; i++) {
        if (ttauri_likely(state.load(order) == from)) {
            return;
        }
        pause_cpu();
    }

    auto backoff = 10ms;
    while (true) {
        if (state.load(order) == from) {
            return;
        }

        std::this_thread::sleep_for(backoff);
        if ((backoff *= 2) > 1s) {
            backoff = 1s;
        }
    }
}

template<typename T>
void transition(std::atomic<T> &state, T from, T to, std::memory_order order=std::memory_order_seq_cst)
{
    using namespace std::literals::chrono_literals;

    for (auto i = 0; i < 5; i++) {
        auto expect = from;
        if (ttauri_likely(state.compare_exchange_weak(expect, to, order))) {
            return;
        }
        pause_cpu();
    }

    auto backoff = 10ms;
    while (true) {
        auto expect = from;
        if (state.compare_exchange_weak(expect, to, order)) {
            return;
        }

        std::this_thread::sleep_for(backoff);
        if ((backoff *= 2) > 1s) {
            backoff = 1s;
        }
    }
}


}

