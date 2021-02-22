// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

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
 * @param order The memory order to use during atomic loads.
 */
template<basic_fixed_string CounterTag, typename T>
tt_no_inline void
contended_wait_for_transition(std::atomic<T> const &state, T to, std::memory_order order = std::memory_order::seq_cst)
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
 * @tparam CounterTag tag to increment if the transition was contended.
 * @tparam T The underlying type of the atomic.
 * @param state variable to monitor.
 * @param to The value the state needs to be before this function returns.
 * @param order The memory order to use for the load atomic.
 */
template<basic_fixed_string CounterTag, typename T>
void wait_for_transition(std::atomic<T> const &state, T to, std::memory_order order = std::memory_order::seq_cst)
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
template<basic_fixed_string BlockCounterTag = "", typename T>
tt_no_inline void contended_transition(std::atomic<T> &state, T from, T to, std::memory_order order = std::memory_order::seq_cst)
{
    using namespace std::literals::chrono_literals;

    if constexpr (BlockCounterTag != "") {
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
template<basic_fixed_string BlockCounterTag = "", typename T>
void transition(std::atomic<T> &state, T from, T to, std::memory_order order = std::memory_order::seq_cst)
{
    auto expect = from;
    if (state.compare_exchange_strong(expect, to, order)) {
        [[likely]] return;
    } else {
        return contended_transition<BlockCounterTag>(state, from, to, order);
    }
}

} // namespace tt
