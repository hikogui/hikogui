// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "unfair_mutex.hpp"
#include "thread.hpp"
#include <thread>

namespace tt {

class unfair_recursive_mutex {
    /* Thread annotation syntax.
     * 
     * FIRST - The thread that acquires/acquired the mutex
     * OWNER - The FIRST thread that recursively requests a lock.
     * OTHER - Another thread while the mutex is held.
     */

    unfair_mutex mutex;

    // FIRST=write, OWNER|OTHER=read
    std::atomic<uint32_t> owner = 0;

    // FIRST=write, OWNER=increment, FIRST|OWNER=decrement
    uint32_t count = 0;

public:
    unfair_recursive_mutex(unfair_recursive_mutex const &) = delete;
    unfair_recursive_mutex &operator=(unfair_recursive_mutex const &) = delete;

    unfair_recursive_mutex() = default;
    ~unfair_recursive_mutex() = default;

    [[nodiscard]] bool is_locked_by_current_thread() const noexcept {
        // The following load() is:
        // - valid-and-equal to thread_id when the OWNER has the lock.
        // - zero or valid-and-not-equal to thread_id when this is an OTHER thread.
        //
        // This only works for comparing the owner with the current thread, it would
        // not work to check the owner with a thread_id of another thread.
        return owner.load(std::memory_order::memory_order_relaxed) == current_thread_id;
    }

    /**
     * When `try_lock()` is called on a thread that already holds the lock true is returned.
     */
    [[nodiscard]] bool try_lock() noexcept {
        // FIRST | OWNER | OTHER
        ttlet thread_id = current_thread_id;

        // The following load() is:
        // - valid-and-equal to thread_id when the OWNER has the lock.
        // - zero or valid-and-not-equal to thread_id when this is an OTHER thread.
        if (owner.load(std::memory_order::memory_order_acquire) == thread_id) {
            // FIRST | OWNER
            tt_assume(count != 0);
            ++count;

            // OWNER
            return true;

        } else if (mutex.try_lock()) { // OTHER (inside the if expression)
            // FIRST
            tt_assume(count == 0);
            count = 1;
            tt_assume(owner == 0);
            owner.store(thread_id, std::memory_order::memory_order_release);

            return true;

        } else {
            // OTHER
            return false;
        }
    }

    void lock() noexcept {
        // FIRST | OWNER | OTHER
        ttlet thread_id = current_thread_id;

        // The following load() is:
        // - valid-and-equal to thread_id when the OWNER has the lock.
        // - zero or valid-and-not-equal to thread_id when this is an OTHER thread.
        if (owner.load(std::memory_order::memory_order_acquire) == thread_id) {
            // FIRST | OWNER
            tt_assume(count != 0);
            ++count;

            // OWNER

        } else {
            // OTHER
            mutex.lock();

            // FIRST
            tt_assume(count == 0);
            count = 1;
            tt_assume(owner == 0);
            owner.store(thread_id, std::memory_order::memory_order_release);
        }
    }

    void unlock() noexcept {
        // FIRST | OWNER
        tt_assume2(is_locked_by_current_thread(), "Unlock must be called on the thread that locked the mutex");

        if (--count == 0) {
            // FIRST

            // Only OTHER can execute in `lock()` or `try_lock()`,
            // where it will either see the thread_id of FIRST or zero.
            // In both cases the OTHER thread is detected correctly.
            owner.store(0, std::memory_order::memory_order_release);

            mutex.unlock();
            // OTHER
        }
        // OWNER | OTHER
    }

};



}
