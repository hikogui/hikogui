// Copyright 2019 Pokitec
// All rights reserved.

#include "unfair_mutex.hpp"
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

unfair_mutex::unfair_mutex() noexcept :
    mutex(std::make_unique<unfair_lock_wrap>()) {}

unfair_mutex::~unfair_mutex() {}


void unfair_mutex::lock() noexcept
{
    mutex->lock();
}

void unfair_mutex::unlock() noexcept
{
    mutex->unlock();
}

}
