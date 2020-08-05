// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "fast_mutex.hpp"
#include "thread.hpp"
#include <thread>

namespace tt {

class recursive_fast_mutex {
    /* Thread annotation syntax.
     * 
     * FIRST - The thread that acquires/acquired the mutex
     * OWNER - The FIRST thread that recursively requests a lock.
     * OTHER - Another thread while the mutex is held.
     */

    fast_mutex mutex;

    // FIRST=write, OWNER|OTHER=read
    std::atomic<uint32_t> owner = 0;

    // FIRST=write, OWNER=increment, FIRST|OWNER=decrement
    uint32_t count = 0;

    void set_owner(uint32_t thread_id) noexcept {
        // FIRST
        tt_assume(count == 0);
        count = 1;

        // Only OTHER can execute in `try_lock(uint32_t)`, where it will either see the thread_id of zero or FIRST.
        // In both cases the OTHER thread is detected correctly.
        tt_assume(owner == 0);
        owner.store(thread_id, std::memory_order::memory_order_release);
    }

    tt_no_inline void contented_lock(uint32_t thread_id) noexcept {
        // OTHER
        mutex.lock();
        // FIRST
        set_owner(thread_id);
    }

    /**
     * When `try_lock()` is called on a thread that already holds the lock true is returned.
     */
    [[nodiscard]] bool try_lock(uint32_t thread_id) noexcept {
        tt_assume2(thread_id != 0, "current_thread_id is not initialized, make sure tt::set_thread_name() has been called");

        // ANY
        if (mutex.try_lock()) {
            // FIRST
            set_owner(thread_id);
            return true;
        }

        // OWNER | OTHER
        if (owner.load(std::memory_order::memory_order_acquire) == thread_id) {
            // OWNER
            tt_assume(count != 0);
            ++count;
            return true;
        }

        // OTHER
        return false;
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
        // ANY
        return try_lock(current_thread_id);
    }

    void lock() noexcept {
        // ANY
        ttlet thread_id = current_thread_id;

        if (tt_unlikely(!try_lock(thread_id))) {
            // OTHER
            contented_lock(thread_id);
            // FIRST
        }
        // FIRST | OWNER
    }

    void unlock() noexcept {
        // FIRST | OWNER
        tt_assume2(owner == current_thread_id, "Unlock must be called on the thread that locked the mutex");

        if (--count == 0) {
            // FIRST

            // Only OTHER can execute in `try_lock(uint32_t)`, where it will either see the thread_id of FIRST or zero.
            // In both cases the OTHER thread is detected correctly.
            owner.store(0, std::memory_order::memory_order_release);

            mutex.unlock();
            // OTHER
        }
        // OWNER | OTHER
    }

};



}
