// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "fast_mutex.hpp"
#include "thread.hpp"
#include <thread>

namespace tt {

class recursive_fast_mutex {
    fast_mutex mutex;

    std::atomic<uint64_t> recurse_info = 0;

    [[nodiscard]] static constexpr bool recurse_info_equal_thread(uint64_t const &info, uint32_t thread_id) noexcept {
        return
            static_cast<uint32_t>(info) != 0 &&
            static_cast<uint32_t>(info >> 32) == thread_id;
    }

    [[nodiscard]] static constexpr bool recurse_info_is_zero(uint64_t const &info) noexcept {
        return static_cast<uint32_t>(info) == 0;
    }

    /*
    * @return True when recursing lock on the same thread.
    */
    [[nodiscard]] bool lock_recurse(uint32_t tid) noexcept {
        auto expected = recurse_info.load(std::memory_order::memory_order_relaxed);
        uint64_t desired;
        do {
            if (!recurse_info_equal_thread(expected, tid)) {
                return false;
            }

            desired = expected + 1;
        } while (!recurse_info.compare_exchange_weak(expected, desired, std::memory_order::memory_order_relaxed));

        return true;
    }

    /*
    * @return True when last recursion is completed.
    */
    [[nodiscard]] bool unlock_recurse() noexcept {
        ttlet result = recurse_info.fetch_sub(1, std::memory_order_relaxed) - 1;
        return recurse_info_is_zero(result);
    }

public:
    recursive_fast_mutex(recursive_fast_mutex const &) = delete;
    recursive_fast_mutex &operator=(recursive_fast_mutex const &) = delete;

    recursive_fast_mutex() = default;
    ~recursive_fast_mutex() = default;

    /**
     * When `try_lock()` is called on a thread that already holds the lock true is returned.
     */
    [[nodiscard]] bool try_lock() noexcept {
        ttlet tid = current_thread_id;
        tt_assume2(tid != 0, "current_thread_id is not initialized, make sure tt::set_thread_name() has been called");

        if (mutex.try_lock()) {
            recurse_info.store(static_cast<uint64_t>(tid) << 32 | 1, std::memory_order::memory_order_relaxed);
            return true;
        }
        return lock_recurse(tid);
    }

    void lock() noexcept {
        if (tt_unlikely(!try_lock())) {
            mutex.lock();
        }
    }

    void unlock() noexcept {
        if (unlock_recurse()) {
            mutex.unlock();
        }
    }

};



}