// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "fast_mutex.hpp"
#include <thread>

namespace tt {

class recursive_fast_mutex {
    fast_mutex mutex;

    fast_mutex thread_id_mutex;
    std::thread::id thread_id;
    int recurse_count = 0;

    /*
    * @return True when recursing lock on the same thread.
    */
    bool lock_recurse() noexcept {
        auto lock = std::scoped_lock(thread_id_mutex);

        if (tt_likely(recurse_count != 0 && thread_id == std::this_thread::get_id())) {
            recurse_count++;
            return true;
        } else {
            return false;
        }
    }

    /*
    * @return True when last recursion is completed.
    */
    bool unlock_recurse() noexcept {
        auto lock = std::scoped_lock(thread_id_mutex);
        return --recurse_count == 0;
    }

public:
    recursive_fast_mutex(recursive_fast_mutex const &) = delete;
    recursive_fast_mutex &operator=(recursive_fast_mutex const &) = delete;

    recursive_fast_mutex() = default;
    ~recursive_fast_mutex() = default;

    

    /**
     * When `try_lock()` is called on a thread that already holds the lock true is returned.
     */
    bool try_lock() noexcept {
        if (tt_likely(mutex.try_lock())) {
            thread_id = std::this_thread::get_id();
            recurse_count = 1;
            return true;
        }
        return lock_recurse();
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