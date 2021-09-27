// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "architecture.hpp"
#include <atomic>
#include <thread>
#include <chrono>

namespace tt {

/** Lock-free fetch-then-max operation on an atomic.
 */
template<typename T>
T fetch_max(std::atomic<T> &lhs, T rhs, std::memory_order order) noexcept
{
    auto expected = lhs.load(order);
    while (expected < rhs) {
        if (lhs.compare_exchange_weak(expected, rhs, order)) {
            return expected;
        }
    }
    return expected;
}

/** Lock-free fetch-then-min operation on an atomic.
 */
template<typename T>
T fetch_min(std::atomic<T> &lhs, T rhs, std::memory_order order) noexcept
{
    auto expected = lhs.load(order);
    while (rhs < expected) {
        if (lhs.compare_exchange_weak(expected, rhs, order)) {
            return expected;
        }
    }
    return expected;
}

} // namespace tt
