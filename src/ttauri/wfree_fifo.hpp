// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "architecture.hpp"
#include "counters.hpp"
#include "memory.hpp"
#include <concepts>
#include <atomic>
#include <memory>
#include <array>

namespace tt::inline v1 {

/** A wait-free multiple-producer/single-consumer fifo designed for absolute performance.
 * Because of performance reasons the ring-buffer is 64kByte.
 * Each slot in the ring buffer consists of a pointer and a byte buffer for storage.
 *
 * The number of slots in the ring-buffer is dictated by the size of each
 * slot and the ring-buffer size.
 *
 * @tparam T Base class of the value type stored in the ring buffer.
 * @tparam SlotSize Size of each slot, must be power-of-two.
 * @param Alignment Optional alignment of wfree_fifo.
 */
template<typename T, std::size_t SlotSize>
class alignas(SlotSize) wfree_fifo {
public:
    static_assert(std::has_single_bit(SlotSize), "Only power-of-two number of messages size allowed.");
    static_assert(SlotSize < 65536);

    using value_type = T;

    static constexpr std::size_t fifo_size = 65536;
    static constexpr std::size_t slot_size = SlotSize;
    static constexpr std::size_t num_slots = fifo_size / slot_size;

    struct slot_type {
        static constexpr std::size_t buffer_size = slot_size - sizeof(value_type *);

        std::atomic<value_type *> pointer = nullptr;
        std::array<std::byte, buffer_size> buffer = {};

        [[nodiscard]] std::byte *begin() noexcept
        {
            return buffer.data();
        }

        [[nodiscard]] std::byte *end() noexcept
        {
            return buffer.data() + buffer_size;
        }
    };

    constexpr wfree_fifo() noexcept = default;
    wfree_fifo(wfree_fifo const &) = delete;
    wfree_fifo(wfree_fifo &&) = delete;
    wfree_fifo &operator=(wfree_fifo const &) = delete;
    wfree_fifo &operator=(wfree_fifo &&) = delete;

    /** Take one message from the fifo slot.
     * Reads one message from the ring buffer and passes it to a call of operation.
     * If no message is available this function returns without calling operation.
     *
     * @param operation A `void(value_type const &)` which is called when a message is available.
     * @return True if a message was available in the fifo.
     */
    template<typename Operation>
    bool take_one(Operation &&operation) noexcept
    {
        auto index = _tail;

        // The shift here should be eliminated by the equal shift inside the index operator.
        auto &slot = _slots[index / slot_size];

        // Check if the slot.pointer is not null, this is when the writer
        // has finished writing the slot.
        if (auto ptr = slot.pointer.load(std::memory_order::acquire)) {
            std::forward<Operation>(operation)(*ptr);

            // Destroy the object depending if it lives inside the ring buffer.
            ttlet byte_ptr = reinterpret_cast<std::byte *>(ptr);
            if (byte_ptr >= slot.begin() and byte_ptr < slot.end()) {
                // Destroy the object inside the ring buffer.
                std::destroy_at(ptr);
            } else {
                // Destroy the object on the heap.
                delete ptr;
            }

            // We are done with the slot.
            slot.pointer.store(nullptr, std::memory_order::release);
            _tail += slot_size;
            return true;
        } else {
            return false;
        }
    }

    /** Take all message from the queue.
     * Reads each message from the ring buffer and passes it to a call of operation.
     * If no message are available this function returns without calling operation.
     *
     * @param operation A `void(value_type const &)` which is called when a message is available.
     */
    template<typename Operation>
    void take_all(Operation const &operation) noexcept
    {
        while (take_one(operation)) {}
    }

    tt_no_inline void contended() noexcept
    {
        using namespace std::chrono_literals;

        // If we get here, that would suck, but nothing to do about it.
        ++global_counter<"wfree_fifo">;
        std::this_thread::sleep_for(16ms);
    }

    /** Create an message in-place on the fifo.
     * @tparam Message The message type derived from value_type to be stored in a free slot.
     * @param args The arguments passed to the constructor of Message.
     */
    template<typename Message, typename... Args>
    tt_force_inline void emplace(Args &&...args) noexcept
    {
        // We need a new index.
        // - The index is a byte index into 64kByte of memory.
        // - Increment index by the slot_size and the _head will overflow naturally
        //   at the end of the fifo.
        // - We don't care about memory ordering with other writer threads. as
        //   each slot has an atomic for handling read/writer contention.
        // - We don't have to check full/empty, this is done on the slot itself.
        ttlet index = _head.fetch_add(slot_size, std::memory_order::relaxed);
        tt_axiom(index % slot_size == 0);

        auto &slot =
            *std::launder(std::assume_aligned<slot_size>(reinterpret_cast<slot_type *>(reinterpret_cast<char *>(this) + index)));

        // Calculate if the Message will fit in the slot.
        constexpr auto offset_within_slot = ceil(sizeof(value_type *), alignof(Message));
        constexpr auto needed_slot_size = offset_within_slot + sizeof(Message);

        if constexpr (needed_slot_size <= sizeof(slot_type)) {
            // Wait until the slot.pointer is a nullptr.
            // And acquire the buffer to start overwriting it.
            // There are no other threads that will make this non-null afterwards.
            while (slot.pointer.load(std::memory_order_acquire)) {
                // If we get here, that would suck, but nothing to do about it.
                [[unlikely]] contended();
            }

            constexpr auto offset_within_buffer = offset_within_slot - sizeof(value_type *);

            // Overwrite the buffer with the new slot.
            auto new_ptr = new (slot.begin() + offset_within_buffer) Message(std::forward<Args>(args)...);

            // Release the buffer for reading.
            slot.pointer.store(new_ptr, std::memory_order::release);

        } else {
            // We need a heap allocated pointer with a fully constructed object
            // Lets do this ahead of time to let another thread have some time
            // to release the ring-buffer-slot.
            ttlet new_ptr = new Message(std::forward<Args>(args)...);
            tt_axiom(new_ptr != nullptr);

            // Wait until the slot.pointer is a nullptr.
            // We don't need to acquire since we wrote into a new heap location.
            // There are no other threads that will make this non-null afterwards.
            while (slot.pointer.load(std::memory_order::relaxed)) {
                // If we get here, that would suck, but nothing to do about it.
                [[unlikely]] contended();
            }

            // Release the heap for reading.
            slot.pointer.store(new_ptr, std::memory_order::release);
        }
    }

private:
    std::array<slot_type, num_slots> _slots = {}; // must be at offset 0
    std::atomic<uint16_t> _head = 0;
    std::array<std::byte, tt::hardware_destructive_interference_size> _dummy;
    uint16_t _tail = 0;
};

} // namespace tt::inline v1
