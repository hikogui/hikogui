// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <array>
#include <atomic>
#include <memory>
#include <thread>

namespace TTauri {

template<typename T, int64_t N>
class wfree_mpsc_message_queue {
    enum class message_state { Empty, Copying, Ready };

    struct message_type {
        T value;
        std::atomic<message_state> state = message_state::Empty;
    };

    using value_type = T;
    constexpr int64_t MAX_NR_MESSAGES = N;
    
    std::array<message_type,MAX_NR_MESSSAGES> messages;
    std::atomic<int64_t> head = 0;
    std::atomic<int64_t> tail = 0;

    /*! Return the number of items in the message queue.
     * For the consumer this may show less items in the queue then there realy are.
     */
    int64_t size() const {
        // head and tail are extremelly large integers, they will never wrap arround.
        return head.load(std::memory_order_relaxed) - tail.load(std::memory_order_relaxed);
    }

    /*!
     * Wait free if queue is not full.
     * Blocks until queue is not full.
     */
    void push(T value) noexcept {
        let index = head.add_fetch(1, std::memory_order_acquire);
        // We acquired the index before we knew if the queue was full.
        // It is assumed that the capacity of the queue is less than the number of threads.

        transition(state, message_state::Empty, message_state::Copying);

    }

    std::optional<T> pop() noexcept {

    }
};

}
