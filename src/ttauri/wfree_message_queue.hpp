// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "atomic.hpp"
#include "fixed_string.hpp"
#include <array>
#include <atomic>
#include <memory>
#include <thread>
#include <optional>
#include <type_traits>
#include <functional>

namespace tt {

template<typename T, size_t Capacity>
class wfree_message_queue;

template<typename T, size_t Capacity, bool WriteOperation>
class wfree_message_queue_operation {
    wfree_message_queue<T,Capacity> *parent;
    size_t index;

public:
    wfree_message_queue_operation() noexcept : parent(nullptr), index(0) {}
    wfree_message_queue_operation(wfree_message_queue<T,Capacity> *parent, size_t index) noexcept : parent(parent), index(index) {}

    wfree_message_queue_operation(wfree_message_queue_operation const &other) = delete;

    wfree_message_queue_operation(wfree_message_queue_operation && other) :
        parent(other.parent), index(other.index)
    {
        tt_axiom(this != &other);
        other.parent = nullptr;
    }

    ~wfree_message_queue_operation()
    {
        if (parent == nullptr) {
            return;
        }

        if constexpr (WriteOperation) {
            parent->write_finish(index);
        } else {
            parent->read_finish(index);
        }
    }

    wfree_message_queue_operation& operator=(wfree_message_queue_operation const &other) = delete;

    wfree_message_queue_operation& operator=(wfree_message_queue_operation && other)
    {
        tt_axiom(this != &other);
        std::swap(index, other.index);
        std::swap(parent, other.parent);
    }

    T &operator*() noexcept
    {
        return (*parent)[index];
    }

    T *operator->() noexcept
    {
        return &(*parent)[index];
    }
};

template<typename T, size_t Capacity>
class wfree_message_queue {
    using index_type = size_t;
    using value_type = T;
    using scoped_write_operation = wfree_message_queue_operation<T,Capacity,true>;
    using scoped_read_operation = wfree_message_queue_operation<T,Capacity,false>;

    struct message_type {
        // The in_use atomic is first, to improve cache-line and prefetch.
        // There should not be much false sharing since the thread that uses the message is
        // also the one that updates the in_use atomic.
        std::atomic<bool> in_use = false;
        value_type value;
    };
    
    static constexpr index_type capacity = Capacity;

    /*! Maximum number of concurent threads that can write into the queue at once.
    */
    static constexpr index_type slack = 16;
    static_assert(capacity > (slack * 2), "The capacity of the message queue should be much larger than its slack.");

    std::array<message_type,capacity> messages;
    alignas(hardware_destructive_interference_size) std::atomic<index_type> head = 0;
    alignas(hardware_destructive_interference_size) std::atomic<index_type> tail = 0;

public:
    wfree_message_queue() = default;
    wfree_message_queue(wfree_message_queue const &) = delete;
    wfree_message_queue(wfree_message_queue &&) = delete;
    wfree_message_queue &operator=(wfree_message_queue const &) = delete;
    wfree_message_queue &operator=(wfree_message_queue &&) = delete;
    ~wfree_message_queue() = default;

    /*! Return the number of items in the message queue.
    * For the consumer this may show less items in the queue then there realy are.
    */
    index_type size() const noexcept {
        // head and tail are extremelly large integers, they will never wrap arround.
        return head.load(std::memory_order::relaxed) - tail.load(std::memory_order::relaxed);
    }

    bool empty() const noexcept {
        return head.load(std::memory_order::relaxed) <= tail.load(std::memory_order::relaxed);
    }

    bool full() const noexcept {
        return head.load(std::memory_order::relaxed) >= (tail.load(std::memory_order::relaxed) + (capacity - slack));
    }

    /*! Write a message into the queue.
    * This function is wait-free when the queue is not full().
    *
    * \return A scoped write operation which can be derefenced to access the message value.
    */
    template<basic_fixed_string BlockCounterTag = "">
    scoped_write_operation write() noexcept {
        return {this, write_start<BlockCounterTag>()};
    }

    /*! Read a message from the queue.
    * This function will block until the message being read is completed by the writing thread.
    *
    * \return A scoped read operation which can be derefenced to access the message value.
    */
    scoped_read_operation read() noexcept {
        return {this, read_start()};
    }

    value_type const &operator[](index_type index) const noexcept {
        return messages[index % capacity].value;
    }

    value_type &operator[](index_type index) noexcept {
        return messages[index % capacity].value;
    }

    /** Start a write into the message queue.
     * This function is wait-free when the queue is not full().
     * Every write_start() must be accompanied by a write_finish().
     *
     * @param CounterTag counter to increment when write is contended
     * @return The index of the message.
     */
    template<basic_fixed_string CounterTag = "">
    index_type write_start() noexcept {
        ttlet index = head.fetch_add(1, std::memory_order::acquire);
        auto &message = messages[index % capacity];

        // We acquired the index before we knew if the queue was full.
        // So we have to wait until the message is empty, however when it is empty we are
        // the only one that holds the message, so we only need to mark it that we are done with
        // writing the message.
        wait_for_transition<CounterTag>(message.in_use, false, std::memory_order::acquire);
        return index;
    }

    /*! Finish the write of a message.
     * This function is wait-free.
     *
     * \param index The index given from write_start().
     */
    void write_finish(index_type index) noexcept {
        auto &message = messages[index % capacity];

        // Mark that the message is finished with writing.
        message.in_use.store(true, std::memory_order::release);
    }

    /*! Start a read from the message queue.
     * This function will block until the message being read is completed by the writing thread.
     * Every read_start() must be accompanied by a read_finish().
     *
     * \return The index of the message.
     */
    template<basic_fixed_string CounterTag = "">
    index_type read_start() noexcept {
        ttlet index = tail.fetch_add(1, std::memory_order::acquire);
        auto &message = messages[index % capacity];

        // We acquired the index before we knew if the message was ready.
        wait_for_transition<CounterTag>(message.in_use, true, std::memory_order::acquire);
        return index;
    }

    /*! Finish a read from the message queue.
    * This function is wait-free.
    *
    * \param index The index given from read_start().
    */
    void read_finish(index_type index) noexcept {
        auto &message = messages[index % capacity];

        // We acquired the index before we knew if the message was ready.
        message.in_use.store(false, std::memory_order::release);

        // The message itself does not need to be destructed.
        // This will happen automatically when wrapping around the ring buffer overwrites the message.
    }
};

}
