// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include <atomic>
#include <memory>

namespace tt {
struct unfair_lock_wrap;

class fast_mutex {
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
    std::atomic<int32_t> semaphore = 0;

    int32_t *semaphore_ptr() noexcept {
        return reinterpret_cast<int32_t *>(&semaphore);
    }

    void lock_contented(int32_t first) noexcept;

#elif TT_OPERATING_SYSTEM == TT_OS_MACOS
    std::unique_ptr<unfair_lock_wrap> mutex;

#else
#error "Not implemented fast_mutex"
#endif

public:
    fast_mutex() noexcept;
    ~fast_mutex();

    void lock() noexcept;

    void unlock() noexcept;
};


};
