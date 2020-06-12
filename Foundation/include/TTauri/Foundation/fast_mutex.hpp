// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include <atomic>

namespace tt {

class fast_mutex {
    std::atomic<int32_t> semaphore = 0;

    int32_t *semaphore_ptr() noexcept {
        return reinterpret_cast<int32_t *>(&semaphore);
    }

    void lock_contented(int32_t first) noexcept;

public:
    void lock() noexcept;

    void unlock() noexcept;
};


};