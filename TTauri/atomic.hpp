// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "os_detect.hpp"
#include "string_tag.hpp"
#include "counters.hpp"
#include <atomic>
#include <thread>
#include <chrono>

namespace TTauri {

inline void pause_cpu()
{
#if COMPILER == CC_MSVC
    //_mm_pause();
#else
    asm("pause");
#endif
}

template<typename T>
no_inline void contended_wait_for_transition(std::atomic<T> &state, T from, std::memory_order order=std::memory_order_seq_cst) no_inline_attr
{
    using namespace std::literals::chrono_literals;

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
force_inline void wait_for_transition(std::atomic<T> &state, T from, std::memory_order order=std::memory_order_seq_cst) force_inline_attr
{
    if (state.load(order) != from) {
        contended_wait_for_transition(state, from, order);
    }
}

template<string_tag BlockCounterTag=0,typename T>
no_inline void contended_transition(std::atomic<T> &state, T from, T to, std::memory_order order=std::memory_order_seq_cst) no_inline_attr
{
    using namespace std::literals::chrono_literals;

    if constexpr (BlockCounterTag != 0) {
        increment_counter<BlockCounterTag>();
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

template<string_tag BlockCounterTag=0,typename T>
force_inline void transition(std::atomic<T> &state, T from, T to, std::memory_order order=std::memory_order_seq_cst) force_inline_attr
{
    auto expect = from;
    if (ttauri_likely(state.compare_exchange_strong(expect, to, order))) {
        return;
    } else {
        return contended_transition<BlockCounterTag>(state, from, to, order);
    }
}


}

