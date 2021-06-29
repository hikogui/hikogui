

#pragma once

#include "unfair_mutex.hpp"
#include <atomic>
#include <memory>
#include <array>

namespace tt {

/** A fifo designed for wait-free write and locked read access.
 * The number of messages in the fifo is dictated by the size of each
 * message and the total buffer size is 64 kByte.
 *
 * @tparam T Base class of the value type stored in the ring buffer.
 * @tparam S Size of each message, must be power-of-two.
 */
template<typename T, size_t S>
class wfree_fifo {
public:
    static_assert(std::has_single_bit(S), "Only power-of-two number of messages size allowed.");

    using value_type = T;

    constexpr size_t buffer_size = 65536;
    constexpr size_t message_size = S;
    constexpr size_t num_messages = buffer_size / N;

    struct message_type {
        std::atomic<value_type *> pointer;
        std::array<std::byte, message_size - sizeof(value_type *)> buffer;
    };


    template<typename F>
    void peek_front(F &&... f) noexcept
    {
        ttlet lock = std::scoped_lock(mutex);

        auto index = _tail;

        // The shift here should be eliminated by the equal shift inside the index operator.
        auto &message = _messages[index / message_size];

        // Check if the message.pointer is not null, this is when the writer
        // has finished writing the message.
        if (auto ptr = message.pointer.load(std::memory_order::acquire)) {
            // Call the callback, while holding the mutex.
            f(*ptr);

            // Destroy the object depending if it lives in the buffer or on the heap.
            if (ptr == reinterpret_cast<value_type *>(message.buffer.data())) {
                std::destroy_at(ptr);
            } else {
                delete ptr;
            }

            // We are done with the message.
            message.pointer.store(nullptr, std::memory_order::release);
            ++_tail;
        }
    }


    /** Create an object in-place on the fifo.
     * @tparam O Derived object type of value_type to be stored in the fifo.
     * @param args The arguments passed to the constructor of O.
     */
    template<typename O, typename... Args>
    void emplace_back(Args &&... args) noexcept
    {
        constexpr bool large_object = sizeof(O) > std::size(message_type::buffer);

        // We need a new index.
        // - The index is a byte index into 64kByte of memory.
        // - Increment index by the message_size and the _head will overflow naturally
        //   at the end of the fifo.
        // - We don't care about memory ordering with other writer threads. as
        //   each message has an atomic for handling read/writer contention.
        // - We don't have to check full/empty, this is done on the message itself.
        size_t index = _head.fetch_add(message_size, std::memory_order::relaxed);

        // The divide here should be eliminated by the equal multiply inside the index operator.
        auto &message = _messages[index / message_size];

        value_type *new_ptr;
        if constexpr (large_object) {
            // We need a heap allocated pointer with a fully constructed object
            new_ptr = new_object = new O(std::forward<Args>(args)...);
        } else {
            // For small object, we will need it to exist in the message buffer.
            // Do not construct this object yet.
            new_ptr = reinterpret_cast<value_type *>(message.buffer.data());
        }

        // Wait until the message.pointer is nullptr indicating that a reader
        // has finished with this message.
        value_type *expected = nullptr;
        while (not message.pointer.compare_exchange_strong(expected, new_ptr, std::memory_order_acquire)) {
            // If we get here, that would suck, but nothing to do about it.
            [[unlikely]] increment_counter<"wfree_fifo">();
            std::this_thread::sleep_for(16ms);
            expected = 0;
        }

        if constexpr (not large_object) {
            // Allocate using placement new the small object in the buffer.
            new (new_ptr) O(std::forward<Args>(args)...);
        }
    }

private:
    unfair_mutex _mutex;
    std::atomic<uint16_t> _head;
    uint16_t _tail;
    std::array<message_type, 256> _messages;
};


}

