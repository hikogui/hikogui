// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "counters.hpp"
#include "os_detect.hpp"
#include <atomic>
#include <thread>
#include <chrono>

namespace tt {

/** Wait for transition.
 * Wait until state has switched to the new state.
 * This function is for the contended state. It should not be inlined so that
 * not so much code is generated at the call site.
 *
 * @param state variable to monitor.
 * @param to The value the state needs to be before this function returns.
 */
template<typename CounterTag, typename T>
tt_no_inline void
contended_wait_for_transition(std::atomic<T> const &state, T to, std::memory_order order = std::memory_order_seq_cst)
{
    using namespace std::literals::chrono_literals;

    increment_counter<CounterTag>();

    auto backoff = 10ms;
    while (true) {
        if (state.load(order) == to) {
            return;
        }

        std::this_thread::sleep_for(backoff);
        if ((backoff *= 2) > 1s) {
            backoff = 1s;
        }
    }
}

/** Wait for transition.
 * Wait until state has switched to the new state.
 * This function is for the non-contended state. The code emitted on x86 should
 * be MOV,CMP,JNE. The JNE is taken on contended state.
 *
 * @param CounterTag tag to increment if the transition was contended.
 * @param state variable to monitor.
 * @param to The value the state needs to be before this function returns.
 */
template<typename CounterTag, typename T>
void wait_for_transition(std::atomic<T> const &state, T to, std::memory_order order = std::memory_order_seq_cst)
{
    if (state.load(order) != to) {
        [[unlikely]] contended_wait_for_transition<CounterTag>(state, to, order);
    }
}

/** Transition from one state to another.
 * This is the non-included version that is used for contended situation.
 *
 * @param state The state variable to monitor and modify.
 * @param from The value that state needs to have before changing it to `to`.
 * @param to The value to set once state has the value `from` .
 * @param order Memory order to use for this state variable.
 */
template<typename BlockCounterTag = void, typename T>
tt_no_inline void contended_transition(std::atomic<T> &state, T from, T to, std::memory_order order = std::memory_order_seq_cst)
{
    using namespace std::literals::chrono_literals;

    if constexpr (!std::is_same_v<BlockCounterTag, void>) {
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

/** Transition from one state to another.
 * This is the included version that is used for non-contended situation.
 * Should emit on x86: CMPXCHG,JNE
 *
 * @param state The state variable to monitor and modify.
 * @param from The value that state needs to have before changing it to `to`.
 * @param to The value to set once state has the value `from` .
 * @param order Memory order to use for this state variable.
 */
template<typename BlockCounterTag = void, typename T>
void transition(std::atomic<T> &state, T from, T to, std::memory_order order = std::memory_order_seq_cst)
{
    auto expect = from;
    if (state.compare_exchange_strong(expect, to, order)) {
        [[likely]] return;
    } else {
        return contended_transition<BlockCounterTag>(state, from, to, order);
    }
}

} // namespace tt
