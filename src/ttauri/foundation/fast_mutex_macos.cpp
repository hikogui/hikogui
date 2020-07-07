// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/foundation/fast_mutex.hpp"
#include <os/lock.h>

namespace tt {

struct unfair_lock_wrap {
    os_unfair_lock mutex = OS_UNFAIR_LOCK_INIT;

    void lock() noexcept {
        os_unfair_lock_lock(&mutex);
    }

    void unlock() noexcept {
        os_unfair_lock_unlock(&mutex);
    }
};

fast_mutex::fast_mutex() noexcept :
    mutex(std::make_unique<unfair_lock_wrap>()) {}

fast_mutex::~fast_mutex() {}


void fast_mutex::lock() noexcept
{
    mutex->lock();
}

void fast_mutex::unlock() noexcept
{
    mutex->unlock();
}

}
