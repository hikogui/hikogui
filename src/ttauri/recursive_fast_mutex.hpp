// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "fast_mutex.hpp"
#include "thread.hpp"
#include <thread>

namespace tt {

class recursive_fast_mutex {
    fast_mutex mutex;

    std::atomic<uint32_t> owner = 0;
    uint32_t count;

    void set_owner(uint32_t thread_id) noexcept {
        // Since we have the lock, no other thread can modify the thread_id or count.
        tt_assume(count == 0);
        count = 1;
        tt_assume(owner == 0);
        owner.store(thread_id, std::memory_order::memory_order_release);
    }

    tt_no_inline void contented_lock(uint32_t thread_id) noexcept {
        mutex.lock();
        set_owner(thread_id);
    }

    /**
     * When `try_lock()` is called on a thread that already holds the lock true is returned.
     */
    [[nodiscard]] bool try_lock(uint32_t thread_id) noexcept {
        tt_assume2(thread_id != 0, "current_thread_id is not initialized, make sure tt::set_thread_name() has been called");

        if (mutex.try_lock()) {
            set_owner(thread_id);
            return true;
        }

        // At this point there is recursion, or this is a second thread.
        if (owner.load(std::memory_order::memory_order_seq_cst) == thread_id) {
            // If we are here then this is recursion by the owning thread.
            tt_assume(count != 0);
            ++count;
            return true;
        }

        // For a second thread the owner was either empty or not equal to thread_id.
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
        return try_lock(current_thread_id);
    }

    void lock() noexcept {
        ttlet thread_id = current_thread_id;

        if (tt_unlikely(!try_lock(thread_id))) {
            // This is a second thread, block on the mutex.
            // Once this thread gets the lock then it should set the owner.
            contented_lock(thread_id);
        }
    }

    void unlock() noexcept {
        tt_assume2(owner == current_thread_id, "Unlock must be called on the thread that locked the mutex");

        // This can only be called by the owning thread, decrement the count and
        // when zero empty the thread_id.
        if (--count == 0) {
            // A second thread may be in try_lock() reading the owner value.
            // Before or after the next write try_lock() will fail.
            owner.store(0, std::memory_order::memory_order_seq_cst);
            mutex.unlock();
        }
    }

};



}
