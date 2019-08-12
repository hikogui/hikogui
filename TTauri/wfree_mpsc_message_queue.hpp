// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <array>
#include <atomic>
#include <memory>
#include <thread>

namespace TTauri {

template<typename T, size_t S>
class wfree_mpsc_message_queue_item {
    constexpr size_t MAX_MESSAGE_SIZE = S;

    alignas(std::hardware_destructive_interference_size)
    std::array<MAX_MESSAGE_SIZE,std::byte> message;

    /*! Size of the object stored in the item.
     * Special values:
     *  * 0 - Empty
     */
    std::atomic<size_t> _size = 0;

    size_t size() {
        return _size.load(std::memory_order_relaxed);
    }

    T const &operator*() const {
        auto current_size = _size.load(std::memory_order_acquire);
        required_assert(current_size > 0);

        let message_ptr = reinterpret_cast<T const *>(message.data());
        return *message_ptr;
    }

    T &operator*() {
        auto current_size = _size.load(std::memory_order_acquire);
        required_assert(current_size > 0);

        auto message_ptr = reinterpret_cast<T *>(message.data());
        return *message_ptr;
    }

    /*! Reset object.
     * With only a single consumer, there is no contention on destruction.
     */
    void reset() {
        auto current_size = _size.load(std::memory_order_acquire);

        if (current_size > 0) {
            auto &message = *(*this);
            std::destroy_at(&message);
            _size.store(0, std::memory_order_release);
        }
    }

    /*! Create a new object.
     * Only a single producer is allowed at an item at a time, so we don't need
     * to compare_and_exchange the size variable. The size variable is used to
     * notify the consumer of finishing the construction.
     */
    template<typename O, typename... Args>
    void emplace(Args&&... args) {
        static_assert(sizeof(O) <= S);

        auto current_size = _size.load(std::memory_order_acquire);
        required_assert(current_size == 0)

        auto memory_ptr = reinterpret_cast<void *>(message.data());
        new(memory_ptr) O(std::forward<Args>(args)...);
        _size.store(sizeof(O), std::memory_order_release);
    }

    template<typename O>
    wfree_mpsc_message_queue_item &operator=(O const &other) {
        emplace<O>(std::forward<O>(other));
        return *this;
    }

    template<typename O>
    wfree_mpsc_message_queue_item &operator=(O &&other) {
        emplace<O>(std::forward<O>(other));
        return *this;
    }
}

template<typename T, size_t S, size_t N>
class wfree_mpsc_message_queue {
    using value_type = T;
    constexpr size_t MAX_MESSAGE_SIZE = S;
    constexpr size_t MAX_NR_MESSAGES = N;
    
    std::array<MAX_NR_MESSAGES,wfree_mpsc_message_queue_item> messages;
    std::atomic<int64_t> head = 0;
    std::atomic<int64_t> tail = 0;

    /*! Return the number of items in the message queue.
     * For the consumer this may show less items in the queue then there realy are.
     */
    int64_t size() const {
        return head.load(std::memory_order_relaxed) - tail.load(std::memory_order_relaxed);
    }

    template<typename O, typename... Args>
    void emplace_back(Args&&... args) {
        let my_head = head.fetch_add(1, std::memory_order_acquire);

        int64_t wait_count = 0;
        while (my_head >= (tail.load(std::memory_order_acquire) + MAX_NR_MESSAGE)) {
            if (++wait_count >= 5) {
                std::this_thread::yield();
            }
        }

        auto &message = messages[head % MAX_NR_MESSAGE];
        message.emplace_back<O>(args...);
    }

    template<typename O>
    void push_back(O const & other) {
        let my_head = head.fetch_add(1, std::memory_order_acquire);

        int64_t wait_count = 0;
        while (my_head >= (tail.load(std::memory_order_acquire) + MAX_NR_MESSAGE)) {
            if (++wait_count >= 5) {
                std::this_thread::yield();
            }
        }

        auto &message = messages[head % MAX_NR_MESSAGE];
        message = other;
    }

    template<typename O>
    void push_back(O&& other) {
        let my_head = head.fetch_add(1, std::memory_order_acquire);

        int64_t wait_count = 0;
        while (my_head >= (tail.load(std::memory_order_acquire) + MAX_NR_MESSAGE)) {
            if (++wait_count >= 5) {
                std::this_thread::yield();
            }
        }

        auto &message = messages[head % MAX_NR_MESSAGE];
        message = std::forward<O>(other);
    }

    bool has_front() const {
        return size() > 0 && messages[tail % MAX_NR_MESSAGE].size() > 0;
    }

    T const &front() const {
        required_assert(size() > 0);
        let &message = messages[tail % MAX_NR_MESSAGE];
        return *message;
    }

    T &front() {
        required_assert(size() > 0);
        auto &message = messages[tail % MAX_NR_MESSAGE];
        return *message;
    }

    template<typename O>
    void pop_front() {
        required_assert(size() > 0);
        auto &message = messages[tail % MAX_NR_MESSAGE];
        message.reset();
        tail.fetch_add(1, std::memory_order_release);
    }
};

}
